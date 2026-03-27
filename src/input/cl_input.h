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

        inline void set_callback(const EventCallbackFn& callback) override {
            m_callback = callback;
        };

    private:
        void run();
    
    private:

        EventCallbackFn m_callback;

        std::thread m_thread;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::atomic<bool> m_running{false};
        std::string m_prompt;
    };
}