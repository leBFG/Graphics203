#pragma once
#include "Event.h"

namespace Skateboard
{
	struct User;

	//windows events
	class SKTBD_API WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t w, uint32_t h)
			: 
				m_Width(w)
			,	m_Height(h) 
		{}

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		EVENT_CLASS_TYPE(EventType::WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		uint32_t m_Width;
		uint32_t m_Height;
	};

	class SKTBD_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() 
		{}

		EVENT_CLASS_TYPE(EventType::WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class SKTBD_API AppTickEvent : public Event
	{
	public:
		AppTickEvent() 
		{}
		EVENT_CLASS_TYPE(EventType::AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class SKTBD_API AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() 
		{}
		EVENT_CLASS_TYPE(EventType::AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class SKTBD_API AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() 
		{}
		EVENT_CLASS_TYPE(EventType::AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	//user manager events

	class SKTBD_API AppLoginEvent : public Event
	{
	public:
		AppLoginEvent(const User& newser) :
			user(newser)
		{
		}
		EVENT_CLASS_TYPE(EventType::UserLogin)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

		const User& GetUser() { return user; }

	private:
		const User& user;
	};

	class SKTBD_API AppLogOutEvent : public Event
	{
	public:
		AppLogOutEvent(const User& loser) :
			user(loser)
		{
		}
		EVENT_CLASS_TYPE(EventType::UserLogOut)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

		const User& GetUser() { return user; }

	private:
		const User& user;
	};
}