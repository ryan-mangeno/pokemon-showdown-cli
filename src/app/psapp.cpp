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

        // MenuLayer is always at the bottom
        // TODO: need to call on attach for all layers
        m_layerstack.push_layer(new MenuLayer(m_client));

        // input thread: only pushes raw strings to queue, never touches layers
        m_input = MakeScope<CLInput>();
        m_input->set_callback([this](Event& e) {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<CommandEvent>([this](CommandEvent& e) {
                std::string cmd = e.get_command();
                // TODO: change to a event queue
                m_input_queue.push(cmd);
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
            process_input();
            std::this_thread::yield();
        }

        m_client->stop();
    }

    void PsApp::shutdown() {
        m_running = false;
        m_input->stop();
    }

    std::string PsApp::build_battle_ui() {
        if (!m_battle_layer) {
            PK_WARN("Can't build battle ui without battle layer!");
            return "";
        }

        std::stringstream ss;

        ss << "\r=================================================\r\n";
        ss << "\r  POKEMON SHOWDOWN CLI  | Room: " << m_battle_layer->get_battle_room() << "\r\n";
        ss << "\r=================================================\r\n\r\n";

        const protocol::BattleState& bs = m_battle_layer->get_battle_state();

        ss << "\r--- OPPONENT TEAM ---\r\n";
        auto& team1 = bs.opponent_team();
        for (size_t i = 0; i < team1.size(); i++) {
            const auto& p = team1[i];
            ss << "\r [s" << (i+1) << "] "
            << (p.active ? "*ACTIVE* " : "         ")
            << p.name << " (" << p.hp_current << "/" << p.hp_max << " HP)\r\n";
        }
        ss << "\r\n";

        ss << "\r--- YOUR TEAM ---\r\n";
        auto& team2 = bs.your_team();
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
        auto& moves = bs.available_moves();
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
        return ss.str();
    }

    std::string PsApp::build_main_menu_ui() {
        std::stringstream ss;

        ss << "\r=================================================\r\n";
        ss << "\r  POKEMON SHOWDOWN CLI  |  MAIN MENU            \r\n";
        ss << "\r=================================================\r\n\r\n";

        ss << "\r  Welcome!\r\n\r\n";

        ss << "\r--- AVAILABLE ACTIONS ---\r\n";
        ss << "\r [1] Search for Random Battle\r\n";
        ss << "\r [q] Quit\r\n";

        ss << "\r=================================================\r\n";
        return ss.str();
    }

    void PsApp::on_update() {
        if (m_in_battle) {
            std::string new_ui = build_battle_ui();
            if (new_ui != m_ui) {  // only update if changed
                m_ui = new_ui;
                m_input->set_input_ui(m_ui);
            }
        } else {
            // TODO: handle more cases for dif uis, also need to make other menus
            std::string new_ui = build_main_menu_ui();
            if (new_ui != m_ui) {
                m_ui = new_ui;
                m_input->set_input_ui(m_ui);
            }
        }
    }

    void PsApp::on_render() {
        // TODO: "render" the linenoise prompt set for cli input
    }

    void PsApp::process_network() {
        protocol::Message msg;
        while (m_client->poll(msg)) {
            on_network_message(msg);
            MessageEvent e(msg);
            push_to_layers(e);
        }
    }

    void PsApp::process_input() {
        std::string cmd;
        while (m_input_queue.pop(cmd)) {
            PK_INFO("[App] Got command: '{}'", cmd);

            if (cmd == "q" || cmd == "quit") {
                PK_INFO("Quitting...");
                m_running = false;
                return;
            }

            CommandEvent e{cmd};
            push_to_layers(e);
        }
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
            if (!j["games"].is_null() && !m_in_battle) {
                std::string room = j["games"].begin().key();
                m_in_battle = true;
                m_client->send("|/join " + room);

                // push battle layer on top
                m_battle_layer = new BattleLayer(m_client, room);
                m_layerstack.push_layer(m_battle_layer);
            }
            // TODO: add message on end game
        } else if (msg.type == "win" || msg.type == "tie") {
            if (m_battle_layer) {
                m_layerstack.pop_layer(m_battle_layer);
                m_battle_layer = nullptr;
                m_in_battle = false;
                PK_INFO("[App] Battle ended, returning to menu");
            }
        }
    }
}