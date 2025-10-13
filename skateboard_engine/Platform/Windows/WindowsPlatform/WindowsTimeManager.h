#pragma once
#include "Skateboard/Time/TimeManager.h"

namespace Skateboard
{
	class WindowsTimeManager : public TimeManager
	{
	public:
		WindowsTimeManager();
		virtual ~WindowsTimeManager() override;

		float& ElapsedTime() override;

		void Reset() override;
		void Start() override;
		void Stop() override;
		void Update() override;

	};

}