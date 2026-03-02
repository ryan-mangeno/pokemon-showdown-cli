#include <pkmpch.h>

#include "connection.h"

#include <core/logger.h>

namespace pkm::net {
    
    Result<boost::asio::ip::tcp::resolver::results_type>
    resolve(boost::asio::io_context& ioc,
            const std::string& host,
            const std::string& port) {
        
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver resolver(ioc);
        auto results = resolver.resolve(host, port, ec);
        if (ec) {
            PK_ERROR("Failed to resolve host {0} at port {1}: {2}", host, port, ec.message());
            return ec;
        } else {
            PK_INFO("Resolves host {0} at port {1}", host, port);
        }
        return results;
    }

    // TCP connect + TLS handshake
    // takes the stream by ref and sets it up in place
    bool connect(WssStream& stream,
                 const boost::asio::ip::tcp::resolver::results_type& endpoints,
                 const std::string& host) {
        return true; 
    }    

    // WebSocket upgrade handshake
    bool handshake(WssStream& stream,
                   const std::string& host,
                   const std::string& path) {
        return true;
    }
}

