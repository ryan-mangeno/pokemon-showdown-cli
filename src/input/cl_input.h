#pragma once

#include "input.h"
#include "core/event/event.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>


namespace pkm {

    class CLInput : public Input {

    public:
        CLInput();
        
        void start() override;
        void stop() override;

        void set_input_ui(const std::string& ui);

        inline void set_callback(const EventCallbackFn& callback) override {
            m_callback = callback;
        };

    private:
        void run();
    
    private:

        EventCallbackFn m_callback;

        std::thread m_thread;
        std::mutex m_ui_mutex;
        std::string m_ui_buffer{"> "};
        std::atomic<bool> m_ui_dirty{true};
        std::atomic<bool> m_running{false};
    };
}