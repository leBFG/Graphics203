#include "WindowsLog.h"

// This ignores all warnings raised inside External headers
#pragma warning(push)
#pragma warning(disable : 4616)
#pragma warning(disable : 6285)
#pragma warning(disable : 26437)
#pragma warning(disable : 26450)
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)
#pragma warning(disable : 26498)
#pragma warning(disable : 26800)
#pragma warning(disable : 4996)
//#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning(pop)

namespace Skateboard
{
	WindowsLog::WindowsLog()
	{
		spdlog::set_pattern("%^[%T]:%v%$");
		auto s_CoreLogger = spdlog::stdout_color_mt("Engine");
		///spdlog::name

		spdlog::sinks::stderr_color_sink_mt* color = static_cast<spdlog::sinks::stderr_color_sink_mt*>(s_CoreLogger->sinks().back().get());
		color->set_color(spdlog::level::level_enum::info, 0x0003);	// first 8 bits not relevant for us - background RGBA - RGBA -> 1 blue and 1 alpha
		s_CoreLogger->set_level(spdlog::level::trace);

		//s_AppLogger = spdlog::stdout_color_mt("App");
		//s_AppLogger->set_level(spdlog::level::trace);
	}

	void WindowsLog::LogMessage_(const LogSeverity& severity, const std::string& Component,
		const std::string& Message)
	{
		spdlog::get("Engine")->log(static_cast<spdlog::level::level_enum>(severity), "["+Component+"]" + " " + Message);
	}

	void WindowsLog::SetOutputSeverity_(const LogSeverity& severity)
	{
		spdlog::get("Engine")->set_level(static_cast<spdlog::level::level_enum>(severity));
	}
}
