#include <pkmpch.h>

#include "wsclient.h"
#include "util/json_loader.h"
#include "net/sslcontext.h"

namespace pkm::net {
    
    WsClient::WsClient(const NetConfig& config) : m_ioc(), m_config(config), m_websocket(nullptr) {
        
        SSLContext& ssl_ctx = SSLContext::get();
        ssl_ctx.init();
        
        
        auto& [port, host, ws_path] = resolve(m_ioc)

        auto results = pkm::net::resolve(ioc, host, port);
        if (!ok(results)) {
            PK_ERROR("Could not initialize Client, please retry!");
        } else {
            m_endpoints = value(results);

            m_websocket = WssSocket(m_ioc, ssl_ctx.native_ctx()); 
            SSL_set_tlsext_host_name(m_websocket.next_layer().native_handle(), host.c_str());
            
             
        }
        
    }

    WsClient::~WsClient() {

    }
    
    bool WsClient::connect() {

        return ::pkm::net::connect(m_websocket, m_endpoints, m_config.ps_server_url, m_config.ps_websocket_path);
        
    }

    void WsClient::close() {
        boost::beast::error_code ec;
        m_websocket.close(websocket::close_code::normal, ec);
        
        if (ec) {
            PK_ERROR("Failed to close connection!");
        } else {
            PK_INFO("Closed connection successfully!");
        }
    }

}
