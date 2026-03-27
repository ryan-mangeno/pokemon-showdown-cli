#pragma once 

#include "event.h"

namespace pkm {

    class CommandEvent : public Event {
    public:
        CommandEvent(std::string cmd) : m_command(std::move(cmd)) {}

        const std::string& get_command() const { return m_command; }

        EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryApplication | EventCategoryKeyboard)
        EVENT_CLASS_TYPE(AppUpdate);

    private:
        std::string m_command;
    };

}