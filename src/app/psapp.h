#pragma once

#include "protocol/psclient.h"
#include "protocol/message.h"
#include "protocol/battlestate.h"
#include "input/input.h"
#include "core/layerstack.h"
#include "core/event/event.h"
#include "core/event/command_event.h"
#include "core/event/message_event.h"
#include "core/event/key_event.h"

#include <string>
#include <sstream>

namespace pkm {

    // -------------------------------------------------------
    // MenuLayer — bottom of stack, always present
    // handles lobby state: searching, waiting, main menu input
    // -------------------------------------------------------
    class MenuLayer : public Layer {
    public:
        MenuLayer(Ref<protocol::PsClient> client)
            : Layer("MenuLayer"), m_client(client) {}

        virtual void on_attach() override {
            PK_INFO("[MenuLayer] Attached");
        }

        virtual void on_event(Event& event) override {
            EventDispatcher dispatcher(event);

            dispatcher.Dispatch<CommandEvent>([this](CommandEvent& e) {
                const std::string& cmd = e.get_command();
                if (cmd == "1") {
                    PK_INFO("[Menu] Searching for battle...");
                    m_client->send("|/search gen9randombattle");
                } else if (cmd == "q" || cmd == "Q") {
                    PK_INFO("[Menu] Quitting...");
                }
                return true;
            });

            dispatcher.Dispatch<MessageEvent>([this](MessageEvent& e) {
                const auto& msg = e.get_msg();
                if (msg.type == "updateuser") {
                    PK_INFO("[Menu] Logged in as: {}", msg.args[0]);
                }
                return false; // dont consume, PsApp needs updatesearch too
            });
        }

    private:
        Ref<protocol::PsClient> m_client;
    };


    // -------------------------------------------------------
    // BattleLayer — pushed on top when battle starts
    // owns BattleState and battle room ID
    // popped by PsApp when win/tie received
    // -------------------------------------------------------
    class BattleLayer : public Layer {
    public:
        BattleLayer(Ref<protocol::PsClient> client, const std::string& battle_room)
            : Layer("BattleLayer")
            , m_client(client)
            , m_battle_room(battle_room)
        {}

        virtual void on_attach() override {
            PK_INFO("[BattleLayer] Attached - room: {}", m_battle_room);
        }

        virtual void on_detach() override {
            PK_INFO("[BattleLayer] Detached");
        }

        virtual void on_event(Event& event) override {
            EventDispatcher dispatcher(event);

            dispatcher.Dispatch<CommandEvent>([this](CommandEvent& e) {
                const std::string& cmd = e.get_command();
                if (cmd == "5") {
                    PK_INFO("--- ENEMY TEAM ---");
                    for (const auto& p : m_state.opponent_team())
                        PK_INFO("  {} | {}/{} HP{}", p.name, p.hp_current, p.hp_max, p.active ? " [ACTIVE]" : "");
                } else if (cmd == "6") {
                    PK_INFO("--- YOUR TEAM ---");
                    for (const auto& p : m_state.your_team())
                        PK_INFO("  {} | {}/{} HP{}", p.name, p.hp_current, p.hp_max, p.active ? " [ACTIVE]" : "");
                } else {
                    m_client->send(m_battle_room + "|/choose move " + cmd);
                }
                return true;
            });

            dispatcher.Dispatch<MessageEvent>([this](MessageEvent& e) {
                const auto& msg = e.get_msg();
                m_state.apply(msg);

                if (msg.type == "request")       on_request(msg);
                else if (msg.type == "turn")     { if (!msg.args.empty()) PK_INFO("=== TURN {} ===", msg.args[0]); }
                else if (msg.type == "faint")    { if (!msg.args.empty()) PK_INFO("{} fainted!", msg.args[0]); }
                else if (msg.type == "move")     { if (msg.args.size() >= 2) PK_INFO("{} used {}", msg.args[0], msg.args[1]); }

                return true;
            });
        }

        const std::string& get_battle_room() const { return m_battle_room; }

    private:
        void on_request(const protocol::Message& msg) {
            if (msg.args.empty() || msg.args[0].empty()) return;
            std::stringstream ss;
            ss << "\n=== YOUR TURN ===\n";
            for (size_t i = 0; i < m_state.available_moves().size(); i++) {
                auto& m = m_state.available_moves()[i];
                ss << " [" << (i+1) << "] " << m.name << " (" << m.pp << "/" << m.max_pp << " PP)";
                if (m.disabled) ss << " [DISABLED]";
                ss << "\n";
            }
            ss << " [5] Enemy Team  [6] Your Team\n";
            PK_INFO("{}", ss.str());
        }

    private:
        Ref<protocol::PsClient>  m_client;
        protocol::BattleState    m_state;
        std::string              m_battle_room;
    };


    // -------------------------------------------------------
    // PsApp, owns everything, drives the main loop
    // -------------------------------------------------------
    class PsApp {
    public:
        PsApp();
        ~PsApp() = default;

        bool init();
        void run();
        void shutdown();

    private:
        void process_network();
        void process_input();
        void push_to_layers(Event& e);
        void on_network_message(const protocol::Message& msg);

    private:
        Ref<protocol::PsClient>            m_client;
        Scope<Input>                       m_input;
        LayerStack                         m_layerstack;
        BattleLayer*                       m_battle_layer{nullptr}; // non-owning ptr for push/pop

        pkm::SPSCQueue<protocol::Message>  m_network_queue{256};
        pkm::SPSCQueue<std::string>        m_input_queue{64};

        std::string  m_input_buffer;
        bool         m_running;
        bool         m_in_battle;
    };

}