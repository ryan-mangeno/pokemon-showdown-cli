#include <pkmpch.h>

#include "sslcontext.h"

namespace net = boost::asio;
namespace ssl = net::ssl;

namespace pkm::net {
    
    bool SSLContext::init() {
        if (m_initialized) return true;

        // create the client with tls v1.2
        m_client = MakeScope<ssl::context>(ssl::context::tlsv12_client);
        m_client->set_default_verify_paths();
        m_client->set_verify_mode(ssl::verify_peer);

        m_initialized = true;
        return true;
    }

    bool SSLContext::shutdown() {
        return true;
    }

} // namespace pkm
