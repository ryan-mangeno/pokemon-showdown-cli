#pragma once

#include "event.h"
#include "protocol/message.h"

namespace pkm {

    class MessageEvent : public Event {
        public:
            MessageEvent(const protocol::Message& msg) : m_msg(msg) {}

            const protocol::Message& get_msg() const { return m_msg; }
            
            EVENT_CLASS_CATEGORY(EventCategoryApplication | EventCategoryBattle)
            EVENT_CLASS_TYPE(AppUpdate);
            
        private:
            protocol::Message m_msg;
    };

}