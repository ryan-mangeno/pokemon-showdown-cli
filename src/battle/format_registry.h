#pragma once

#include "format.h"
#include <map>

namespace pkm::battle {

    const std::map<std::string, BattleFormat>& get_formats();

}