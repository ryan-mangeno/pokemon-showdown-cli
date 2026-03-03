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
            PK_INFO("Resolved host {0} at port {1}", host, port);
        }
        return results;
    }

    // TCP connect + TLS handshake
    // takes the stream by ref and sets it up in place
    bool connect(WssStream& stream,
                 const boost::asio::ip::tcp::resolver::results_type& endpoints,
                 const std::string& host,
                 const std::string& websocket) {
        
        boost::system::error_code connect_ec;
        boost::beast::get_lowest_layer(stream).connect(endpoints, connect_ec);
        if (connect_ec) {
            PK_ERROR("Failed to connect to endpoint(s)!");
            return false;
        }

        boost::system::error_code ssl_handshake_ec;
        stream.next_layer().handshake(boost::asio::ssl::stream_base::client, ssl_handshake_ec);
        if (ssl_handshake_ec) {
            PK_ERROR("Failed to execute ssl handshake!");
            return false;
        }
        
        boost::system::error_code host_handshake_ec;
        stream.handshake(host, websocket, host_handshake_ec);
        if (host_handshake_ec) {
            PK_ERROR("Failed to handshake with host!");
            return false;
        }
        
        PK_INFO("Connected to host: {0}", host);
        return true;
    }    

    // WebSocket upgrade handshake
    bool handshake(WssStream& stream,
                   const std::string& host,
                   const std::string& path) {
        return true;
    }
}

