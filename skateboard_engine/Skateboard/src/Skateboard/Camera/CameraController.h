#pragma once
#include "Skateboard/Core.h"
#include "Camera.h"

namespace Skateboard
{

	class CameraController
	{
	public:
		CameraController() = default;
		DISABLE_COPY_AND_MOVE(CameraController);
		virtual ~CameraController() = default;

		void Update(PerspectiveCamera& camera, float dt);

	private:

	};


}