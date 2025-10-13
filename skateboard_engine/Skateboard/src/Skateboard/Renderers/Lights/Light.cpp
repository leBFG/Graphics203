#include "sktbdpch.h"

#define SKTBD_LOG_COMPONENT "LIGHTS"
#include "Skateboard/Log.h"

#include "Light.h"


namespace Skateboard
{
	Light::Light()
		:
		m_Diffuse(1.0f, 1.0f, 1.0f),
		m_FalloffStart(1.0f),
		m_Direction(0.1f, -1.0f, 0.4f),
		m_FalloffEnd(10.0f),
		m_Position(0.0f, 10.0f, 0.0f),
		m_SpotPower(64.0f),
		m_Radiance(0.01f, 0.01f, 0.01f),
		m_ProjectionMatrix	(glm::identity<float4x4>()),
		m_ViewMatrix		(glm::identity<float4x4>()),
		m_ViewProjTex		(glm::identity<float4x4>())
	{
	}

	Light::Light(const Light& rhs)
	{
		m_Diffuse			= rhs.m_Diffuse;
		m_FalloffStart		= rhs.m_FalloffStart;
		m_Direction			= rhs.m_Direction;
		m_FalloffEnd		= rhs.m_FalloffEnd;
		m_Position			= rhs.m_Position;
		m_SpotPower			= rhs.m_SpotPower;
		m_Radiance			= rhs.m_Radiance;
		m_ProjectionMatrix	= rhs.m_ProjectionMatrix;
		m_ViewMatrix		= rhs.m_ViewMatrix;
		m_ViewProjTex		= rhs.m_ViewProjTex;
	}

	auto Light::operator=(const Light& rhs) -> Light&
	{
		m_Diffuse			= rhs.m_Diffuse;
		m_FalloffStart		= rhs.m_FalloffStart;
		m_Direction			= rhs.m_Direction;
		m_FalloffEnd		= rhs.m_FalloffEnd;
		m_Position			= rhs.m_Position;
		m_SpotPower			= rhs.m_SpotPower;
		m_Radiance			= rhs.m_Radiance;
		m_ProjectionMatrix	= rhs.m_ProjectionMatrix;
		m_ViewMatrix		= rhs.m_ViewMatrix;
		m_ViewProjTex		= rhs.m_ViewProjTex;
		return *this;
	}

	void Light::SetDiffuse(float3 diffuse)
	{
		m_Diffuse = diffuse;
	}

	void Light::SetFalloffStart(float falloffStart)
	{
		m_FalloffStart = falloffStart;
	}

	void Light::SetDirection(float3 direction)
	{
		m_Direction = direction;
	}

	void Light::SetFalloffEnd(float falloffEnd)
	{
		m_FalloffEnd = falloffEnd;
	}

	void Light::SetPosition(float3 position)
	{
		m_Position = position;
	}

	void Light::SetSpotPower(float spotPower)
	{
		m_SpotPower = spotPower;
	}

	void Light::SetRadiance(float3 radiance)
	{
		m_Radiance = radiance;
	}

	void Light::GenerateOrthographicProjectionMatrix(float viewWidth, float viewHeight, float nearPlane, float farPlane)
	{
		m_ProjectionMatrix = glm::orthoLH_ZO(-viewWidth / 2.f, viewWidth / 2.f, -viewHeight / 2.f, viewHeight / 2.f, nearPlane, farPlane);
	}

	void Light::GeneratePerspectiveProjectionMatrix(float nearPlane, float farPlane)
	{
		SKTBD_CORE_WARN("This light class does not yet support perspective projection matrices!");
		m_ProjectionMatrix = glm::perspectiveLH(SKTBD_PI / 2.f, 1.f, nearPlane, farPlane);
	}

	void Light::GenerateViewMatrix()
	{
		float3 nDir = glm::normalize(m_Direction);

		// Direction = Target - position <-> position = target - direction
		float3 target = float3( 0.f, 0.f, 0.f );
		m_Position = (target - nDir) * float3(10.f, 10.0f, 10.0f);

		// TODO: Projection (point lights, spotlights) would probably want this instead
		//vector target = VectorAdd(dir, pos);

		// As the direction is variable, need to rotate up accordingly
		const float yAngle = glm::angle(float3(0.f, 1.f, 0.f), nDir);
		const float xAngle = glm::angle(float3(1.f, 0.f, 0.f), nDir);
		const float zAngle = glm::angle(float3(0.f, 0.f, 1.f), nDir);
		const float4x4 viewRotation = glm::yawPitchRoll(yAngle, xAngle, zAngle);
		vector up = { 0.f, 1.f, 0.f, 1.0f };
		up = viewRotation * up;

		// Regenerate the matrices
		m_ViewMatrix = glm::lookAtLH(m_Position, target, float3(up));
		GenerateTextureSpaceMatrix();
	}

	void Light::GenerateTextureSpaceMatrix()
	{
		// This matrix allows to transform a position from normalised device coordinates (NDC) to texture space (u,v)
		float4x4 T = {
			.5f,	0.f,	0.f,	0.f,
			.0f,	-.5f,	0.f,	0.f,
			.0f,	0.f,	1.f,	0.f,
			.5f,	.5f,	0.f,	1.f
		};
		//m_ViewProjTex = m_ViewMatrix * m_ProjectionMatrix * T;
		m_ViewProjTex = T * m_ProjectionMatrix * m_ViewMatrix;
	}
}
