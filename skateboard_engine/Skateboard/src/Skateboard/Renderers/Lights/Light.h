#pragma once
#include "Skateboard/Mathematics.h"


namespace Skateboard
{
	//class Light
	//{
	//public:
	//	explicit Light();
	//	Light(const Light& rhs);
	//	auto operator=(const Light& rhs) -> Light&;

	//	Light(Light&& rhs) noexcept = delete;
	//	auto operator=(Light&& rhs) noexcept = delete;

	//	~Light() = default;

	//	// <summary>
	//	void SetDiffuse(float3 diffuse);
	//	[[nodiscard("")]] float3 GetDiffuse() const { return m_Diffuse; }

	//	void SetFalloffStart(float falloffStart);
	//	[[nodiscard("")]] float GetFalloffStart() const { return m_FalloffStart; }

	//	void SetDirection(float3 direction);
	//	[[nodiscard("")]] float3 GetDirection() const { return m_Direction; }

	//	void SetFalloffEnd(float falloffEnd);
	//	[[nodiscard("")]] float GetFalloffEnd() const { return m_FalloffEnd; }

	//	void SetPosition(float3 position);
	//	[[nodiscard("")]] float3 GetPosition() const { return m_Position; }

	//	void SetSpotPower(float spotPower);
	//	[[nodiscard("")]] float GetSpotPower() const { return m_SpotPower; }

	//	void SetRadiance(float3 radiance);
	//	[[nodiscard("")]] float3 GetRadiance() const { return m_Radiance; }

	//	void GenerateOrthographicProjectionMatrix(float viewWidth, float viewHeight, float nearPlane, float farPlane);
	//	void GeneratePerspectiveProjectionMatrix(float nearPlane, float farPlane);
	//	void GenerateViewMatrix();
	//	void GenerateTextureSpaceMatrix();

	//	[[nodiscard("")]] const float4x4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	//	[[nodiscard("")]] const float4x4& GetViewMatrix() const { return m_ViewMatrix; }
	//	[[nodiscard("")]] const float4x4& GetTextureSpaceMatrix() const { return m_ViewProjTex; }

	//private:

	//	float3 m_Diffuse;
	//	float m_FalloffStart;
	//	float3 m_Direction;
	//	float m_FalloffEnd;
	//	float3 m_Position;
	//	float m_SpotPower;
	//	float3 m_Radiance;

	//	float4x4 m_ProjectionMatrix;
	//	float4x4 m_ViewMatrix;
	//	float4x4 m_ViewProjTex;
	//};
}

