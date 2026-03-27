#pragma once

#include "net/wsclient.h"
#include "core/thread/queue.h"
#include "core/defines.h"
#include "message.h"

#include <thread>
#include <atomic>
#include <functional>

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

            pkm::SPSCQueue<Message>      m_inbound{256};
            pkm::SPSCQueue<std::string>  m_outbound{64};
            std::thread m_network_thread;
    };
}