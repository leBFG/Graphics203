#include "sktbdpch.h"
#include "TimeManager.h"

#define SKTBD_LOG_COMPONENT "TIME MANAGER"
#include "Skateboard/Log.h"

namespace Skateboard
{
	TimeManager::TimeManager()
		:
			m_Stopped(false)
		,	m_DeltaTime(0.f)
		,	m_TotalTime(0.f)
		,	m_CurrentTime(0)
		,	m_StopTime(0)
		,	m_PausedTime(0)
		,	m_FPS(0.f)
	{
	}

	TimeManager::~TimeManager()
	{
	}
}

