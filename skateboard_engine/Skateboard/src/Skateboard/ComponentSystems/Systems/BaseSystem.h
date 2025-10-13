#pragma once
#include "Skateboard/Scene/ECS.h"
#include "Skateboard/Time/TimeManager.h"

namespace Skateboard
{
	class BaseSystem {
	public:
		virtual ~BaseSystem() = default;

		virtual void Init(entt::registry& reg) {};
		virtual void Shutdown() {};

		// These are just two of functions I have added in
		// You can add in an initialize function here too if that helps your project organisation!
		virtual void HandleInput(Skateboard::TimeManager* time, entt::registry& reg) {};
		virtual void RunUpdate(Skateboard::TimeManager* time, entt::registry& reg) = 0;
	};
}