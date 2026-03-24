#include <pkmpch.h>

#include "psapp.h"
#include "protocol/parser.h"
#include "input/cl_input.h"
#include "core/logger.h"
#include "core/event/key_event.h"

namespace pkm {

    PsApp::PsApp() : m_running(false) {}

    bool PsApp::init() {
        m_client = MakeScope<protocol::PsClient>();
        if (!m_client->init()) {
            PK_ERROR("Failed to initialize PsClient!");
            return false;
        }

        m_input = MakeScope<CLInput>();
        m_input->set_callback ( [this](Event& e) {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<KeyTypedEvent>([this](KeyTypedEvent& e) {
                char c = e.get_keycode();
                if (c == '\n') {
                    if (m_input_buffer == "5") {
                        PK_INFO("--- ENEMY TEAM ---");
                        for (const auto& p : m_state.opponent_team()) {
                            PK_INFO("{} | {}/{} HP | Active: {}", p.name, p.hp_current, p.hp_max, p.active);
                        }
                    } else if (m_input_buffer == "6") {
                        PK_INFO("--- YOUR TEAM ---");
                        for (const auto& p : m_state.your_team()) {
                            PK_INFO("{} | {}/{} HP | Active: {}", p.name, p.hp_current, p.hp_max, p.active);
                        }
                    } else if (!m_input_buffer.empty()) {
                        m_client->send(m_battle_room + "|/choose move " + m_input_buffer);
                    }
                    m_input_buffer.clear();
                } else if (c == 127) {
                    // backspace
                    if (!m_input_buffer.empty()) m_input_buffer.pop_back();
                } else {
                    m_input_buffer += c;
                }
                return true;
            });
        } );
        
        // TODO: change to event system like input
        m_client->on_message = [this](const protocol::Message& msg) {
            on_message(msg);
        };

        m_input->start();
        m_running = true;
        return true;
    }

    void PsApp::run() {
        m_client->run();
    }

    void PsApp::shutdown() {
        m_input->stop();
        m_client->shutdown();
        m_running = false;
    }

    void PsApp::on_message(const protocol::Message& msg) {
        if (!msg.room_id.empty() && msg.room_id.length() > m_battle_room.length()) {
            m_battle_room = msg.room_id;
        }
        m_state.apply(msg);
        if (msg.type == "request") {
            on_battle_request(msg);
        } 
    }

    void PsApp::on_battle_request(const protocol::Message& msg) {
        std::string dashboard = build_battle_ui();
        m_input->request(dashboard);
    }

    std::string PsApp::build_battle_ui() {
        std::stringstream ss;

        ss << "\033[2J\033[1;1H"; 

        ss << "=================================================\n";
        ss << "  POKEMON SHOWDOWN CLI  | Room: " << m_battle_room << "\n";
        ss << "=================================================\n\n";

        // OPPONENT TEAM
        ss << "--- OPPONENT TEAM ---\n";
        for (const auto& p : m_state.opponent_team()) {
            ss << (p.active ? " [ACTIVE] " : " [BENCH]  ") << p.name << "\n";
        }
        ss << "\n";

        // YOUR TEAM
        ss << "--- YOUR TEAM ---\n";
        auto& team = m_state.your_team();
        for (size_t i = 0; i < team.size(); i++) {
            const auto& p = team[i];
            ss << " [s" << (i+1) << "] " 
               << (p.active ? "*ACTIVE* " : "         ")
               << p.name << " (" << p.hp_current << "/" << p.hp_max << " HP)\n";
        }
        ss << "\n";

        // AVAILABLE MOVES 
        ss << "--- AVAILABLE ACTIONS ---\n";
        auto& moves = m_state.available_moves();
        for (size_t i = 0; i < moves.size(); i++) {
            ss << " [" << (i+1) << "] " << moves[i].name 
               << " (" << moves[i].pp << "/" << moves[i].max_pp << " PP)";
            if (moves[i].disabled) ss << " [DISABLED]";
            ss << "\n";
        }
        
        ss << "=================================================\n";
        return ss.str();
    }

    
}