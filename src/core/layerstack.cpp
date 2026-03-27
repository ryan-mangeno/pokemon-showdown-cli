#include <pkmpch.h>

#include "layerstack.h"

namespace pkm {

	LayerStack::LayerStack()
		: m_layer_iterator_offset(0)
	{
	}

	LayerStack::~LayerStack() {
		for (Layer* layer : m_layers) {
			layer->on_detach();
			delete layer;
		}
	}
	
	void LayerStack::push_layer(Layer* layer) {
		m_layers.emplace(begin() + (m_layer_iterator_offset++), layer);
	}

	void LayerStack::push_overlay(Layer* overlay) {
		m_layers.emplace_back(overlay);
	}

	void LayerStack::pop_layer(Layer* layer) {
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end()) {
			m_layers.erase(it);

			// we wont need to check for bounds, in the case when we remove first element
			// m_layer_iterator_offset will be offset to then next available slot for a layer
			// ... it will be at offset : 1, if there is 1 layer, so when we decrement
			// there will be no more layers and we will be at offset 0, being correct offset
			m_layer_iterator_offset--;
		}
	}

	void LayerStack::pop_overlay(Layer* overlay) {
		auto it = std::find(m_layers.begin(), m_layers.end(), overlay);
		if (it != m_layers.end())
			m_layers.erase(it);
	}

}