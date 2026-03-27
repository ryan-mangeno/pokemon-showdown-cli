#pragma once

#include <vector>
#include "layer.h"

namespace pkm {

	// layer stack is owned by the application
	class LayerStack {
	public:
		LayerStack();
		~LayerStack();

		void push_layer(Layer* layer);
		void push_overlay(Layer* overlay);
		void pop_layer(Layer* layer);
		void pop_overlay(Layer* overlay);

		// layers will be put into first half of list
		// overlays in second half of the list
		
		// alows us to iterate over layers in range based for loop in our application
		std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_layers.end(); }

	private:
		
		// every frame these need to be iterated over for things like updates
		// this is not an actual stack, because we will end up pushing layers in the middle
		// this is to avoid overhead of having two seperate lists for layers and overlays
		// also because overlays and layers are very similar

		std::vector<Layer*> m_layers;

		// we keep a iterator offset since iterators can be invalidated when pushing
		// and erasing elements from container  and also reallocations
		uint32_t m_layer_iterator_offset;
	};

}