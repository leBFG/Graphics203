#pragma once
#include "Skateboard/Log.h"

namespace Skateboard
{
	class PlaystationLogger final : public Logger
	{
	public:
		PlaystationLogger();

	protected:
		void LogMessage_(const LogSeverity& severity, const std::string& Component,
		                 const std::string& Message) override;
		void SetOutputSeverity_(const LogSeverity& severity) override;
	};
}


