#pragma once 

#include <string>
#include "core/logger.h"

namespace pkm::protocol {
   
    /*   two cases ... 
        "p2: Hoopa"  -> side: "p2", slot: "",  name: "Hoopa"
        "p2a: Hoopa" -> side: "p2", slot: "a", name: "Hoopa"
    */
    struct Ident {
        char slot;
        std::string side;
        std::string name;

        Ident(char slot, const std::string& side, const std::string& name) :
            slot(slot), side(side), name(name) {}

        Ident() = default;
        ~Ident() = default;
    };


}

