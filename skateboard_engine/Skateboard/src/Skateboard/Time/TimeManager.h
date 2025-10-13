#pragma once
#include "Skateboard/Core.h"
#include <memory>

namespace Skateboard
{
	class TimeManager
	{
	public:
		TimeManager();
		DISABLE_COPY_AND_MOVE(TimeManager);
		virtual ~TimeManager();

		virtual float& ElapsedTime() = 0;

		virtual void Reset() = 0;
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void Update() = 0;

		const float& DeltaTime()	{ return m_DeltaTime; }
		const float& FPS()			{ return m_FPS; }

	protected:
		bool m_Stopped;				// A bool to stop the timer
		uint64_t m_PreviousTime;		// The time on the previous measure of time on this timer
		float m_TicksPerSecond;		// The number of ticks per second of the performance timer
		float m_DeltaTime;			// The delta time between two frames in seconds
		float m_TotalTime;			// The total time that passed since the start of the application
		uint64_t m_BaseTime;			// The time at the start of the application (0) or when it has been Reset()
		uint64_t m_CurrentTime;		// The current tick count
		uint64_t m_StopTime;			// The time at which we stopped the timer
		uint64_t m_PausedTime;		// The time passed while the time was paused
		float m_FPS;				// The fra

	};


}