#pragma once 

#include <filesystem>
#include <string_view>
#include <vector>
#include <string>

namespace pkm {
    // project root is made during cmake build 
    constexpr std::string_view ROOT_DIR = PROJECT_ROOT;
    
    std::vector<std::string> tokenize(const std::string& input);
}
