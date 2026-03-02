#pragma once


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

#include "defines.h" 

namespace pkm {

    class Logger {
        public:
            static Ref<spdlog::logger>& get_logger() {
                return s_logger;
            }

            static void init();

        private:
            Logger();
            ~Logger();

        private:
            static Ref<spdlog::logger> s_logger;
    };

}

#if defined (PK_DEBUG) 
    #define PK_FATAL(...) pkm::Logger::get_logger()->critical(__VA_ARGS__) 
    #define PK_ERROR(...) pkm::Logger::get_logger()->error(__VA_ARGS__)
    #define PK_WARN(...)  pkm::Logger::get_logger()->warn(__VA_ARGS__)
    #define PK_INFO(...) pkm::Logger::get_logger()->info(__VA_ARGS__)
    #define PK_TRACE(...) pkm::Logger::get_logger()->trace(__VA_ARGS__)

#else 
    #define PK_FATAL(...) 
    #define PK_ERROR(...) 
    #define PK_WARN(...) 
    #define PK_INFO(...) 
    #define PK_TRACE(...)
#endif
