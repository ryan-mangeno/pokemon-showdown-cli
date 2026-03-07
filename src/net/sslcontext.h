#pragma once

#include "core/defines.h"
#include <boost/asio/ssl/context.hpp>

namespace pkm::net {
   
    // should be initialized once at startup and reused for every tls connection
    class SSLContext {
        public:

            inline static SSLContext& get() {
                static SSLContext instance;
                return instance;
            }

            inline boost::asio::ssl::context& native_ctx() {
                return *m_client;
            }

            bool init();
            bool shutdown();

        private:
            SSLContext() = default;
            ~SSLContext() = default;

        private:
            bool m_initialized = false;
            Scope<boost::asio::ssl::context> m_client;

    };


} // namespace pkm
