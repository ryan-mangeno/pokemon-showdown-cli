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

    std::vector<Message> parse_message(const std::string& raw) {
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

    Ident parse_ident(const std::string& ident) {
        if (ident.empty()) {
            PK_ERROR("Empty ident passed!");
            return Ident();
        }

        const char delim = ':';
        size_t pos = ident.find(delim);

        if (pos != std::string::npos) {
            std::string meta_data = ident.substr(0, pos);

            auto split_pos = std::find_if(ident.begin() + pos + 1, ident.end(),
                             [](char c) { return c != ' '; });

            if (split_pos == ident.end()) {
               PK_ERROR("No pokemon name in ident {}!", ident);
               return Ident();
            }

            size_t start_idx = std::distance(ident.begin(), split_pos); 
            std::string pkm_name = ident.substr(start_idx);
            std::string side = ident.substr(0,2);
            
            if (meta_data.size() == 2) { // case 1
                return {'\0', side, pkm_name};
            } else if (meta_data.size() == 3) { // case 2
                char slot = ident[2]; 
                return {slot, side, pkm_name};
            } else {
                PK_ERROR("Unexpected ident size: {}", ident);
                return Ident();
            }
            
        } else {
            PK_ERROR("Failed to parse ident {}!", ident);
            return Ident();
        }
    }


}
