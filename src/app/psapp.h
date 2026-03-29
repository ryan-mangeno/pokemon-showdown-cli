#pragma once

#include "protocol/psclient.h"
#include "protocol/message.h"
#include "protocol/battlestate.h"
#include "input/cl_input.h"
#include "core/layerstack.h"
#include "core/event/event.h"
#include "core/event/command_event.h"
#include "core/event/message_event.h"
#include "core/event/key_event.h"

#include <string>
#include <sstream>

namespace pkm {

    // -------------------------------------------------------
    // MenuLayer: bottom of stack, always present
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
                PK_INFO("[MenuLayer] Command received: '{}'", e.get_command());
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
    // BattleLayer - pushed on top when battle starts
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
                PK_INFO("[BattleLayer] Command received: '{}'", cmd);
                
                // TODO: if user enters a command that is invalid we should pass to ui to be rendered

                // temporary but should be a battle way to handle this
                if (cmd == "1" || cmd == "2" || cmd == "3" || cmd == "4") {
                    m_client->send(m_battle_room + "|/choose move " + cmd);
                } else if ( cmd == "f") {
                    m_client->send(m_battle_room + "|/forfeit");
                }  else if (cmd == "t") {
                    if (m_timer_active) {
                        m_client->send(m_battle_room + "|/timer off");
                        m_timer_active = false;
                    } else {
                        m_client->send(m_battle_room + "|/timer on");
                        m_timer_active = true;
                    }
                }
                // do not propogate command events further for now
                return true;
            });

            dispatcher.Dispatch<MessageEvent>([this](MessageEvent& e) {
                const auto& msg = e.get_msg();
                m_state.apply(msg);
                /* TODO: decide how i want to handle these
                if (msg.type == "request")       on_request(msg);
                else if (msg.type == "turn")     { if (!msg.args.empty()) PK_INFO("=== TURN {} ===", msg.args[0]); }
                else if (msg.type == "faint")    { if (!msg.args.empty()) PK_INFO("{} fainted!", msg.args[0]); }
                else if (msg.type == "move")     { if (msg.args.size() >= 2) PK_INFO("{} used {}", msg.args[0], msg.args[1]); }
                */
                return true;
            });
        }

        const protocol::BattleState& get_battle_state() const { return m_state; }
        const std::string& get_battle_room() const { return m_battle_room; }

    private:

    private:
        Ref<protocol::PsClient>  m_client;
        protocol::BattleState    m_state;
        std::string              m_battle_room;
        bool                     m_timer_active = false;
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
        
        // TODO: should be "rendering" ui in cli and 
        // not use CLInput but Input for m_input
        void on_update();
        void on_render();

    private:
        void process_network();
        void process_input();
        void push_to_layers(Event& e);
        void on_network_message(const protocol::Message& msg);

        std::string build_battle_ui();

    private:
        Ref<protocol::PsClient>            m_client;
        Scope<CLInput>                     m_input;
        LayerStack                         m_layerstack;
        BattleLayer*                       m_battle_layer{nullptr}; // non-owning ptr for push/pop

        pkm::SPSCQueue<protocol::Message>  m_network_queue{256};
        pkm::SPSCQueue<std::string>        m_input_queue{64};

        bool         m_running;
        bool         m_in_battle;
        std::string  m_ui;
    };

}