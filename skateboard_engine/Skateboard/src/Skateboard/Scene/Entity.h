#pragma once
#include <cstdint>

#include "entt.hpp"
#include "Scene.h"
#include "ECS.h"

namespace Skateboard
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, entt::registry* owner) : m_EntityHandle(handle), m_OwningRegistry(owner) {};
		Entity(const Entity& other) = default;

		template<typename T, typename ... Args> T& AddComponent(Args&&... args)
		{
			if (HasComponent<T>())
			{
				SKTBD_MSG_WARN("Tried to add a component that already exists. Cancelling operation..");
				return GetComponent<T>();
			}
			T& component = m_OwningRegistry->emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

			return component;
		}

		// Can only add one per entity as of now
		template<typename T, typename ... Args> NativeScriptComponent& AddNativeScriptComponent(Args&&... args) {
			NativeScriptComponent& nsc = m_OwningRegistry->emplace<NativeScriptComponent>(m_EntityHandle, std::forward<Args>(args) ...);
			nsc.Bind<T>();
			nsc.InstantiateFunction();
			((T*)nsc.Instance)->SetEntity(m_EntityHandle, m_OwningRegistry);
			return nsc;
		};

		template<typename T> T& GetComponent() const
		{
			ASSERT(HasComponent<T>(), "Entity  does not have component!");

			return m_OwningRegistry->get<T>(m_EntityHandle);
		}

		template<typename T> bool HasComponent() const
		{
			return m_OwningRegistry->any_of<T>(m_EntityHandle);
		}

		template<typename T> void RemoveComponent() const
		{
			ASSERT(HasComponent<T>(), "Entity does not have the component to be removed!");

			T componentCopy = m_OwningRegistry->get<T>(m_EntityHandle);
			m_OwningRegistry->remove<T>(m_EntityHandle);
		}

		operator bool() const { return m_EntityHandle != entt::null; }

		// @brief Implicit conversion operator to an unsigned 32-bit integer.
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		// @brief Implicit conversion operator to a entity handle.
		operator entt::entity() const { return m_EntityHandle; }

		// @brief Comparator operator for a valid entity handle.
		bool operator ==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_OwningRegistry == other.m_OwningRegistry;
		}

		// @brief Comparator operator for entity object comparisons.
		bool operator !=(const Entity& other) const
		{
			return !(*this == other);
		}

		entt::entity GetEnttHandle() const { return m_EntityHandle; }
		entt::registry* GetOwningRegistry() const { return m_OwningRegistry; }

	private:
		entt::entity m_EntityHandle{ entt::null };
		entt::registry* m_OwningRegistry;
	};
}


