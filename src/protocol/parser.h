#pragma once

#include <string>
#include <vector>

#include "message.h"
#include "ident.h"

namespace pkm::protocol {
   
    std::vector<Message> parse_message(const std::string& raw);
 
    Ident parse_ident(const std::string& ident);


}
