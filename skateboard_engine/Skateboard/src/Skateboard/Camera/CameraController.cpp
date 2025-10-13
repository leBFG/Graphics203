#include <sktbdpch.h>
#include "CameraController.h"
#include "Camera.h"

#include "Skateboard/Input.h"
#include "Skateboard/Platform.h"

namespace Skateboard
{
	void CameraController::Update(PerspectiveCamera& camera, float dt)
	{
		bool hasMoved = camera.HasMoved();
		float3 position = camera.GetPosition();
		float3 rotation = camera.GetRotation();
		float moveSpeed = camera.GetMoveSpeed();
		const float sensitivity = camera.GetSensitivity();

		//// Get window specifics
		//Platform& platform = Platform::GetPlatform();
		//GraphicsContext* context = platform.GetGraphicsContext().get();
		//const int32_t clientWidth = context->GetClientWidth();
		//const int32_t clientHeight = context->GetClientHeight();

		hasMoved = false;


		//const float speedMultiplier = IsKeyDown(LOBYTE(VK_SHIFT)) ? 10.f : IsKeyDown(LOBYTE(VK_CONTROL)) ? .2f : 1.f;
		//moveSpeed *= speedMultiplier;
		float ly = Input::GetLeftStickY();
		if (ly < -0.05)
		{
			float Yangle = glm::radians(rotation.y);
			position.x += sinf(Yangle) * moveSpeed * dt * glm::abs(ly);    // Moving using the idea of a ZX trigonometric unit circle
			position.z += cosf(Yangle) * moveSpeed * dt * glm::abs(ly);    // sinf(0) = 0 -> no movement in X, cosf(0) = 1 -> movement in Z
			hasMoved = true;
		}
		if (Input::GetLeftStickY() > 0.05)
		{
			float Yangle = glm::radians(rotation.y);
			position.x -= sinf(Yangle) * moveSpeed * dt * glm::abs(ly);
			position.z -= cosf(Yangle) * moveSpeed * dt * glm::abs(ly);
			hasMoved = true;
		}

		float lx = Input::GetLeftStickX();
		if (lx < -0.05)
		{
			float Yangle = glm::radians(rotation.y);
			position.z += sinf(Yangle) * moveSpeed * dt * glm::abs(lx);
			position.x -= cosf(Yangle) * moveSpeed * dt * glm::abs(lx);
			hasMoved = true;
		}
		if (lx > 0.05)
		{
			float Yangle = glm::radians(rotation.y);
			position.z -= sinf(Yangle) * moveSpeed * dt * glm::abs(lx);
			position.x += cosf(Yangle) * moveSpeed * dt * glm::abs(lx);
			hasMoved = true;
		}
		if (Input::IsButtonDown(Pad_Button_R1))
		{
			position.y -= moveSpeed * dt;   // Lock the up movement to the world axis, do not consider the up vector (makes more sense)
			hasMoved = true;
		}
		if (Input::IsButtonDown(Pad_Button_L1))
		{
			position.y += moveSpeed * dt;
			hasMoved = true;
		}

		float ry = Input::GetRightStickY();
		if (ry < -0.05f)
		{
			rotation.x -= sensitivity * 15.f * dt * glm::abs(ry); // Using the keyboard, it needs to be about 5 times faster than the mouse sensibility to be coherent
			if (rotation.x < -CAMERA_PITCH_MAX) rotation.x = -CAMERA_PITCH_MAX;
			hasMoved = true;
		}
		if (ry > 0.05f)
		{
			rotation.x += sensitivity * 15.f * dt * ry;
			if (rotation.x > CAMERA_PITCH_MAX) rotation.x = CAMERA_PITCH_MAX;
			hasMoved = true;
		}

		float rx = Input::GetRightStickX();
		if (rx < -0.05f)
		{
			rotation.y -= sensitivity * 15.f * dt * glm::abs(rx);
			rotation.y += (rotation.y < 0.f) * 360.f;
			hasMoved = true;
		}
		if (rx > 0.05f)
		{
			rotation.y += sensitivity * 15.f * dt * rx;
			rotation.y -= (rotation.y > 360.f) * 360.f;
			hasMoved = true;
		}

		// Finally, update the camera matrix
		if (hasMoved)
		{
			camera.SetMoved(true);
			camera.SetPosition(position);
			camera.SetRotation(rotation);
			camera.UpdateViewMatrix();
		}
	}
}