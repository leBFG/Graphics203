#pragma once
#include "Event.h"

namespace Skateboard
{

	class SKTBD_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			:
				m_MouseX(x)
			,	m_MouseY(y)
		{}

		float GetMouseX() const { return m_MouseX; }
		float GetMouseY() const { return m_MouseY; }

		EVENT_CLASS_TYPE(EventType::MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX;
		float m_MouseY;
	};

	class SKTBD_API MouseScrolledEvent : public Event
	{
	public:

		MouseScrolledEvent(float xOffset, float yOffset)
			:
				m_XOffset(xOffset)
			,	m_YOffset(yOffset)
		{}
		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		EVENT_CLASS_TYPE(EventType::MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_XOffset;
		float m_YOffset;
	};

	class SKTBD_API MouseButtonEvent : public Event
	{
	public:
		int32_t GetMouseButton() const { return m_Button; }
		
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:
		MouseButtonEvent() = default;
		MouseButtonEvent(int32_t button) : m_Button(button) {};

		int32_t m_Button;
	};

	class SKTBD_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int32_t button) : MouseButtonEvent(button) {};
		EVENT_CLASS_TYPE(EventType::MouseButtonPressed)
	};

	class SKTBD_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int32_t button) : MouseButtonEvent(button) {};
		EVENT_CLASS_TYPE(EventType::MouseButtonReleased)
	};

}