#pragma once 

#include "event.h"
#include "core/layer.h"

namespace pkm {
    class LayerPushEvent : public Event {
    public:
        LayerPushEvent(Layer* layer, bool is_overlay) : m_layer_ptr(layer), m_is_overlay(is_overlay) {}

        Layer* get_layer_ptr() { return m_layer_ptr; }
        bool is_overlay() const { return m_is_overlay; }

        EVENT_CLASS_CATEGORY(EventCategoryApplication);
        EVENT_CLASS_TYPE(LayerPush);

    private:
        Layer* m_layer_ptr;
        bool m_is_overlay;
    };

    class LayerPopEvent : public Event {
    public:
        LayerPopEvent(Layer* layer, bool is_overlay) : m_layer_ptr(layer), m_is_overlay(is_overlay) {}

        Layer* get_layer_ptr() { return m_layer_ptr; }
        bool is_overlay() const { return m_is_overlay; }

        EVENT_CLASS_CATEGORY(EventCategoryApplication);
        EVENT_CLASS_TYPE(LayerPop);

    private:
        Layer* m_layer_ptr;
        bool m_is_overlay;
    };

}

