#include <pkmpch.h>

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

    PsClient::PsClient()
        : m_initialized(false)
        , m_running(false)
        , m_logged_in(false)
        , m_searching(false)
        , m_in_battle(false)
        , m_ws(nullptr)
    {}

    bool PsClient::init() {
        if (m_initialized) {
            PK_WARN("Already initialized");
            return true;
        }

        pkm::load_env("config/.env");

        pkm::net::NetConfig ncfg;
        pkm::JsonLoader::load(ncfg, NET_CONFIG_PATH.c_str());
        m_ws = MakeRef<pkm::net::WsClient>(ncfg);

        if (m_ws) {
            m_initialized = true;
            return true;
        }

        PK_ERROR("Failed to create websocket client!");
        return false;
    }

    void PsClient::start() {
        if (!m_ws->connect()) {
            PK_ERROR("Failed to connect!");
            return;
        }

        m_running = true;
        m_network_thread = std::thread(&PsClient::network_loop, this);
    }

    void PsClient::stop() {
        m_running = false;
        // ioc stop wakes up run() so the thread can exit
        m_ws->get_ioc().stop();
        if (m_network_thread.joinable()) {
            m_network_thread.join();
        }
        m_ws->close();
    }

    bool PsClient::poll(Message& out) {
        // called by main thread to drain inbound queue
        // handle auth messages here since they're pure networking
        Message msg;
        if (!m_inbound.pop(msg)) return false;

        if (msg.type == "challstr") {
            on_chall_str(msg);
        } else if (msg.type == "updateuser") {
            on_update_user(msg);
        }

        // give the message to the caller regardless
        // layers will handle everything else
        out = std::move(msg);
        return true;
    }

    void PsClient::send(const std::string& msg) {
        PK_TRACE("Sending: {}", msg);
        m_ws->send(msg);
    }

    void PsClient::network_loop() {
        PK_INFO("Network thread started");
        m_ws->start_read_loop(m_inbound, parse_message);
        m_ws->get_ioc().run(); // blocks here until ioc is stopped
        PK_INFO("Network thread exiting");
    }

    void PsClient::on_chall_str(const Message& msg) {
        if (m_logged_in) return;

        std::string full_challstr = msg.args[0] + "|" + msg.args[1];
        const char* user = std::getenv("PS_USERNAME");
        const char* pass = std::getenv("PS_PASSWORD");

        if (!user || !pass) {
            PK_ERROR("Login credentials missing from .env!");
            return;
        }

        std::string token = pkm::net::request_assertion(user, pass, full_challstr);
        if (!token.empty()) {
            PK_INFO("Verifying challstr...");
            send(std::string("|/trn ") + user + ",0," + token);
        } else {
            PK_ERROR("Unable to request assertion");
        }
    }

    void PsClient::on_update_user(const Message& msg) {
        if (msg.args.empty()) return;
        bool is_named = (msg.args.size() > 1 && msg.args[1] == "1");
        PK_INFO("Username: {} | Authenticated: {}", msg.args[0], is_named);
        if (is_named) m_logged_in = true;
    }
}