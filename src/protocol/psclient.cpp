#include <pkmpch.h>

#include "psclient.h"
#include "util/json_loader.h"
#include "core/logger.h"
#include "net/netconfig.h"
#include "net/wsclient.h"
#include "protocol/parser.h"

#include <nlohmann/json.hpp>

namespace pkm::protocol {

    PsClient::PsClient() : m_initialized(false), m_in_battle(false), m_battle_room(""), m_connected(false), m_ws(nullptr) {}

    bool PsClient::init() {
        if (m_initialized) {
            PK_WARN("Already connected, no need for initializion twice");
            return true;
        }
        
        pkm::net::NetConfig ncfg;
        pkm::JsonLoader::load(ncfg, NET_CONFIG_PATH.c_str());
        m_ws = MakeRef<pkm::net::WsClient>(ncfg);
        
        if(m_ws) {
            m_initialized = true;
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
                auto msgs = parse_message(m_ws->receive());
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
        else if (msg.type == "win")          on_win(msg);
    }

    void PsClient::on_update_user(const Message& msg) {
        PK_TRACE("On Update");
        PK_INFO("Username: {}", msg.args[0]);
        m_ws->send("|/search gen9randombattle");
        msg.print();
    }

    void PsClient::on_chall_str(const Message& msg) {
        PK_TRACE("On Chall");
    }

    void PsClient::on_update_search(const Message& msg) {
        if (msg.args[0].empty()) return;
        
        auto j = nlohmann::json::parse(msg.args[0]);
        if (!j["games"].is_null() && !m_in_battle) {
            m_battle_room = j["games"].begin().key();
            m_in_battle = true;
            PK_INFO("Battle room assigned: {}", m_battle_room);
            m_ws->send("|/join " + m_battle_room);
        } 
    }

    void PsClient::on_battle(const Message& msg) {
        PK_TRACE("On Battle");
        m_battle_room = msg.args[0];
        m_ws->send("|/join " + m_battle_room);
    }

    void PsClient::on_win(const Message& msg) {
        PK_INFO("Winner: {}", msg.args[0]);
        m_connected = false;
    }

    void PsClient::on_request(const Message& msg) {
        PK_TRACE("On Request");

        if (msg.args[0].empty()) return;
        
        auto j = nlohmann::json::parse(msg.args[0]);

        msg.print();
    
        if (j.contains("forceSwitch")) {
            auto& side = j["side"]["pokemon"];
            for (int i=0; i<side.size(); ++i) {
                // force switch to valid pokemon
                // TODO: let user pick
                std::string condition = side[i]["condition"];
                if (condition != "0 fnt") {
                    m_ws->send(m_battle_room + "|/choose switch " + std::to_string(i+1));
                }
            }
        } else if (j.contains("active")) {
            auto& moves = j["active"][0]["moves"];
            // just force select any valid move
            // TODO: let user pick move
            for (int i=0 ; i<moves.size(); ++i) {
                if (!moves[i]["disabled"].get<bool>()) {
                    m_ws->send(m_battle_room + "|/choose move " + std::to_string(i+1)); 
                }
            }
        }
    }

 
    
}
