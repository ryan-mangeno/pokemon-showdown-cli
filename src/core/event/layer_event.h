#pragma once 

#include "event.h"

namespace pkm {

    class LayerEvent : public Event {
    public:
        LayerEvent(Layer* layer) : m_layer_ptr(layer) {}

        Layer* get_layer_ptr() { return m_layer_ptr; }

        EVENT_CLASS_CATEGORY(EventCategoryApplication);
        EVENT_CLASS_TYPE(Layer);

    private:
        Layer* m_layer_ptr;
    };

}

