#pragma once

#include "protocol/psclient.h"
#include "protocol/message.h"
#include "input/input.h"
#include "protocol/battlestate.h"

#include <string>

namespace pkm {

    class PsApp {
    public:
        PsApp();
        ~PsApp() = default;

        bool init();
        void run();
        void shutdown();

    private:
        void on_message(const protocol::Message& msg);
        void on_battle_request(const protocol::Message& msg);
        std::string build_battle_ui();
        
    private:
        Scope<protocol::PsClient>   m_client;
        Scope<Input>                m_input;
        protocol::BattleState       m_state;

        // TODO: decide how to move these out
        std::string m_battle_room;
        std::string m_input_buffer;
        bool m_running;
    };

}