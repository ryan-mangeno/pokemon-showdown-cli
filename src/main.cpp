#include "pkmpch.h"

#include <iostream>
#include <string>
#include <net/sslcontext.h> 
#include <net/connection.h>
namespace beast     = boost::beast;
namespace websocket = beast::websocket;
namespace http      = beast::http;
namespace net       = boost::asio;
namespace ssl       = net::ssl;
using tcp = net::ip::tcp;

#include <util/util.h>
#include <util/json_loader.h>
#include <core/logger.h>
#include <net/connection.h>

int main() {

    pkm::Logger::init();   
    auto [port, host, path] = pkm::ConfigLoader::load();

    pkm::net::SSLContext& ssl_ctx = pkm::net::SSLContext::get();
    ssl_ctx.init();
    
    net::io_context ioc;
    auto results = pkm::net::resolve(ioc, host, port);
    if (!pkm::net::ok(results)) {
        return 1;
    }
    auto resolved = pkm::net::value(results);
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, ssl_ctx.native_ctx());

    // SNI
    SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.data());

    // Connect
    pkm::net::connect(ws, resolved, host, path);

    // read first message, should be the challstr
    beast::flat_buffer buf;
    ws.read(buf);
    std::string msg = beast::buffers_to_string(buf.data());
    std::cout << "Received:\n" << msg << "\n";

    beast::error_code ec;
    ws.close(websocket::close_code::normal, ec);
    return 0;
}
