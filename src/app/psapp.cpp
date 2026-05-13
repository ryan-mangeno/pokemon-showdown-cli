#include <pkmpch.h>

#include "psapp.h"
#include "input/cl_input.h"
#include "core/logger.h"
#include "core/event/message_event.h"
#include "core/event/command_event.h"
#include "core/event/key_event.h"
#include "core/event/battle_event.h"

#include <nlohmann/json.hpp>

namespace pkm {

    PsApp::PsApp() : m_running(false), m_dirty_ui(false) {}

    bool PsApp::init() {
        m_client = MakeRef<protocol::PsClient>();
        if (!m_client->init()) {
            PK_ERROR("Failed to initialize PsClient!");
            return false;
        }

        // menu is always at the bottom of layer stack
        MenuLayer* menu = new MenuLayer(m_client, [this](Scope<Event> e) {
            m_event_queue.push(std::move(e));
        });
        m_event_queue.push(MakeScope<LayerPushEvent>(menu, false));

        m_input = MakeScope<CLInput>();
        m_input->set_callback([this](Event& e) {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<CommandEvent>([this](CommandEvent& e) {
                Scope<Event> event = MakeScope<CommandEvent>(e);
                m_event_queue.push(std::move(event));
                return true;
            });
        });

        m_input->start();
        m_running = true;
        return true;
    }

    void PsApp::run() {
        m_client->start();

        while (m_running) {
            on_update();
            poll();
            on_render();
        }

        shutdown();
    }

    void PsApp::shutdown() {
        PK_INFO("Shutting Down PsApp ...");
        m_input->stop();
        m_client->stop();
        m_running = false;
    }
    
    void LoginOverlay::on_render() {
        std::stringstream ss;

        if (!m_is_signup) {
            ss << "\r=================================================\r\n";
            ss << "\r  POKEMON SHOWDOWN CLI  |  LOGIN                \r\n";
            ss << "\r=================================================\r\n\r\n";

            ss << "\r  Please log in to your account.\r\n\r\n";

            ss << "\r  Type your username and press [Enter].\r\n\r\n";

            ss << "\r--- AVAILABLE ACTIONS ---\r\n";
            ss << "\r [b] Back to Main Menu\r\n";
            ss << "\r [q] Quit\r\n";

            ss << "\r=================================================\r\n";
        } else {
            ss << "\r=================================================\r\n";
            ss << "\r  POKEMON SHOWDOWN CLI  |  SIGN-UP              \r\n";
            ss << "\r=================================================\r\n\r\n";

            ss << "\r  Create a new Pokemon Showdown account.\r\n\r\n";

            ss << "\r  Type your desired username and press [Enter].\r\n\r\n";

            ss << "\r--- AVAILABLE ACTIONS ---\r\n";
            ss << "\r [b] Back to Main Menu\r\n";
            ss << "\r [q] Quit\r\n";

            ss << "\r=================================================\r\n";
        }
        std::cout << ss.str() << std::endl;
    }

   
    void BattleLayer::on_render() {
        std::stringstream ss;

        ss << "\r=================================================\r\n";
        ss << "\r  POKEMON SHOWDOWN CLI  | Room: " << m_battle_room << "\r\n";
        ss << "\r=================================================\r\n\r\n";

        ss << "\r--- OPPONENT TEAM ---\r\n";
        const auto& team1 = m_state.opponent_team();
        for (size_t i = 0; i < team1.size(); i++) {
            const auto& p = team1[i];
            ss << "\r [s" << (i+1) << "] "
            << (p.active ? "*ACTIVE* " : "         ")
            << p.name << " (" << p.hp_current << "/" << p.hp_max << " HP)\r\n";
        }
        ss << "\r\n";

        ss << "\r--- YOUR TEAM ---\r\n";
        const auto& team2 = m_state.your_team();
        for (size_t i = 0; i < team2.size(); i++) {
            const auto& p = team2[i];
            ss << "\r [s" << (i+1) << "] "
            << (p.active ? "*ACTIVE* " : "         ")
            << p.name << " (" << p.hp_current << "/" << p.hp_max << " HP)"
            << (p.active ? (" : Tera - " + m_state.active_pokemon().tera_type)  : "")
            << "\r\n";
        }
        ss << "\r\n";

        ss << "\r--- AVAILABLE ACTIONS ---\r\n";
        const auto& moves = m_state.available_moves();
        if (moves.empty()) {
            ss << "\r  Waiting for server...\r\n";
        } else {
            for (size_t i = 0; i < moves.size(); i++) {
                ss << "\r [" << (i+1) << "] " << moves[i].name
                << " (" << moves[i].pp << "/" << moves[i].max_pp << " PP)";
                if (moves[i].disabled) ss << " [DISABLED]";
                ss << "\r\n";
            }
        }

        ss << "\r\n [f] Forfeit\r\n";
        ss << "\r [t] Toggle Timer\r\n";
        ss << "\r [s<1-6>] Switch\r\n";
        ss << "\r [move] <optional:tera> \r\n";

        ss << "\r=================================================\r\n";
        std::cout << ss.str() << std::endl;
    }

