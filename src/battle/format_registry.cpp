#include "format_registry.h"

namespace pkm::battle {

const std::map<std::string, BattleFormat>& get_formats() {

    static const std::map<std::string, BattleFormat> formats = {

        {
            "Random Battle",
            {
                .display_name = "Random Battle",
                .protocol_id = "gen9randombattle",
                .category = "S/V Singles",
                .rated = true
            }
        },

        {
            "Unrated Random Battle",
            {
                .display_name = "Unrated Random Battle",
                .protocol_id = "gen9unratedrandombattle",
                .category = "S/V Singles",
                .rated = false
            }
        }

    };

    return formats;
}

}