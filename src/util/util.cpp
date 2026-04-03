#include <pkmpch.h>

#include "util.h"

#include <sstream>

namespace pkm {

    std::vector<std::string> tokenize(const std::string& input) {
        std::istringstream iss(input);
        std::vector<std::string> tokens;
        std::string tok;

        while (iss >> tok) {
            tokens.push_back(tok);
        }

        return tokens;
    }
}