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
#include <net/netconfig.h>
#include <net/wsclient.h>

int main() {

    pkm::Logger::init();
    pkm::net::NetConfig ncfg;
    pkm::JsonLoader::load(ncfg, NET_CONFIG_PATH.c_str());

    // Connect
    pkm::net::connect(ws, resolved, host, path);

    // read first message, should be the challstr
    beast::flat_buffer buf;
    ws.read(buf);
    std::string msg = beast::buffers_to_string(buf.data());
    std::cout << "Received:\n" << msg << "\n";

    beast::error_code ec;
    ws.close(websocket::close_code::normal, ec);
        
    pkm::net::WsClient client(ncfg);
    
    if (client.connect()) {
        auto msg = client.receive();
        std::cout << "Received: " << msg << std::endl;
    }

    return 0;
}
