#include <pkmpch.h>

#include "logger.h"

namespace pkm {

    Ref<spdlog::logger> Logger::s_logger = nullptr;
    
    void Logger::init() {
        spdlog::set_pattern("%^[%T] %n: %v%$");

		    s_logger = spdlog::stdout_color_mt("PKM_LOGGER");
		    s_logger->set_level(spdlog::level::trace);

		    PK_INFO("Initialized Log!");
    }

}
