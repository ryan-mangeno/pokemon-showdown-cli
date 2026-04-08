#include <pkmpch.h>

#include "cl_input.h"
#include "core/logger.h"
#include "core/event/command_event.h"

#include <iostream>
#include <linenoise.h>

namespace pkm {

    CLInput::CLInput()
        : m_running(false) {}

    void CLInput::start() {
        m_running = true;
        m_thread = std::thread(&CLInput::run, this);
    }

    void CLInput::stop() {
        m_running = false;
        if (m_thread.joinable()) m_thread.join();
    }

    void CLInput::set_input_ui(const std::string& ui) {
        {
            std::lock_guard<std::mutex> lock(m_ui_mutex);
            m_ui_buffer = ui;
            m_ui_dirty = true;
        }
    }

    void CLInput::run() {
        linenoiseSetMultiLine(1);
        linenoiseHistorySetMaxLen(100);

        char buf[1024];
        buf[0] = '\0';
        struct linenoiseState ls;

        std::string active_prompt;
    
        // initial fetch
        {
            std::lock_guard<std::mutex> lock(m_ui_mutex);
            active_prompt = m_ui_buffer;
            m_ui_dirty = false;
        }

        linenoiseEditStart(&ls, -1, -1, buf, sizeof(buf), active_prompt.c_str());

        while (m_running) {
            fd_set readfds;
            struct timeval tv = {0, 100000};
            FD_ZERO(&readfds);
            FD_SET(ls.ifd, &readfds);

            int retval = select(ls.ifd + 1, &readfds, NULL, NULL, &tv);

            if (retval > 0) {
                char* line = linenoiseEditFeed(&ls);
                if (line != linenoiseEditMore) {
                    if (line) {
                        linenoiseHistoryAdd(line);
                        if (m_callback) {
                            CommandEvent e{std::string(line)};
                            m_callback(e);
                        }
                        free(line);
                    }
                    linenoiseEditStop(&ls);
                    linenoiseEditStart(&ls, -1, -1, buf, sizeof(buf), "> ");
                }
            } else {
                // timeout, redraw ui if dirty
                if (m_ui_dirty.load()) {
                    m_ui_dirty = false;
                    linenoiseHide(&ls);
                    std::cout << "\r\n" << m_ui_buffer << std::flush;
                    linenoiseShow(&ls);
                }
            }
        }
        linenoiseEditStop(&ls);
    }
}