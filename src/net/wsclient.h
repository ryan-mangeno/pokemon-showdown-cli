#pragma once

#include "net/connection.h"
#include "net/sslcontext.h"
#include "net/types.h"
#include "net/netconfig.h"

#include "util/json_loader.h"

#include "core/defines.h"
#include "net/connection.h"

namespace pkm::net {
    


    class WsClient {
        using ResolverResults = boost::asio::ip::tcp::resolver::results_type;
        public:
            WsClient(const NetConfig& config);
            ~WsClient();

            bool connect();
            void send(const std::string& message);
            std::string recieve();
            void close();
            
        private:
            net::io_context m_ioc;
            websocket::stream<beast::ssl_stream<beast::tcp_stream>> m_websocket;
            NetConfig m_config;
            ResolverResults m_endpoints;

            // TODO:
            // bool m_has_cached_endpoints;
    }


}
