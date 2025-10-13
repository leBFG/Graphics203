#pragma comment(lib, "SceDbg_nosubmission_stub_weak")
#include "libdbg.h"

#include "PlaystationLog.h"

namespace Skateboard
{
	PlaystationLogger::PlaystationLogger()
	{
		sceDbgSetMinimumLogLevel(SCE_DBG_LOG_LEVEL_TRACE);
	}
	
	void PlaystationLogger::LogMessage_(const LogSeverity& severity, const std::string& Component,
	                                    const std::string& Message)
	{
		sceDbgLoggingHandler("", 0, severity, Component.c_str(), Message.c_str());
	}

	void PlaystationLogger::SetOutputSeverity_(const LogSeverity& severity)
	{
		sceDbgSetMinimumLogLevel(severity);
	}
}