    void MenuLayer::on_render() {
        std::stringstream ss;

        ss << "\r=================================================\r\n";
        ss << "\r  POKEMON SHOWDOWN CLI  |  MAIN MENU            \r\n";
        ss << "\r=================================================\r\n\r\n";

        ss << "\r  Welcome!\r\n\r\n";

        ss << "\r--- AVAILABLE ACTIONS ---\r\n";
        ss << "\r [1] Search for Random Battle\r\n";
        ss << "\r [2] Login\r\n";
        ss << "\r [3] Sign-up\r\n";
        ss << "\r [q] Quit\r\n";

        ss << "\r=================================================\r\n";
        std::cout << ss.str() << std::endl;
    }

    void PsApp::on_update() {
        process_network();
    }

    void PsApp::on_render() {
        if (m_dirty_ui && !m_layerstack.empty()) {
            // TODO: add a better way to get top of stack
            // also should add some additional error handling, this is primitive
            auto top_layer = m_layerstack.back(); 
            top_layer->on_render();
            m_dirty_ui = false;
        }
    }

    void PsApp::process_network() {
        protocol::Message msg;
        while (m_client->poll(msg)) {
            MessageEvent e(msg);
            push_to_layers(e);
        }
    }

    void PsApp::poll() {
        Scope<Event> e = nullptr;
        bool exit = false;
        while (m_event_queue.pop(e) && !exit) {
            PK_INFO("[App] Got event: '{}'", e->get_name());

            switch(e->get_event_type()) {
                case EventType::Command: { 
                    CommandEvent* cmd_event = dynamic_cast<CommandEvent*>(e.get());
                    const std::string& cmd = cmd_event->get_command();
                    if (cmd == "q") {
                        PK_INFO("Quitting...");
                        m_running = false;
                        exit = true;
                    } 
                    break;
                }

                case EventType::LayerPush: {
                    LayerPushEvent* layer_event = dynamic_cast<LayerPushEvent*>(e.get());   
                    Layer* layer = layer_event->get_layer_ptr();
                    bool is_overlay = layer_event->is_overlay();
                    is_overlay ? m_layerstack.push_overlay(layer) : m_layerstack.push_layer(layer);
                    m_dirty_ui = true;
                    break;
                }

                case EventType::LayerPop: {
                    LayerPopEvent* layer_event = dynamic_cast<LayerPopEvent*>(e.get());   
                    Layer* layer = layer_event->get_layer_ptr();
                    bool is_overlay = layer_event->is_overlay();
                    is_overlay ? m_layerstack.pop_overlay(layer) : m_layerstack.pop_layer(layer);
                    m_dirty_ui = true; 
                    break;
                }

                case EventType::Login: {
                    LoginEvent* login_event = dynamic_cast<LoginEvent*>(e.get());   
                    bool is_signup = login_event->is_signup();
                    LoginOverlay* login = new LoginOverlay(m_client, is_signup, [this](Scope<Event> e) {
                        m_event_queue.push(std::move(e));
                    });
                    m_layerstack.push_overlay(login);
                    m_dirty_ui = true; 
                    break;
                }

                case EventType::BattleSearch: {
                    BattleSearchEvent* battle_event = dynamic_cast<BattleSearchEvent*>(e.get());   
                    BattleLayer* battle = new BattleLayer(m_client, [this](Scope<Event> e) {
                        m_event_queue.push(std::move(e));
                    });
                    m_layerstack.push_overlay(battle);
                    m_dirty_ui = true; 
                    break;
                } 

                case EventType::BattleJoin: {
                    m_dirty_ui = true; 
                    break;
                }

                default:
                    break;
            } 

            push_to_layers(*e);
        }
        m_input->poll();
    }

    void PsApp::push_to_layers(Event& e) {
        // dispatch top-down, stop if handled
        for (size_t i = m_layerstack.size(); i > 0; --i) {
            m_layerstack[i - 1]->on_event(e);
            if (e.get_handled()) break;
        }
    }
}
