#pragma once 

#include "event.h"

namespace pkm {

    // TODO: maybe a battle event abstract class since there will be dif battle event types
    // maybe we dont even need seperate classes for some of these

    class BattleSearchEvent : public Event {
    public:
        BattleSearchEvent() = default;

        EVENT_CLASS_CATEGORY(EventCategoryApplication);
        EVENT_CLASS_TYPE(BattleSearch);

    private:
    };

    class BattleJoinEvent : public Event {
    public:
        BattleJoinEvent() = default;

        EVENT_CLASS_CATEGORY(EventCategoryApplication);
        EVENT_CLASS_TYPE(BattleJoin);

    private:
    };

}

