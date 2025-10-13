#include "sktbdpch.h"
#include "Camera.h"

namespace Skateboard
{
	Camera::Camera() :
		m_NearPlane(0.f),
		m_FarPlane(0.f),
		m_Position(0.f, 0.f, 0.f),
		m_Rotation(0.f, 0.f, 0.f),
		m_ViewMatrix(glm::identity<float4x4>()),
		m_ProjectionMatrix(glm::identity<float4x4>())
	{
	}

	Camera::Camera(float nearPlane, float farPlane, float3 position, float3 target, float3 up) :
		m_NearPlane(nearPlane),
		m_FarPlane(farPlane),
		m_Position(position),
		m_Rotation(0.f, 0.f, 0.f),
		m_ViewMatrix(glm::lookAtLH(m_Position, target, up)),
		m_ProjectionMatrix(glm::identity<float4x4>())
	{
	}
	void Camera::UpdateViewMatrix( Transform& trans)
	{
		vector up = vector(0.f, 1.f, 0.f, 1.f);  // Set w to 1 for the following matrix multiplication
		vector position = vector(trans.Translation,0);
		vector lookAt = vector(0.f, 0.f, 1.f, 1.f);

		// Init rotation matrix
		const float4x4 viewRotation =  glm::toMat4(trans.Rotation);

		// Transform the lookAt and up vectors based on the current view rotation
		up = viewRotation * up;
		lookAt = viewRotation * lookAt;

		// Translate the target position to the position of the camera
		lookAt = lookAt + position;

		// Finally, create the view matrix
		m_ViewMatrix = glm::lookAtLH(float3(position), float3(lookAt), float3(up));

		
	}
	void Camera::UpdateViewMatrix()
	{
		// Init data
		vector up = vector(0.f, 1.f, 0.f, 1.f);  // Set w to 1 for the following matrix multiplication
		vector position = vector(m_Position.x, m_Position.y, m_Position.z, 0.f);
		vector lookAt = vector(0.f, 0.f, 1.f, 1.f);

		// Init rotation matrix
		const float yaw = glm::radians(m_Rotation.y);
		const float pitch = glm::radians(m_Rotation.x);
		const float roll = glm::radians(m_Rotation.z);
		const float4x4 viewRotation = glm::yawPitchRoll(yaw, pitch, roll);

		// Transform the lookAt and up vectors based on the current view rotation
		up = viewRotation * up;
		lookAt = viewRotation * lookAt;

		// Translate the target position to the position of the camera
		lookAt = lookAt + position;

		// Finally, create the view matrix
		m_ViewMatrix = glm::lookAtLH(float3(position), float3(lookAt), float3(up));
	}

	PerspectiveCamera::PerspectiveCamera():
		m_MovementSpeed(CAMERA_DEFAULT_MOVESPEED),
		m_Sensitivity(CAMERA_DEFAULT_SENSITIVITY),
		m_Fov(0.f),
		m_AspectRatio(0.f),
		m_HasMoved(false)
	{
	}

	PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane, float3 position, float3 target, float3 up) :
		Camera(nearPlane, farPlane, position, target, up),
		m_MovementSpeed(CAMERA_DEFAULT_MOVESPEED),
		m_Sensitivity(CAMERA_DEFAULT_SENSITIVITY),
		m_Fov(fov),
		m_AspectRatio(aspectRatio),
		m_HasMoved(false)
	{
		m_ProjectionMatrix = glm::perspectiveLH(m_Fov, aspectRatio, m_NearPlane, m_FarPlane);
		//Build(m_Fov, aspectRatio, m_NearPlane, m_FarPlane, position, target, up);
	}

	void PerspectiveCamera::Build(float fov, float aspectRatio, float nearPlane, float farPlane, float3 position, float3 target, float3 up)
	{
		// Build projection
		m_ProjectionMatrix = glm::perspectiveLH(fov, aspectRatio, nearPlane, farPlane);

		// Build the view matrix
		m_ViewMatrix = glm::lookAtLH(position, target, up);

		// Update relevant members
		m_Fov = fov;
		m_AspectRatio = aspectRatio;
		m_NearPlane = nearPlane;
		m_FarPlane = farPlane;
		m_Position = position;
	}

	void PerspectiveCamera::SetFov(float fov)
	{
		m_Fov = fov;
		m_ProjectionMatrix = glm::perspectiveLH(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
	}

	void PerspectiveCamera::SetFrustum(float nearPlane, float farPlane)
	{
		m_NearPlane = nearPlane;
		m_FarPlane = farPlane;
		m_ProjectionMatrix = glm::perspectiveLH(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
	}

	void PerspectiveCamera::OnResize(int newClientWidth, int newClientHeight)
	{
		m_AspectRatio = static_cast<float>(newClientWidth) / newClientHeight;
		m_ProjectionMatrix = glm::perspectiveLH(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
	}

	OrthographicCamera::OrthographicCamera(float viewWidth, float viewHeight, float nearPlane, float farPlane, float3 position, float3 target, float3 up) :
		Camera(nearPlane, farPlane, position, target, up)
	{
		m_ProjectionMatrix = glm::orthoLH_ZO( - viewWidth / 2.f, viewWidth / 2.f, -viewHeight / 2.f, viewHeight / 2.f, nearPlane, farPlane);
		
	}

	void OrthographicCamera::OnResize(int newClientWidth, int newClientHeight)
	{
		m_ProjectionMatrix = glm::orthoLH_ZO(-static_cast<float>(newClientWidth) / 2.f, static_cast<float>(newClientWidth) / 2.f, -static_cast<float>(newClientHeight) / 2.f, static_cast<float>(newClientHeight) / 2.f, m_NearPlane, m_FarPlane);
	}
}