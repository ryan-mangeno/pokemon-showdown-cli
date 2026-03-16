#include <pkmpch.h>

#include "cl_input.h"
#include "core/logger.h"

#include <iostream>

namespace pkm {

    CLInput::CLInput(std::function<void(const std::string&)> on_response)
        : m_on_response(on_response), m_prompt("") {}

    CLInput::~CLInput() {
        stop();
    }

    void CLInput::start() {
        m_running = true;
        m_thread = std::thread(&CLInput::run, this);
    }

    void CLInput::stop() {
        m_running = false;
        m_cv.notify_all();
        if (m_thread.joinable()) m_thread.join();
    }

    void CLInput::request(const std::string& prompt) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_prompt = prompt;
            m_requested = true;
        }
        m_cv.notify_one();
    }

    void CLInput::run() {
        while (m_running) {
            // sleep until request arrives
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] {
                return m_requested.load() || !m_running.load();
            });

            if (!m_running) break;

            std::string prompt = m_prompt;
            m_requested = false;
            lock.unlock();

            // prompt user and read response
            PK_INFO("{}", prompt.c_str());
            std::string response;
            // TODO: can result in zombie thread if time limit runs out
            // this needs to be more modular
            if (std::getline(std::cin, response)) {
                if (m_on_response) m_on_response(response);
            }
        }
    }
}