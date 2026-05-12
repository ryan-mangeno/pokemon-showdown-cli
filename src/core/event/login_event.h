#pragma once 

#include "event.h"

namespace pkm {

    class LoginEvent : public Event {
    public:
        LoginEvent(bool logging_in) : m_is_logging_in(logging_in) {}

        EVENT_CLASS_CATEGORY(EventCategoryApplication);
        EVENT_CLASS_TYPE(Login);

        bool is_signup () const { return m_is_logging_in; }

    private:
        bool m_is_logging_in;
    };

}

