#include <pkmpch.h>

#include "cl_input.h"
#include "core/logger.h"
#include "core/event/key_event.h"

#include <iostream>
#include <linenoise.h>

namespace pkm {

    CLInput::CLInput()
        : m_prompt(""), m_running(false) {}

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
        linenoiseSetMultiLine(1);
        linenoiseHistorySetMaxLen(100);

        while (m_running) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] {
                return m_requested.load() || !m_running.load();
            });

            if (!m_running) break;
            std::string prompt = m_prompt;
            m_requested = false;
            lock.unlock();

            std::cout << prompt << std::flush;

            char buf[1024];
            struct linenoiseState ls;
            linenoiseEditStart(&ls, -1, -1, buf, sizeof(buf), "> ");

            bool done = false;
            while (!done && m_running) {
                fd_set readfds;
                struct timeval tv = {0, 100000};
                FD_ZERO(&readfds);
                FD_SET(ls.ifd, &readfds);

                if (select(ls.ifd + 1, &readfds, NULL, NULL, &tv) > 0) {
                    char* line = linenoiseEditFeed(&ls);
                    if (line != linenoiseEditMore) {
                        if (line) {
                            linenoiseHistoryAdd(line);
                            for (char* p = line; *p; p++) {
                                if (m_callback) {
                                    KeyTypedEvent e(static_cast<int>(*p));
                                    m_callback(e);
                                }
                            }
                            if (m_callback) {
                                KeyTypedEvent e(static_cast<int>('\n'));
                                m_callback(e);
                            }
                            free(line);
                        }
                        done = true;
                    }
                }
            }
            linenoiseEditStop(&ls);
        }
    }
}