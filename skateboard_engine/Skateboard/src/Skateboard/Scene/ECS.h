#pragma once
#include <cstdint>
#include "entt.hpp"

namespace Skateboard {

	class BaseComponent;
	class Entity;

	class ECS 
	{
	public:
		ECS() = default;
		~ECS() = default;

		static Entity CreateEntity(const std::string_view& name, entt::registry& Registry);

		//Slow
		static Entity GetEntityByTag(const std::string_view& tag, entt::registry& Registry);
	};

	typedef ECS ECSSystem;
}