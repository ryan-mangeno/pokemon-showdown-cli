#include <pkmpch.h>

#include "psclient.h"
#include "util/json_loader.h"
#include "core/logger.h"
#include "net/netconfig.h"
#include "net/wsclient.h"
#include "protocol/parser.h"


namespace pkm::protocol {

    PsClient::PsClient() : m_connected(false), m_ws(nullptr) {}

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
    }

    void PsClient::run() {
        if (m_ws->connect()) {
            auto msgs = pkm::protocol::parse(m_ws->receive());
            for (auto& msg : msgs) {
                PK_INFO("Room: {0}", msg.room_id);
                PK_INFO("Type: {0}", msg.type);
                for (auto& arg : msg.args) {
                    PK_INFO("Arg: {0}", arg);
                }
            }
        }
    }
}
