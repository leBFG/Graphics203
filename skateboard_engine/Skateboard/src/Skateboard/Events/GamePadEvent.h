#pragma once
#include "Event.h"


namespace Skateboard
{

	class SKTBD_API GamePadEvent : public Event
	{
	public:
		GamePadEvent(int32_t button, int8_t gamePad) 
			: 
				m_Button(button) 
		{}

		EVENT_CLASS_CATEGORY(EventCategoryGamePad | EventCategoryInput)

	protected:
		int32_t m_Button;
		int8_t m_GamePadNumber;
	};

	class SKTBD_API GamePadButtonPressedEvent : public GamePadEvent
	{
	public:
		GamePadButtonPressedEvent(int32_t button, int8_t gamePad)
			:
				GamePadEvent(button, gamePad)
		{}

		EVENT_CLASS_TYPE(EventType::GamePadButtonPressed)
	};

	class SKTBD_API GamePadButtonReleasedEvent : public GamePadEvent
	{
	public:
		GamePadButtonReleasedEvent(int32_t button, int8_t gamePad)
			:
			GamePadEvent(button, gamePad)
		{}

		EVENT_CLASS_TYPE(EventType::GamePadButtonReleased)
	};

	class SKTBD_API GamePadButtonDownEvent : public GamePadEvent
	{
	public:
		GamePadButtonDownEvent(int32_t button, int8_t gamePad)
			:
			GamePadEvent(button, gamePad)
		{}

		EVENT_CLASS_TYPE(EventType::GamePadButtonDown)
	};

	class SKTBD_API GamePadJoyStickEvent : public GamePadEvent
	{
	public:
		GamePadJoyStickEvent(uint8_t x, uint8_t y, int8_t gamePad)
			:
				GamePadEvent(0, gamePad)
			,	m_iX(x)
			,	m_X((static_cast<float>(x / 255u) - 0.5f) * 2.0f)
			,	m_iY(y)
			,	m_Y((static_cast<float>(y / 255u) - 0.5f) * 2.0f)
		{}

		EVENT_CLASS_CATEGORY(EventCategoryGamePadJoyStick | EventCategoryInput)
		EVENT_CLASS_TYPE(EventType::GamePadJoyStickMoved)

	protected:
		float m_X;
		uint8_t m_iX;
		float m_Y;
		uint8_t m_iY;
	};

	class SKTBD_API GamePadTriggerEvent : public GamePadEvent
	{
	public:
		GamePadTriggerEvent(uint8_t value, int8_t gamePad)
			:
				GamePadEvent(0, gamePad)
			,	m_iValue(value)
			,	m_Value(static_cast<float>(value / 255u))
		{}

		EVENT_CLASS_CATEGORY(EventCategoryGamePadTrigger | EventCategoryInput)
	protected:
		uint8_t m_iValue;
		float m_Value;
	};

	class SKTBD_API GamePadTriggerPressedEvent : public GamePadTriggerEvent
	{
	public:
		GamePadTriggerPressedEvent(uint8_t value, int8_t gamePad)
			:
			GamePadTriggerEvent(value, gamePad)
		{}

		EVENT_CLASS_TYPE(EventType::GamePadTriggerPressed)
	};

	class SKTBD_API GamePadTriggerReleasedEvent : public GamePadTriggerEvent
	{
	public:
		GamePadTriggerReleasedEvent(uint8_t value, int8_t gamePad)
			:
			GamePadTriggerEvent(value, gamePad)
		{}

		EVENT_CLASS_TYPE(EventType::GamePadTriggerReleased)
	};

	// TODO: The contents of this event were generated via copilot, requires proper implementation.
	class SKTBD_API GamePadOrientationEvent : public GamePadEvent
	{
	public:
		GamePadOrientationEvent(uint8_t x, uint8_t y, uint8_t z, int8_t gamePad)
			:
				GamePadEvent(0, gamePad)
			,	m_iX(x)
			,	m_X((static_cast<float>(x / 255u) - 0.5f) * 2.0f)
			,	m_iY(y)
			,	m_Y((static_cast<float>(y / 255u) - 0.5f) * 2.0f)
			,	m_iZ(z)
			,	m_Z((static_cast<float>(z / 255u) - 0.5f) * 2.0f)
		{}

		EVENT_CLASS_CATEGORY(EventCategoryGamePadOrientation | EventCategoryInput)

	protected:
		uint8_t m_iX;
		uint8_t m_iY;
		uint8_t m_iZ;

		float m_X;
		float m_Y;
		float m_Z;
	};

}