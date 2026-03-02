#pragma once

#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/ssl.hpp>

namespace pkm::net {
    // WebSocket Secure Stream
    using WssStream =
    boost::beast::websocket::stream<
        boost::beast::ssl_stream<
            boost::beast::tcp_stream
        >
    >;
    
    template <typename T>
    using Result = std::variant<T, boost::beast::error_code>;
    
    // helper to check success
    template <typename T>
    bool ok(const Result<T>& r) {
        return std::holds_alternative<T>(r);
    }
    
    // helper to get value
    template <typename T>
    T& value(Result<T>& r) {
        return std::get<T>(r);
    }
    
    // helper to get error code
    template <typename T>
    boost::beast::error_code error(const Result<T>& r) {
        return std::get<boost::beast::error_code>(r);
    }
}
