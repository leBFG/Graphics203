#pragma once
#include "Skateboard/Log.h"

namespace spdlog
{
	class logger;
}

namespace Skateboard
{
	class WindowsLog : public Logger
	{
	public:
		WindowsLog();

		void LogMessage_(const LogSeverity& severity, const std::string& Component, const std::string& Message) override;
		void SetOutputSeverity_(const LogSeverity& severity) override;

		//inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		//inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return s_AppLogger; }


	/*private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_AppLogger;*/
	};
}
