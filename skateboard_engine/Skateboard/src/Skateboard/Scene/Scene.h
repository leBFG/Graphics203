#pragma once

#include "Skateboard/SizedPtr.h"
#include "Skateboard/Scene/AABB.h"

#include "Skateboard/Camera/CameraController.h"
#include "Skateboard/Renderers/Lights/Light.h"
#include "Skateboard/Scene/Components.h"
#include "Skateboard/Input.h"
#include "Skateboard/Events/Event.h"
#include "Skateboard/Time/TimeManager.h"
#include "PipelineMap.h"
#include "ECS.h"

namespace Skateboard
{
	class Entity;

	class Scene
	{
		friend class Entity;
		friend class SceneBuilder;
	public:
		explicit Scene(const std::string& name);

		Scene() = delete;
		DISABLE_COPY_AND_MOVE(Scene);
		virtual ~Scene();

		virtual void OnHandleInput(TimeManager* time);
		virtual void OnUpdate(TimeManager* time);
		virtual void OnRender();
		virtual void OnImGuiRender() {};

		virtual void OnSceneEnter() {};
		virtual void OnSceneExit() {};

		virtual void OnEvent(Skateboard::Event& e) {};

		inline const int GetSceneIndex() { return sceneIndex; };

	protected:
		std::string sceneName;
		int sceneIndex;
	};
}
