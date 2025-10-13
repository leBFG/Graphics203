#pragma once

#include "Skateboard/Utilities/StringConverters.h"
#include "Flossy.hpp"

//#define SKTBD_LOG_COMPONENT "Log"

#ifndef SKTBD_LOG_COMPONENT
#define  SKTBD_LOG_COMPONENT ""
#endif // SKTBD_LOG_COMPONENT

namespace Skateboard
{
	enum LogSeverity : uint8_t
	{
		Trace,
		Debug,
		Info,
		Warn,
		Error,
		Critical
	};

	class Logger
	{
	public:
		virtual ~Logger() = default;
		static void RegisterLogger(Logger* PlatformLogger)
		{
			m_Logger = PlatformLogger;
			LogMessage(Trace, "Starting up Log");
		}

		template<typename CharTString>
		static void LogMessage(const LogSeverity& severity, const std::string& Component, CharTString Message)
		{
			if constexpr (std::is_constructible_v<std::string, CharTString>)
			{
				m_Logger->LogMessage_(severity, Component, Message);
			}
			else if constexpr (std::is_constructible_v<std::wstring, CharTString>)
			{
				m_Logger->LogMessage_(severity, Component, ToString(Message));
			} else 
			{
				//Logger doth not know the type of string you are trying to shove in it and will do nothing
				//static_assert(false);
			}
		}

		template<typename CharTString>
		static void LogMessage(const LogSeverity& severity, CharTString Message)
		{
			if constexpr (std::is_convertible_v<decltype(Message), std::string>)
			{
				m_Logger->LogMessage_(severity, "Generic", Message);
			}
			else if constexpr (std::is_convertible_v<decltype(Message),std::wstring>)
			{
				m_Logger->LogMessage_(severity, "Generic", ToString(std::wstring(Message)));
			}
			else
			{
				//Logger doth not know the type of string you are trying to shove in will do nothing
				//static_assert(false);
			}
		}
		
	protected:
		virtual void LogMessage_(const LogSeverity& severity, const std::string& Component, const std::string& Message) = 0;
		virtual void SetOutputSeverity_(const LogSeverity& severity) = 0;

		static inline Logger* m_Logger = nullptr;
	};

}

#if defined(SKTBD_SHIP)
// log macros with custom component
#define SKTBD_LOG_CRITICAL(...)
#define SKTBD_LOG_ERROR(...)
#define SKTBD_LOG_WARN(...)
#define SKTBD_LOG_INFO(...)
#define SKTBD_LOG_TRACE(...)

// log macors without auto component
#define SKTBD_MSG_CRITICAL(...)
#define SKTBD_MSG_ERROR(...)
#define SKTBD_MSG_WARN(...)
#define SKTBD_MSG_INFO(...)
#define SKTBD_MSG_TRACE(...)

#else

// Core log macros
#define SKTBD_LOG(severity, component, ...)		{::Skateboard::Logger::LogMessage(severity, component ,flossy::format(__VA_ARGS__));}

// Core log macros
#define SKTBD_LOG_CRITICAL(component,...)		SKTBD_LOG(::Skateboard::LogSeverity::Critical,	 component ,flossy::format(__VA_ARGS__))
#define SKTBD_LOG_ERROR(component,...)			SKTBD_LOG(::Skateboard::LogSeverity::Error,	 component ,flossy::format(__VA_ARGS__))
#define SKTBD_LOG_WARN(component,...)			SKTBD_LOG(::Skateboard::LogSeverity::Warn,		 component ,flossy::format(__VA_ARGS__))
#define SKTBD_LOG_INFO(component,...)			SKTBD_LOG(::Skateboard::LogSeverity::Info,		 component ,flossy::format(__VA_ARGS__))
#define SKTBD_LOG_DEBUG(component,...)			SKTBD_LOG(::Skateboard::LogSeverity::Debug,	 component ,flossy::format(__VA_ARGS__))
#define SKTBD_LOG_TRACE(component,...)			SKTBD_LOG(::Skateboard::LogSeverity::Trace,	 component ,flossy::format(__VA_ARGS__))

// Core log macros
#define SKTBD_MSG_CRITICAL(...)		{::Skateboard::Logger::LogMessage(::Skateboard::LogSeverity::Critical, SKTBD_LOG_COMPONENT ,flossy::format(__VA_ARGS__));}
#define SKTBD_MSG_ERROR(...)			{::Skateboard::Logger::LogMessage(::Skateboard::LogSeverity::Error,	   SKTBD_LOG_COMPONENT ,flossy::format(__VA_ARGS__));}
#define SKTBD_MSG_WARN(...)			{::Skateboard::Logger::LogMessage(::Skateboard::LogSeverity::Warn,	   SKTBD_LOG_COMPONENT ,flossy::format(__VA_ARGS__));}
#define SKTBD_MSG_INFO(...)			{::Skateboard::Logger::LogMessage(::Skateboard::LogSeverity::Info,	   SKTBD_LOG_COMPONENT ,flossy::format(__VA_ARGS__));}
#define SKTBD_MSG_DEBUG(...)			{::Skateboard::Logger::LogMessage(::Skateboard::LogSeverity::Debug,	   SKTBD_LOG_COMPONENT ,flossy::format(__VA_ARGS__));}
#define SKTBD_MSG_TRACE(...)			{::Skateboard::Logger::LogMessage(::Skateboard::LogSeverity::Trace,	   SKTBD_LOG_COMPONENT ,flossy::format(__VA_ARGS__));}

#endif

