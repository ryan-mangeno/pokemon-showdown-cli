#pragma once

#include <string>

namespace pkm::battle {

    struct BattleFormat {
        std::string display_name;
        std::string protocol_id;
        std::string category;
        bool rated;
    };

}