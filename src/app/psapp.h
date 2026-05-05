#pragma once

#include "protocol/psclient.h"
#include "protocol/message.h"
#include "protocol/battlestate.h"
#include "input/cl_input.h"
#include "core/layerstack.h"
#include "core/event/event.h"
#include "core/event/event_sink.h"
#include "core/event/command_event.h"
#include "core/event/message_event.h"
#include "core/event/login_event.h"
#include "core/event/key_event.h"
#include "core/event/layer_event.h"
#include "util/util.h"

#include <string>
#include <sstream>
#include <iostream>

namespace pkm {

    // MenuLayer: bottom of stack, always present
    // handles lobby state: searching, waiting, main menu input
    class MenuLayer : public Layer {
    public:
        MenuLayer(Ref<protocol::PsClient> client, PsApp::EventSubmitFn submit)
            :   Layer("MenuLayer") 
              , m_client(client)
              , m_submit(std::move(submit)) {}

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
                    // TODO:
                    // create battle layer then on attach it should call search
                    m_client->send("|/search gen9randombattle");
                } else if (cmd == "2") {
                    PK_INFO("[Menu] Adding login overlay for logging in"); 
                    m_submit(MakeScope<LoginEvent(true)>);
                } else if (cmd == "3") {
                    PK_INFO("[Menu] Adding login overlay for signup"); 
                    m_submit(MakeScope<LoginEvent(false)>);
                    
                } else if (cmd == "q") {
                    PK_INFO("[Menu] Quitting...");
                }
                return true;
            });

            dispatcher.Dispatch<MessageEvent>([this](MessageEvent& event) {
                const auto& msg = event.get_msg();
                if (msg.type == "updateuser") {
                    PK_INFO("[Menu] Logged in as: {}", msg.args[0]);
                } else if (msg.type == "win") {
                    std::cout << "\nWinner: " << msg.args[0] << '\n';
                }
                return false; // dont consume, PsApp needs updatesearch too
            });
        }

        virtual void on_render() override;

    private:
        Ref<protocol::PsClient> m_client;
        PsApp::EventSubmitFn m_submit;
    };


    class LoginOverlay : public Layer {
        public:
            LoginOverlay(Ref<protocol::PsClient> client, bool is_login, PsApp::EventSubmitFn submit) : 
                  m_client(client)
                , m_is_logging_in(is_login) 
                , m_submit(std::move(submit)) {}

            virtual void on_attach() override {
                PK_INFO("[LoginOverlay] Attached");
            }

            virtual void on_detach() override {
                PK_INFO("[LoginOverlay] Detached");
            }

            virtual void on_event(Event& e) override {
                EventDispatcher dispatcher(e);

                dispatcher.Dispatch<CommandEvent>([this](CommandEvent& event) {
                    const std::string& cmd = event.get_command();
                    const std::vector<std::string> tokens = tokenize(cmd);
                    PK_INFO("[LoginOverlay] Command Recieved: '{}'", cmd);

                    if (tokens.size() != 1) { 
                        std::cout << "Invalid Input, enter again!\n"; 
                        return true; 
                    }
                    
                    if (!m_entered_username) {
                        bool success = m_client->set_username(tokens[0]);
                        if (!success) { 
                            std::cout << "Invalid username, retry!\n"; 
                            return true; 
                        }
                        std::cout << "Username set: " << tokens[0] << '\n'; 
                        m_entered_username = true;
                    } else {
                        bool success = m_client->set_password(tokens[0]);
                        if (!success) { 
                            std::cout << "Invalid password, retry!\n"; 
                            return true; 
                        }
                        std::cout << "Password set ...\n";
                        m_entered_password = true;
                    }
                    
                    return true; // dont propogate command to next layers
                });
            }

            virtual void on_render() override;

        private:
            Ref<protocol::PsClient> m_client;
            bool m_is_logging_in{false};
            bool m_entered_username{false};
            bool m_entered_password{false};
            PsApp::EventSubmitFn m_submit;

            
    };

    // BattleLayer - pushed on top when battle starts
    // owns BattleState and battle room ID
    // popped by PsApp when win/tie received
    class BattleLayer : public Layer {
    public:
        BattleLayer(Ref<protocol::PsClient> client, const std::string& battle_room, PsApp::EventSubmitFn submit)
            : Layer("BattleLayer")
            , m_client(client)
            , m_battle_room(battle_room)
            , m_submit(std::move(submit))
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

        virtual void on_render() override;

        const protocol::BattleState& get_battle_state() const { return m_state; }
        const std::string& get_battle_room() const { return m_battle_room; }

    private:

    private:
        Ref<protocol::PsClient>  m_client;
        protocol::BattleState    m_state;
        std::string              m_battle_room;
        bool                     m_timer_active = false;
        PsApp::EventSubmitFn     m_submit;
    };





    // PsApp, owns everything, drives the main loop
    class PsApp : public EventSink {
    public:

        using EventSubmitFn = std::function<void(Scope<Event>)>;

        PsApp();
        ~PsApp() = default;

        bool init();
        void run();
        
        void on_update();
        void on_render();

        void submit(Event& e) override {
            m_event_queue.push(std::move(e));
        }

    private:
        void shutdown();

        void process_network();
        void poll();
        void push_to_layers(Event& e);
        void on_network_message(const protocol::Message& msg);

        // move these into the layers

    private:
        Ref<protocol::PsClient>            m_client;
        Scope<CLInput>                     m_input;

        LayerStack                         m_layerstack;

        pkm::SPSCQueue<protocol::Message>  m_network_queue{256};
        // TODO: introduce multi threaded event queue, priority, etc
        pkm::SPSCQueue<Scope<Event>>       m_event_queue{64};

        bool         m_running;
    };

}
