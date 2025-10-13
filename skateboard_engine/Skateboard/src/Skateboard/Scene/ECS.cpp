#include "sktbdpch.h"
#include "ECS.h"
#include "Entity.h"
#include "Components.h"

#define SKTBD_LOG_COMPONENT "ECS SYSTEM"
#include "Skateboard/Log.h" 

namespace Skateboard 
{
	Entity ECS::CreateEntity(const std::string_view& name, entt::registry& Registry)
	{
		Entity entity = { Registry.create(), &Registry } ;
		TagComponent& tag = entity.AddComponent<TagComponent>();
		tag.tag = name.empty() ? "Entity" : name;
		return entity;
	}


	Entity ECS::GetEntityByTag(const std::string_view& tag, entt::registry& Registry)
	{

		// This function is surely convinent, but very inefficient -- please do not use this often.
		auto group = Registry.view<TagComponent>();
		for (auto entity : group) 
		{
			const auto& tagComp = group.get<TagComponent>(entity);
			if (tagComp.tag == tag) {
				return { entt::to_entity(Registry.storage<TagComponent>(), tagComp),  &Registry };
			}
		}
		
		return Entity();
	}


	


}