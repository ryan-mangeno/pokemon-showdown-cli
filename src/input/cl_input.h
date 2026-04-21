#pragma once

#include "input.h"
#include "core/event/event.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

#include <linenoise.h>

namespace pkm {
    
static constexpr int BUFFER_SIZE = 1024;

    class CLInput : public Input {

    public:
        CLInput();
        
        void start() override;
        void stop() override;
        void poll() override;
        void set_input_ui(const std::string& ui);

        inline void set_callback(const EventCallbackFn& callback) override {
            m_callback = callback;
        };

    private:
        void run();
    
    private:

        EventCallbackFn m_callback;

        std::string m_ui_buffer{"> "};
        char m_lsbuffer[BUFFER_SIZE];
        struct linenoiseState m_ls;
        bool m_ui_dirty{true};
        bool m_running{false};
        
    };
}
