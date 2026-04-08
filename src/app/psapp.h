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
#include "util/util.h"

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

        virtual void on_event(Event& e) override {
            EventDispatcher dispatcher(e);

            dispatcher.Dispatch<CommandEvent>([this](CommandEvent& event) {
                PK_INFO("[MenuLayer] Command received: '{}'", event.get_command());
                const std::string& cmd = event.get_command();
                if (cmd == "1") {
                    PK_INFO("[Menu] Searching for battle...");
                    m_client->send("|/search gen9randombattle");
                } else if (cmd == "q" || cmd == "Q") {
                    PK_INFO("[Menu] Quitting...");
                }
                return true;
            });

            dispatcher.Dispatch<MessageEvent>([this](MessageEvent& event) {
                const auto& msg = event.get_msg();
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

        virtual void on_event(Event& e) override {
            EventDispatcher dispatcher(e);

            dispatcher.Dispatch<CommandEvent>([this](CommandEvent& event) {
                const std::string& cmd = event.get_command();
                const std::vector<std::string> tokens = tokenize(cmd);
                PK_INFO("[BattleLayer] Command received: '{}'", cmd);

                if (tokens.size() == 0) {
                    PK_WARN("Empty Command Recieved!");
                    return true;
                }
                
                if (tokens[0] == "1" || tokens[0] == "2" || tokens[0] == "3" || tokens[0] == "4") {
                    if (m_state.is_force_switch()) {
                        m_client->send(m_battle_room + "|/choose switch " + cmd[0]);
                    } else {
                        std::string tokenize_str = (tokens.size() == 2 && tokens[1] == "tera") ? " terastallize" : "";
                        m_client->send(m_battle_room + "|/choose move " + cmd[0] + tokenize_str);
                    }
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
                } else if (cmd == "s1" || cmd == "s2" || cmd == "s3" || 
                           cmd == "s4" || cmd == "s5" || cmd == "s6") {
                    std::string slot = cmd.substr(1); // strip the 's'
                    m_client->send(m_battle_room + "|/choose switch " + slot);
                }
                // do not propogate command events further for now
                return true;
            });

            dispatcher.Dispatch<MessageEvent>([this](MessageEvent& event) {
                const auto& msg = event.get_msg();
                m_state.apply(msg);
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
        
        void on_update();
        void on_render();

    private:
        void process_network();
        void poll();
        void push_to_layers(Event& e);
        void on_network_message(const protocol::Message& msg);

        std::string build_battle_ui();
        std::string build_main_menu_ui();

    private:
        Ref<protocol::PsClient>            m_client;

        // TODO: maybe ifdef based on headless/not 
        Scope<CLInput>                     m_input;

        LayerStack                         m_layerstack;
        BattleLayer*                       m_battle_layer{nullptr}; // non-owning ptr for push/pop

        pkm::SPSCQueue<protocol::Message>  m_network_queue{256};
        // TODO: introduced multi threaded event queue, priority, etc
        pkm::SPSCQueue<Scope<Event>>       m_event_queue{64};

        bool         m_running;
        bool         m_in_battle;
        std::string  m_ui;
    };

}