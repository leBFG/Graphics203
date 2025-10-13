#pragma once


#include <string>
#include <vector>

#include "Skateboard/Core.h"
#include "Skateboard/Camera/Camera.h"
#include "Skateboard/Mathematics.h"

#include "entt.hpp"
#include "ECS.h"

namespace Skateboard
{
	class Entity;

	struct TagComponent
	{
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& string)
			:
			tag(string) {}
	};

	struct TransformComponent
	{
		/*
		float3 Translation	= { 0.f,0.f,0.f };
		float3 Rotation		= { 0.f, 0.f,0.f };
		float3 Scale		= { 1.f, 1.f, 1.f };
		*/

		Transform m_transform;

		TransformComponent() = default;
		~TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& trans)
		{
			m_transform.Translation = trans;
		}

		operator Transform& () { return m_transform; };
		operator const Transform () { return m_transform; };
		operator const Transform& () { return m_transform; };
		operator const glm::mat4x4 () { return m_transform.AsMatrix(); };
		
		[[nodiscard]] glm::mat4x4 GetTransform() const
		{
			/*auto euler = glm::eulerAngles(m_transform.Rotation);
			return	glm::translate(m_transform.Translation) *
					glm::yawPitchRoll(glm::radians(euler.y), glm::radians(euler.x), glm::radians(euler.z)) *
					glm::scale(m_transform.Scale);
		*/
			return m_transform.AsMatrix();
		}
	};

	struct CameraComponent 
	{
		CameraComponent() = default;
		DISABLE_COPY(CameraComponent)
		ENABLE_MOVE(CameraComponent)


		std::unique_ptr<Camera> Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

	};

	//Forward Declarations
	class BaseNativeScript;
	class TimeManager;


	struct NativeScriptComponent
	{
		BaseNativeScript* Instance = nullptr;

		std::function <void()> InstantiateFunction;
		std::function <void()> DestroyInstanceFunction;

		std::function<void()> OnCreateFunction;
		std::function<void()> OnDestroyFunction;
		std::function<void(TimeManager*)> OnHandleInputFunction;
		std::function<void(TimeManager*)> OnUpdateFunction;

		void (*DestoryScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			//TODO: I Removed this for compilation on the PS5! It's giving massive warnings
			InstantiateFunction = [&]() {Instance = new T; };
			DestroyInstanceFunction = [&]() {delete (T*)Instance; Instance = nullptr; };

			OnCreateFunction = [&]() {((T*)Instance)->OnCreate();};
			OnDestroyFunction = [&]() {((T*)Instance)->OnDestroy();};
			OnUpdateFunction = [&](TimeManager* tm) {((T*)Instance)->OnUpdate(tm);};
			OnHandleInputFunction = [&](TimeManager* tm) {((T*)Instance)->OnHandleInput(tm);};

		}
	};
}


