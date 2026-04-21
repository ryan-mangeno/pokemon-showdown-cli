#include <pkmpch.h>

#include "cl_input.h"
#include "core/logger.h"
#include "core/event/command_event.h"

#include <iostream>

namespace pkm {

    CLInput::CLInput()
        : m_running(false) {}

    void CLInput::start() {
        m_running = true;

        linenoiseSetMultiLine(1);
        linenoiseHistorySetMaxLen(100);
        
        // start with empty buffer
        m_lsbuffer[0] = '\0';
        m_ui_dirty = false;

        linenoiseEditStart(&m_ls, -1, -1, m_lsbuffer, sizeof(m_lsbuffer), "> ");
    }

    void CLInput::stop() {
        linenoiseEditStop(&m_ls);
        m_running = false;
    }

    void CLInput::set_input_ui(const std::string& ui) {
        m_ui_buffer = ui;
        m_ui_dirty = true;
    }

    void CLInput::poll() {
        fd_set readfds;
        struct timeval tv = {0, 100000};
        FD_ZERO(&readfds);
        FD_SET(m_ls.ifd, &readfds);

        int retval = select(m_ls.ifd + 1, &readfds, NULL, NULL, &tv);

        if (retval > 0) {
            char* line = linenoiseEditFeed(&m_ls);
            if (line != linenoiseEditMore) {
                if (line) {
                    linenoiseHistoryAdd(line);
                    if (m_callback) {
                        CommandEvent e{std::string(line)};
                        m_callback(e);
                    }
                    free(line);
                }
                linenoiseEditStop(&m_ls);
                linenoiseEditStart(&m_ls, -1, -1, m_lsbuffer, sizeof(m_lsbuffer), "> ");
            }
        } else {
            // timeout, redraw ui if dirty
            if (m_ui_dirty) {
                m_ui_dirty = false;
                linenoiseHide(&m_ls);
                std::cout << "\r\n" << m_ui_buffer << std::flush;
                linenoiseShow(&m_ls);
            }
        }
    }
}
