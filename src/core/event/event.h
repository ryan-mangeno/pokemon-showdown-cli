#pragma once

#include "core/defines.h"

#include <string>

namespace pkm {

	// when an event occurs it gets sent out and proccessed immediatly
	// TODO: - batch events and proccess during event section of update stage
    // multi threaded event system, also add event types for battles, network events, etc

	enum class EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	// bit feild to apply multiple events
	// some events are both keyboard and input
	enum EventCategory {
		None = 0,
		EventCategoryApplication     = BIT(0),
		EventCategoryInput           = BIT(1),
		EventCategoryKeyboard        = BIT(2),
		EventCategoryMouse           = BIT(3),
		EventCategoryMouseButton     = BIT(4),
        EventCategoryNetwork  		 = BIT(5),
        EventCategoryBattle  		 = BIT(6),
        EventCategoryCLI      		 = BIT(7),
	};

// TODO: should be mindful about platform stuff in future 
//#ifdef PK_PLATFORM_MACOS

#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
							    virtual EventType get_event_type() const override { return get_static_type(); }\
								virtual const char* get_name() const override { return #type; }

//#endif 

#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return category; }

	class Event {
		friend class EventDispatcher;

	public:
		virtual EventType get_event_type() const = 0;
		virtual const char* get_name() const = 0;
		virtual int get_category_flags() const = 0;
		virtual std::string to_str() const { return get_name(); }

		inline bool get_handled() const { return m_handled; }
		inline bool is_in_category(EventCategory category) const
		{
			return get_category_flags() & category;
		}
	protected:
		bool m_handled = false;

	};


	class EventDispatcher {
		template<typename T>
		using EventFn = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event)
			: m_event(event) {}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_event.get_event_type() == T::get_static_type())
			{					
				//(T*)&m_Event) -> casts a T pointer to address of m_Event
				//(*(T*)&m_Event) -> defrefrences the pointer so we can use it as a ref
				m_event.m_handled = func(*(T*)&m_event);
				return true;
			}
			return false;
		}
		

	private:
		Event& m_event;
	};


	inline std::ostream& operator <<(std::ostream & os, const Event & e) {
		return os << e.to_str();
	}

}