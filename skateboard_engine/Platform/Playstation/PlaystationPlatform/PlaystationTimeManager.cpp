#include "PlaystationTimeManager.h"
#include "sktbdpch.h"


namespace Skateboard
{
	AGCTimeManager::AGCTimeManager() : TimeManager()
	{
		// totalTime += (endTime - startTime) / ptcFreq;				[result in ms]
		m_TicksPerSecond = sceKernelGetProcessTimeCounterFrequency();	// / 1000.0f;

		// endTime = startTime = sceKernelGetProcessTimeCounter
		m_BaseTime = sceKernelGetProcessTimeCounter() / m_TicksPerSecond;
		m_PreviousTime = m_BaseTime;
	}

	AGCTimeManager::~AGCTimeManager() 
	{
	}

	float& AGCTimeManager::ElapsedTime()
	{
		if (m_Stopped) m_TotalTime = (float)(m_StopTime - m_PausedTime - m_BaseTime) / m_TicksPerSecond;

		else m_TotalTime = (float)(m_CurrentTime - m_PausedTime - m_BaseTime) / m_TicksPerSecond;
		m_TotalTime *= 1000.0f;
		return m_TotalTime;
	}

	void AGCTimeManager::Reset()
	{
		// Query the current time and resets from there
		m_BaseTime = sceKernelGetProcessTimeCounter() / m_TicksPerSecond;

		// Just like in the constructor, both m_BaseTime and m_PrevTime are initialised to the same value
		m_BaseTime = m_CurrentTime;
		m_PreviousTime = m_CurrentTime;

		// Resume the timer
		m_Stopped = false;
		// m_StopTime is reset cause we are currently not stopped
		m_StopTime = 0;
	}

	void AGCTimeManager::Start()
	{
		// If we are resuming the timer..
		if (m_Stopped)
		{
			// Query the current time and starts from there
			m_CurrentTime = sceKernelGetProcessTimeCounter() / m_TicksPerSecond;

			// Accumulate the pause time
			// m_PausedTime describes the time that has passed while the timer was stopped
			// Thus, subtract m_StopTime from m_CurrentTime
			m_PausedTime += m_CurrentTime - m_StopTime;

			// Since we are starting the timer back up, m_PreviousTime is no longer valid
			m_PreviousTime = m_CurrentTime;

			// We are no longer stopped
			m_StopTime = 0;
			m_Stopped = false;
		}
	}

	void AGCTimeManager::Stop()
	{
		// Dont try to stop twice!
		if (!m_Stopped)
		{
			// Query the current time as m_StopTime
			// m_StopTime is therefore the time at which the timer stopped
			m_StopTime = m_CurrentTime = sceKernelGetProcessTimeCounter() / m_TicksPerSecond;

			// Stop the timer
			m_Stopped = true;
		}
	}

	void AGCTimeManager::Update()
	{
		m_TotalTime = sceKernelGetProcessTimeCounter() / m_TicksPerSecond;

		m_CurrentTime = sceKernelGetProcessTimeCounter();

		m_DeltaTime = (m_CurrentTime - m_PreviousTime) / m_TicksPerSecond;
		m_FPS = 1.f / m_DeltaTime;

		m_PreviousTime = m_CurrentTime;


	}



}