#include <pkmpch.h>

#include "parser.h"

namespace pkm::protocol {
    
    static std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> tokens;
        std::stringstream ss(s);
        std::string token;
        while (std::getline(ss, token, delim)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::vector<Message> parse(const std::string& raw) {
        if (raw.empty()) return {};
        
        std::vector<std::string> lines = split(raw, '\n');
        
        std::string current_room;
        std::vector<Message> msgs;

        for (const std::string& line : lines) {
            if (line.empty()) continue;
            
            // room context line
            if (line[0] == '>') {
                current_room = line.substr(1); 
                continue;
            }
            
            if (line[0] == '|') {
                std::vector<std::string> tokens = split(line.substr(1), '|');
                if (tokens.empty()) continue;

                Message msg;
                msg.room_id = current_room;
                msg.type    = tokens[0];
                
                msg.args    = std::vector<std::string>(
                            tokens.begin() + 1, tokens.end());

                msgs.push_back(std::move(msg));
            }
        }

        return msgs;
    }

}
