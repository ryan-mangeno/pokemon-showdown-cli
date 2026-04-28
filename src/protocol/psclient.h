#pragma once

#include "net/wsclient.h"
#include "core/thread/queue.h"
#include "core/defines.h"
#include "message.h"

#include <thread>
#include <atomic>
#include <functional>
#include <cstring>
#include <algoritm>

namespace pkm::protocol {

    class PsClient {
        public:
            PsClient(); 
            ~PsClient() = default;

            bool init();
            void start();           // connect + spawn network thread, returns immediately
            void stop();            // signal shutdown + join network thread
            void send(const std::string& msg);
            bool poll(Message& out); // main thread calls this to drain inbound
    
            inline bool set_username(const std::string& username) {
                if (username.empty()) {
                    return false;
                }

                if (username.find_first_not_of(" \t\r\n") == std::string::npos) {
                    return false;
                }

                size_t len = std::min(username.length(), sizeof(m_username) - 1);
                std::memcpy(m_username, username.c_str, len);
                m_username[len] = '\0';
                
                return true;
            }

            inline bool set_password(const std::string& password) {
                if (username.empty()) {
                    return false;
                }

                if (username.find_first_not_of(" \t\r\n") == std::string::npos) {
                    return false;
                }

                size_t len = std::min(password.length(), sizeof(m_password) - 1);
                std::memcpy(m_password, password.c_str(), len);
                m_password[len] = '\0';

                return true;
            }

        private:
            void network_loop();
            void on_chall_str(const Message& msg);   
            void on_update_user(const Message& msg); 

        private:
            Ref<pkm::net::WsClient> m_ws;

            bool m_initialized;
            std::atomic<bool> m_running;
            bool m_logged_in;
            bool m_searching;
            bool m_in_battle;
            std::string m_battle_room;
            char m_username[32]{};
            char m_password[32]{};

            pkm::SPSCQueue<Message>      m_inbound{256};
            pkm::SPSCQueue<std::string>  m_outbound{64};
            std::thread m_network_thread;
    };
}
