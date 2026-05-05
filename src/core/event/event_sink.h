#pragma once

#include "event.h"

namespace pkm {

    class EventSink {
    public:
        virtual ~EventSink() = default;
        virtual void submit(Scope<Event> e) = 0;
    };

}