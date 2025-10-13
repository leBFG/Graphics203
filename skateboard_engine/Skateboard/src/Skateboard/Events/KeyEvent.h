#pragma once
#include "Event.h"

namespace Skateboard
{

	class SKTBD_API KeyEvent : public Event
	{
	public:
		int32_t GetKeyCode() { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(int32_t keycode)
			: 
				m_KeyCode(keycode)
		{}

		int32_t m_KeyCode;
	};

	class SKTBD_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int32_t keycode, int32_t repeatCount)
			: 
				KeyEvent(keycode)
			,	m_RepeatCount(repeatCount)
		{}

		int32_t GetRepeatCount() { return m_RepeatCount; }

		EVENT_CLASS_TYPE(EventType::KeyPressed);
	private:
		int32_t m_RepeatCount;
	};

	class SKTBD_API KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int32_t keycode)
			: 
				KeyEvent(keycode)
		{}

		EVENT_CLASS_TYPE(EventType::KeyTyped);
	};

	class SKTBD_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int32_t keycode)
			: 
				KeyEvent(keycode)
		{}

		EVENT_CLASS_TYPE(EventType::KeyReleased);

	};

}