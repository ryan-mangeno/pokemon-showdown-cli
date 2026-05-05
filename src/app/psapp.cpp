#include <pkmpch.h>

#include "psapp.h"
#include "input/cl_input.h"
#include "core/logger.h"
#include "core/event/message_event.h"
#include "core/event/command_event.h"
#include "core/event/key_event.h"

#include <nlohmann/json.hpp>

namespace pkm {

    PsApp::PsApp() : m_running(false), m_in_battle(false) {}

    bool PsApp::init() {
        m_client = MakeRef<protocol::PsClient>();
        if (!m_client->init()) {
            PK_ERROR("Failed to initialize PsClient!");
            return false;
        }

        // menu is always at the bottom of layer stack
        m_layerstack.push_layer(new MenuLayer(m_client));

        m_input = MakeScope<CLInput>();
        m_input->set_callback([this](Event& e) {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<CommandEvent>([this](CommandEvent& e) {
                Scope<Event> event = MakeScope<CommandEvent>(e);
                m_event_queue.push(event);
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
            on_render();
            process_network();
            poll();
            std::this_thread::yield();
        }

        shutdown();
    }

    void PsApp::shutdown() {
        m_input->stop();
        m_client->stop();
        m_running = false;
    }
    
    void LoginOverlay::on_render() {
        std::stringstream ss;

        if (m_is_logging_in) {

            ss << "\r=================================================\r\n";
            ss << "\r  POKEMON SHOWDOWN CLI  |  LOGIN                \r\n";
            ss << "\r=================================================\r\n\r\n";

            ss << "\r  Please log in to your account.\r\n\r\n";

            ss << "\r  Type your username and press Enter.\r\n\r\n";

            ss << "\r--- AVAILABLE ACTIONS ---\r\n";
            ss << "\r [b] Back to Main Menu\r\n";
            ss << "\r [q] Quit\r\n";

            ss << "\r=================================================\r\n";
        } else {
            ss << "\r=================================================\r\n";
            ss << "\r  POKEMON SHOWDOWN CLI  |  SIGN-UP              \r\n";
            ss << "\r=================================================\r\n\r\n";

            ss << "\r  Create a new Pokemon Showdown account.\r\n\r\n";

            ss << "\r  Type your desired username and press Enter.\r\n\r\n";

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
        ss << "\r  POKEMON SHOWDOWN CLI  | Room: " << m_battle_layer->get_battle_room() << "\r\n";
        ss << "\r=================================================\r\n\r\n";

        const protocol::BattleState& bs = m_battle_layer->get_battle_state();

        ss << "\r--- OPPONENT TEAM ---\r\n";
        const auto& team1 = bs.opponent_team();
        for (size_t i = 0; i < team1.size(); i++) {
            const auto& p = team1[i];
            ss << "\r [s" << (i+1) << "] "
            << (p.active ? "*ACTIVE* " : "         ")
            << p.name << " (" << p.hp_current << "/" << p.hp_max << " HP)\r\n";
        }
        ss << "\r\n";

        ss << "\r--- YOUR TEAM ---\r\n";
        const auto& team2 = bs.your_team();
        for (size_t i = 0; i < team2.size(); i++) {
            const auto& p = team2[i];
            ss << "\r [s" << (i+1) << "] "
            << (p.active ? "*ACTIVE* " : "         ")
            << p.name << " (" << p.hp_current << "/" << p.hp_max << " HP)"
            << (p.active ? (" : Tera - " + bs.active_pokemon().tera_type)  : "")
            << "\r\n";
        }
        ss << "\r\n";

        ss << "\r--- AVAILABLE ACTIONS ---\r\n";
        const auto& moves = bs.available_moves();
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
        // TODO:  render ui of the top layer
    }

    void PsApp::on_render() {
    }

    void PsApp::process_network() {
        protocol::Message msg;
        while (m_client->poll(msg)) {
            on_network_message(msg);
            MessageEvent e(msg);
            push_to_layers(e);
        }
    }

    void PsApp::poll() {
        Scope<Event> e = nullptr;
        while (m_event_queue.pop(e)) {
            PK_INFO("[App] Got event: '{}'", e->get_name());

            if (e->get_event_type() == EventType::Command) {
                CommandEvent* cmd_event = dynamic_cast<CommandEvent*>(e.get());
                const std::string& cmd = cmd_event->get_command();
                if (cmd == "q") {
                    PK_INFO("Quitting...");
                    m_running = false;
                    break;
                } 
            } else if (e->get_event_type() == EventType::Layer) {
                LayerEvent* layer_event = dyanmic_cast<LayerEvent*>(e.get());   
                Layer* layer = layer_event->get_layer_ptr();
                m_layerstack.push_layer(layer_event.get_layer_ptr());
                break;
            }

            push_to_layers(*e);
        }
        m_input->poll();
    }

    void PsApp::push_to_layers(Event& e) {
        // dispatch top-down, stop if handled
        for (auto it = m_layerstack.end(); it != m_layerstack.begin();) {
            (*(--it))->on_event(e);
            if (e.get_handled()) break;
        }
    }

    void PsApp::on_network_message(const protocol::Message& msg) {
        // PsApp handles structural decisions, push/pop layers
        // everything else goes to layers via MessageEvent
        PK_TRACE("Network Msg: {}", msg.type);
        if (msg.type == "updatesearch" && !msg.args.empty()) {
            auto j = nlohmann::json::parse(msg.args[0]);
            if (!j["games"].is_null()) {
                std::string room = j["games"].begin().key();
                // TODO: maybe send a join in the battle layer and just send an event here
                m_client->send("|/join " + room);
            }
        } else if (msg.type == "win" || msg.type == "tie") {
            // TODO:
            // send layer battle end event to let battle layer pop itsself
            PK_INFO("[App] Battle ended, returning to menu");
        }
    }
}
