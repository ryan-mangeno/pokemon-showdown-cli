#pragma once

#include "event/event.h"

namespace pkm {

	class Layer {
	public:
		
		Layer(const char* name = "Layer");
		virtual ~Layer();

		virtual void on_attach()  {};
		virtual void on_detach()  {};
		virtual void on_update() {};
		virtual void on_event(Event& e) {}
		
		inline const char* get_name() const { return m_debugname; }

	protected:
		const char* m_debugname;
	};
}