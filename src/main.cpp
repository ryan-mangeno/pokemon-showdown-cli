#include "pkmpch.h"

#include <iostream>
#include <string>
#include <net/sslcontext.h> 

namespace beast     = boost::beast;
namespace websocket = beast::websocket;
namespace http      = beast::http;
namespace net       = boost::asio;
namespace ssl       = net::ssl;
using tcp = net::ip::tcp;

int main() {
    const std::string host = "sim.smogon.com";
    const std::string port = "443";
    const std::string path = "/showdown/websocket";

    net::io_context ioc;
    pkm::net::SSLContext& ssl_ctx = pkm::net::SSLContext::get();
    ssl_ctx.init();

    tcp::resolver resolver(ioc);
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, ssl_ctx.native_ctx());

    // SNI
    SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str());

    // Connect
    auto results = resolver.resolve(host, port);
    beast::get_lowest_layer(ws).connect(results);
    ws.next_layer().handshake(ssl::stream_base::client);
    ws.handshake(host, path);

    std::cout << "Connected!\n";

    // read first message, should be the challstr
    beast::flat_buffer buf;
    ws.read(buf);
    std::string msg = beast::buffers_to_string(buf.data());
    std::cout << "Received:\n" << msg << "\n";

    beast::error_code ec;
    ws.close(websocket::close_code::normal, ec);
    return 0;
}
