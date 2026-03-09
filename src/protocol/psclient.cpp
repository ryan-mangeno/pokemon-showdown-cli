#include <pkmpch.h>

#include "psclient.h"
#include "util/json_loader.h"
#include "core/logger.h"
#include "net/netconfig.h"
#include "net/wsclient.h"
#include "protocol/parser.h"

namespace pkm::protocol {

    PsClient::PsClient() : m_in_battle(false), m_battle_room(0), m_connected(false), m_ws(nullptr) {}

    bool PsClient::init() {
        if (m_connected) {
            PK_WARN("Already connected, no need for initializion twice");
            return true;
        }
        
        pkm::net::NetConfig ncfg;
        pkm::JsonLoader::load(ncfg, NET_CONFIG_PATH.c_str());
        m_ws = MakeRef<pkm::net::WsClient>(ncfg);
        
        if(m_ws) {
            m_connected = true;
            return true;
        } else {
            PK_ERROR("Failed to created websocket client!");
            return false;
        }
    }

    void PsClient::shutdown() {
        m_ws->close();   
        m_connected = false;
    }

    void PsClient::run() {
        if (m_ws->connect()) {
            m_connected = true;
            while (m_connected) {
                auto msgs = pkm::protocol::parse(m_ws->receive());
                for (auto& msg : msgs) {
                    dispatch(msg);
                }
            }
        } else {
            PK_ERROR("Failed to connect PsClient!");
        }
    }
    
    void PsClient::dispatch(const Message& msg) {
        if (msg.type == "updateuser")        on_update_user(msg);
        else if (msg.type == "challstr")     on_chall_str(msg);
        else if (msg.type == "updatesearch") on_update_search(msg);
        else if (msg.type == "b")            on_battle(msg);
        else if (msg.type == "request")      on_request(msg);
    }

    void PsClient::on_update_user(const Message& msg) {
        PK_TRACE("On Update");
        PK_INFO("Username: {}", msg.args[0]);
    }

    void PsClient::on_chall_str(const Message& msg) {
        PK_TRACE("On Chall");
    }

    void PsClient::on_update_search(const Message& msg) {
        PK_TRACE("On Update");
    }

    void PsClient::on_battle(const Message& msg) {
        PK_TRACE("On Update");
    }

    void PsClient::on_request(const Message& msg) {
        PK_TRACE("On Request");
    }

 
    
}
