#include <pkmpch.h>

#include "layerstack.h"

namespace pkm {

	LayerStack::LayerStack()
		: m_layer_iterator_offset(0)
	{
	}

	LayerStack::~LayerStack() {
		for (int i = m_layers.size() - 1; i >= 0; i--) {
			Layer* layer = m_layers[i];
			if (layer) {
				layer->on_detach();
				delete layer;
				m_layers[i] = nullptr; 
			}
		}
		m_layers.clear();
	}
	
	void LayerStack::push_layer(Layer* layer) {
		m_layers.emplace(begin() + (m_layer_iterator_offset++), layer);
		layer->on_attach();
	}

	void LayerStack::push_overlay(Layer* overlay) {
		m_layers.emplace_back(overlay);
		overlay->on_attach();
	}

	void LayerStack::pop_layer(Layer* layer) {
		auto it = std::find(m_layers.begin(), m_layers.begin() + m_layer_iterator_offset, layer);
		if (it != m_layers.end()) {
			(*it)->on_detach();
			delete *it;          
			m_layers.erase(it);   
			m_layer_iterator_offset--;
		}
	}

	void LayerStack::pop_overlay(Layer* overlay) {
		auto it = std::find(m_layers.begin() + m_layer_iterator_offset, m_layers.end(), overlay);
		if (it != m_layers.end()) {
			(*it)->on_detach();
			delete *it;          
			m_layers.erase(it);   
			m_layer_iterator_offset--;
		}
	}

}