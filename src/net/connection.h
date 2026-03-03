#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <string>

#include "types.h"

namespace pkm::net {
    
    // DNS, turns hostname into list of IP endpoints
    Result<boost::asio::ip::tcp::resolver::results_type>
    resolve(boost::asio::io_context& ioc,
            const std::string& host,
            const std::string& port);

    // TCP connect + TLS handshake
    // takes the stream by ref and sets it up in place
    bool connect(WssStream& stream,
                 const boost::asio::ip::tcp::resolver::results_type& endpoints,
                 const std::string& host,
                 const std::string& websocket);

    // WebSocket upgrade handshake
    bool handshake(WssStream& stream,
                   const std::string& host,
                   const std::string& path);
}
