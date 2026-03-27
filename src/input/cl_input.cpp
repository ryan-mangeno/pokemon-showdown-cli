#include <pkmpch.h>

#include "cl_input.h"
#include "core/logger.h"
#include "core/event/command_event.h"

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

    void CLInput::run() {
        linenoiseSetMultiLine(1);
        linenoiseHistorySetMaxLen(100);

        char buf[1024];
        struct linenoiseState ls;
        linenoiseEditStart(&ls, -1, -1, buf, sizeof(buf), "> ");

        while (m_running) {
            fd_set readfds;
            struct timeval tv = {0, 100000};
            FD_ZERO(&readfds);
            FD_SET(ls.ifd, &readfds);

            if (select(ls.ifd + 1, &readfds, NULL, NULL, &tv) > 0) {
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
            }
        }
        linenoiseEditStop(&ls);
    }
}