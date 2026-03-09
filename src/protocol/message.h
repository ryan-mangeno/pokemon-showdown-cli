#pragma once

#include <string>
#include <vector>

#include "core/logger.h"

namespace pkm::protocol {
    
    struct Message {
        std::string room_id;
        std::string type;
        std::vector<std::string> args; 
        
        const std::string& arg(size_t i) noexcept {
            static const std::string empty{""};
            if (i < 0 || i >= args.size()) {
                return empty;
            } else {
                return args[i];
            }
        }

        void print() const noexcept {
            PK_INFO("Room ID: {}", room_id);
            PK_INFO("Type: {}", type);
            for (const std::string& arg : args) {
                PK_INFO("{}", arg);
            }
        }
    };

}
