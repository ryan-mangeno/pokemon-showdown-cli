#include <pkmpch.h>

#include "logger.h"
#include "spdlog/sinks/basic_file_sink.h"


namespace pkm {

    Ref<spdlog::logger> Logger::s_logger = nullptr;
    
    void Logger::init() {
        spdlog::set_pattern("%^[%T] %n: %v%$");

        s_logger = spdlog::basic_logger_mt("PKM_LOGGER", "pkm_shdwn.log");
        
        s_logger->flush_on(spdlog::level::trace);
        s_logger->set_level(spdlog::level::trace);

        PK_INFO("Initialized Log!");
    }

}
