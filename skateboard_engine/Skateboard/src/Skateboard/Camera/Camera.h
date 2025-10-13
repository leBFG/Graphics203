#pragma once
#include "SkateBoard/Mathematics.h"

#define CAMERA_PITCH_MAX 90.f
#define CAMERA_DEFAULT_MOVESPEED 15.f
#define CAMERA_DEFAULT_SENSITIVITY 10.f

namespace Skateboard
{
	class Camera
	{
	protected:
		Camera();
		Camera(float nearPlane, float farPlane, float3 position, float3 target, float3 up);

	public:
		virtual ~Camera() {}

		void SetPosition(const float3& position) { m_Position = position; }
		void SetRotation(const float3& rotation) { m_Rotation = rotation; }
		void SetNearPlane(float nearPlane) { m_NearPlane = nearPlane; }
		void SetFarPlane(float farPlane) { m_FarPlane = farPlane; }

		const float3& GetPosition() const { return m_Position; }
		const float3& GetRotation() const { return m_Rotation; }
		const float4x4& GetViewMatrix() const { return m_ViewMatrix; }
		const float4x4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		[[nodiscard("")]] float GetFarPlane() const { return m_FarPlane; }
		[[nodiscard("")]] float GetNearPlane() const { return m_NearPlane; }

		void UpdateViewMatrix();
		void UpdateViewMatrix( Transform& T);

	protected:
		float m_NearPlane, m_FarPlane;
		float3 m_Position, m_Rotation;
		float4x4 m_ViewMatrix, m_ProjectionMatrix;
	};

	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera();
		PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane, float3 position = {0.f,0.f,-10.f}, float3 target = { 0.f,0.f,0.f }, float3 up = { 0.f,1.f,0.f });
		virtual ~PerspectiveCamera() override {};

		void Build(float fov, float aspectRatio, float nearPlane, float farPlane, float3 position, float3 target, float3 up);

		const bool HasMoved() const { return m_HasMoved; };
		const float GetMoveSpeed() const { return m_MovementSpeed; }
		const float GetSensitivity() const { return m_Sensitivity; }
		[[nodiscard("")]] float GetFov() const { return m_Fov; }
		void SetMoved(bool hasMoved) { m_HasMoved = hasMoved; }
		void SetFov(float fov);
		void SetFrustum(float nearPlane, float farPlane);
		void SetMoveSpeed(float ms) { m_MovementSpeed = ms; }
		void SetSensitivity(float ls) { m_Sensitivity = ls; }
		void OnResize(int newClientWidth, int newClientHeight);

	private:
		float m_MovementSpeed, m_Sensitivity;
		float m_Fov, m_AspectRatio;
		bool m_HasMoved;
	};

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float viewWidth, float viewHeight, float nearPlane, float farPlane, float3 position = {0.f,0.f,0.f}, float3 target = { 0.f,0.f,0.f }, float3 up = { 0.f,1.f,0.f });
		virtual ~OrthographicCamera() override {}

		void OnResize(int newClientWidth, int newClientHeight);

	private:

	};
}