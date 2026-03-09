#pragma once

#include "net/wsclient.h"

#include "core/defines.h"
#include "message.h"

namespace pkm::protocol {

    class PsClient {
        public:
            PsClient();
            ~PsClient() = default;

            bool init();
            void shutdown();

            void run();
        
        private:
            void dispatch(const Message& msg);
            void on_update_user(const Message& msg);
            void on_chall_str(const Message& msg);
            void on_update_search(const Message& msg);
            void on_battle(const Message& msg);
            void on_request(const Message& msg);

        private:
            Ref<pkm::net::WsClient> m_ws;

            bool m_connected;
            bool m_in_battle;
            int32_t m_battle_room;

    };
    

}
