#include <pkmpch.h>

#include <cstdlib>

#include "psclient.h"
#include "util/json_loader.h"
#include "core/logger.h"
#include "net/netconfig.h"
#include "net/wsclient.h"
#include "protocol/parser.h"
#include "util/env_loader.h"
#include "net/auth.h"

#include <nlohmann/json.hpp>

namespace pkm::protocol {

    PsClient::PsClient() : m_initialized(false), m_in_battle(false), m_searching(false), m_battle_room(""), m_connected(false), m_ws(nullptr) {}

    bool PsClient::init() {
        if (m_initialized) {
            PK_WARN("Already connected, no need for initializion twice");
            return true;
        }

        pkm::load_env("config/.env");
        
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

        if (!m_ws->connect()) {
            PK_ERROR("Failed to run Client!");
            return;
        }
        
        m_connected = true;
        m_network_thread = std::thread(&PsClient::network_loop, this);

        while (m_connected) {
            Message msg;
            if (m_inbound.pop(msg)) {
                if (on_message) on_message(msg);
                dispatch(msg);
            }
        }

        m_network_thread.join();
    }

    void PsClient::login() {
        if (m_logged_in) {
            // TODO: send chall str if we are already logged in
        }

        const char* user = std::getenv("PS_USERNAME");
        const char* pass = std::getenv("PS_PASSWORD");

        if (user && pass) {
            PK_INFO("Logging in as: {}", user);

        } else {
            PK_ERROR("Login credentials missing from .env!");
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
        std::string username = msg.args[0];
        bool is_named = (msg.args[1] == "1");

        PK_INFO("Username: {} | Authenticated: {}", username, is_named);

        // only initiate a search if we are officially logged in
        if (is_named) {
            m_logged_in = true; 

            if (!m_in_battle && !m_searching) {
                PK_INFO("Login successful! Queueing for Gen 9 Random Battle...");
                m_searching = true;
                send("|/search gen9randombattle");
            }
        }
    }

    void PsClient::on_chall_str(const Message& msg) {
        
        if (!m_in_battle && !m_searching) {
            std::string full_challstr = msg.args[0] + "|" + msg.args[1];
            
            std::string user = std::getenv("PS_USERNAME");
            std::string pass = std::getenv("PS_PASSWORD");
            std::string token = pkm::net::request_assertion(user, pass, full_challstr);

            if (!token.empty()) {
                PK_INFO("Verifying Challstr ...");
                send("|/trn " + user + ",0," + token);
            } else {
                PK_ERROR("Unnable to request assertion ...");
            }
        }
    }

    void PsClient::on_update_search(const Message& msg) {
        if (msg.args[0].empty()) return;
        
        auto j = nlohmann::json::parse(msg.args[0]);
        
        if (j.contains("games") && !j["games"].is_null() && !m_in_battle) {
            m_battle_room = j["games"].begin().key();
            m_in_battle = true;
            m_searching = false;
            PK_INFO("Battle room assigned: {}", m_battle_room);
            send("|/join " + m_battle_room);
        } 
    }

    void PsClient::on_battle(const Message& msg) {
        m_battle_room = msg.args[0];
        send("|/join " + m_battle_room);
    }

    void PsClient::on_win(const Message& msg) {
        PK_INFO("Winner: {}", msg.args[0]);
        m_connected = false;
        send("|/leave " + m_battle_room); 
        m_battle_room = "";
    }

    void PsClient::on_request(const Message& msg) {
        if (msg.args.empty() || msg.args[0].empty()) return;
    }

    void PsClient::network_loop() {
        PK_INFO("Network thread started.");

        // starting continuous read cycle, passing in inbound spsc queue and parser func
        m_ws->start_read_loop(m_inbound, parse_message);

        // run the asio event loop - this blocks and handles the strand/async work
        m_ws->get_ioc().run(); 
    }

    void PsClient::send(const std::string& msg) {
        m_ws->send(msg);
    }

}
