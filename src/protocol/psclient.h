#pragma once

#include "net/wsclient.h"

#include "core/defines.h"

namespace pkm::protocol {

    class PsClient {
        public:
            PsClient();
            ~PsClient() = default;

            bool init();
            void shutdown();

            void run();

        private:
            bool m_connected;
            Ref<pkm::net::WsClient> m_ws;
    };
    

}
