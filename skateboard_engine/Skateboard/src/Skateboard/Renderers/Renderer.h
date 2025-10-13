#pragma once

#include "sktbdpch.h"
#include "Skateboard/Log.h"

namespace Skateboard
{
	class GraphicsContext;

	struct SKTBDPassBuffer
	{
		float4x4 ViewMatrix;
		float4x4 ProjectionMatrix;
		float4x4 ViewMatrixInverse;
		float4x4 ProjectionMatrixInverse;
		float3 CameraPosition;
		float Padding;
	};

	struct SKTBDLightBuffer
	{
		float3	Diffuse{ 0.9f, 0.9f, 0.9f };
		float	FalloffStart{ 1.0f };
		float3	Direction{ 0.f, -1.0f, 0.0f };
		float	FalloffEnd{ 10.0f };
		float3	Position{ 0.f, 5.0f, 0.f };
		float	SpotPower{ 64.0f };
		float3	Radiance{ 0.01f, 0.01f, 0.01f };
		float	Pad0{ 0.f };
		float4x4 LightViewProjTex;
	};

	/*#define MAX_GEOMETRY 8
	#define MAX_INSTANCES 64*/
	#define MAX_BONE_COUNT 128

	struct AnimWorld
	{
		glm::mat4x4 WorldTransform;
		glm::mat4x4 BoneTransforms[MAX_BONE_COUNT];
	};

	/*struct FrameData
	{
		glm::mat4x4 ViewMatrix;
		glm::mat4x4 ProjectionMatrix;
		glm::mat4x4 CameraMatrix;
	};*/

	class IRendererSystem
	{
		//renderers should not be copyable or movable
		DISABLE_COPY_AND_MOVE(IRendererSystem)

	public:
		IRendererSystem() = default;
		virtual ~IRendererSystem() = default;

		virtual void Init() = 0;
		virtual void Begin() = 0;
		virtual void Update() = 0;
		virtual void End() = 0;
		virtual void Shutdown() = 0;

		virtual void OnRegistered(entt::registry* registry, const std::string name){};
		virtual void OnUnregistered(const std::string name){};

		entt::registry* Register(entt::registry* registry, const std::string name)
		{
			if (m_Registries.contains(name))
			{
				SKTBD_MSG_WARN("Render System already has {} ", name.c_str());
				return m_Registries[name];
			}
			else
			{
				m_Registries[name] = registry;
				OnRegistered(registry, name);
				return registry;
			}
		}

		void Unregister(const std::string& name)
		{
			if (m_Registries.contains(name))
			{
				OnUnregistered(name);
				m_Registries.erase(name);
			}
			else
			{
				SKTBD_MSG_WARN("Render System has no {} registered with it ", name.c_str());
			}
		}

	protected:
		std::unordered_map<std::string, entt::registry*> m_Registries;
	};

}