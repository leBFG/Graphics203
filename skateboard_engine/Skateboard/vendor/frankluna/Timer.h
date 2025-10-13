#pragma once
// Include the Win32 API to query the performance timer
#include <Windows.h>

class Timer
{
public:
	Timer();
	~Timer();

	const float& DeltaTime() const { return m_DeltaTime; }
	const float& TotalTime();
	const float& FPS() const { return m_FPS; }

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	bool m_Stopped;				// A bool to stop the timer
	INT64 m_PreviousTime;		// The time on the previous measure of time on this timer
	float m_TicksPerSecond;		// The number of ticks per second of the performance timer
	float m_DeltaTime;			// The delta time between two frames in seconds
	float m_TotalTime;			// The total time that passed since the start of the application
	INT64 m_BaseTime;			// The time at the start of the application (0) or when it has been Reset()
	INT64 m_CurrentTime;		// The current tick count
	INT64 m_StopTime;			// The time at which we stopped the timer
	INT64 m_PausedTime;			// The time passed while the time was paused
	float m_FPS;				// The frames per second of this applications
};

