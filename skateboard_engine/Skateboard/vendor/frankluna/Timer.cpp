#include "Timer.h"
#include <cassert>

Timer::Timer() : m_Stopped(false), m_DeltaTime(0.f), m_TotalTime(0.f), m_CurrentTime(0), m_StopTime(0), m_PausedTime(0), m_FPS(0.f)
{
	// Start with measuring the frequency (counts per second) of the performance timer
	INT64 countPerSecond;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countPerSecond);
	assert(countPerSecond != 0 && "The frequency of the performance timer returned was 0. Program unsupported.");
	m_TicksPerSecond = (float)countPerSecond;

	// Query the first measure of time to initialise the timer
	QueryPerformanceCounter((LARGE_INTEGER*)&m_BaseTime);
	m_PreviousTime = m_BaseTime;
}

Timer::~Timer()
{
}

const float& Timer::TotalTime()
{
	// ***Comments from Frank Luna DirectX 12 as they are very complete***
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// mStopTime - mBaseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from mStopTime:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (m_Stopped) m_TotalTime = (float)(m_StopTime - m_PausedTime - m_BaseTime) / m_TicksPerSecond;

	// The distance mCurrTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from mCurrTime:  
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime

	else m_TotalTime = (float)(m_CurrentTime - m_PausedTime - m_BaseTime) / m_TicksPerSecond;
	return m_TotalTime;
}

void Timer::Reset()
{
	// Query the current time and resets from there
	QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrentTime);

	// Just like in the constructor, both m_BaseTime and m_PrevTime are initialised to the same value
	m_BaseTime = m_CurrentTime;
	m_PreviousTime = m_CurrentTime;

	// Resume the timer
	m_Stopped = false;
	// m_StopTime is reset cause we are currently not stopped
	m_StopTime = 0;
}

void Timer::Start()
{
	// If we are resuming the timer..
	if (m_Stopped)
	{
		// Query the current time and starts from there
		QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrentTime);

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

void Timer::Stop()
{
	// Dont try to stop twice!
	if (!m_Stopped)
	{
		// Query the current time as m_StopTime
		// m_StopTime is therefore the time at which the timer stopped
		QueryPerformanceCounter((LARGE_INTEGER*)&m_StopTime);

		// Stop the timer
		m_Stopped = true;
	}
}

void Timer::Tick()
{
	// First check if the timer is supposed to be stopped, in which case delta time must be zero
	if (m_Stopped)
	{
		m_DeltaTime = 0.f;
		return;
	}

	// Otherwise, measure the current time and substract with the previous time
	// It results in the time passed in between the last measure and this function call,
	// which should be once per frame. Thus, we get the number of ticks between two frames
	// The last operation is therefore to divide it by the number of ticks per second, to
	// result of the time elapsed in seconds between two frames
	QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrentTime);
	m_DeltaTime = (float)(m_CurrentTime - m_PreviousTime) / m_TicksPerSecond;

	// Calculate the frames per seconds
	m_FPS = 1.f / m_DeltaTime;

	// Assign the current time as the previous time for the next frame
	m_PreviousTime = m_CurrentTime;

	// Deltatime can become negative if the processor goes into a power safe mode or
	// if we get shuffled to another processor
	if (m_DeltaTime < 0.f) m_DeltaTime = 0.f;
}
