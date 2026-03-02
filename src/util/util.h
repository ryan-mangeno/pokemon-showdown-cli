#pragma once 

#include <filesystem>
#include <string_view>

namespace pkm {
    
    const std::string ROOT_DIR = PROJECT_ROOT;
    
    inline std::filesystem::path config_path() {
        return std::filesystem::path(ROOT_DIR) / "config" / "config.json";
    }
}
