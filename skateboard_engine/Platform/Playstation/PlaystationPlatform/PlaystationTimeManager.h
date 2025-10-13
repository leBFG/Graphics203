#pragma once
#include "Skateboard/Time/TimeManager.h"


namespace Skateboard
{

	class AGCTimeManager : public TimeManager
	{
	public:
		AGCTimeManager();
		virtual ~AGCTimeManager() override;

		float& ElapsedTime() override;

		void Reset() override;
		void Start() override;
		void Stop() override;
		void Update() override;

	};

}