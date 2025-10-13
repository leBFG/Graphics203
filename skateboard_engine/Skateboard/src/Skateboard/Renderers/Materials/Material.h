#pragma once
#include "Skateboard/Mathematics.h"

namespace Skateboard
{
//	struct SKTBDMaterialGPUData
//	{
//		float4 m_Albedo;
//		int32_t m_AlbedoMapIndex;
//		int3 padding;
//		float3 m_FresnelR0;
//		float m_Metallic;
//		float3 m_Specular;
//		float m_Roughness;
//	};
//
//	class Material
//	{
//	public:
//		Material();
//
//		Material(const Material& rhs) noexcept;
//		Material& operator=(const Material& rhs) noexcept;
//		Material(Material&& rhs) noexcept = delete;
//		auto operator=(Material&& rhs) noexcept = delete;
//		~Material() = default;
//
//		void SetAlbedo(float4 albedo) { m_Data.m_Albedo = albedo; }
//		[[nodiscard]] float4 GetAlbedo() const { return m_Data.m_Albedo; }
//
//		void SetAlbedoMapIndex(int32_t map) { m_Data.m_AlbedoMapIndex = map; }
//		[[nodiscard]] int32_t GetAlbedoMapIndex() const { return m_Data.m_AlbedoMapIndex; }
//
//		void SetFresnel(float3 fresnel) { m_Data.m_FresnelR0 = fresnel; }
//		[[nodiscard]] float3 GetFresnel() const { return m_Data.m_FresnelR0; }
//
//		void SetMetallic(float metallic) { m_Data.m_Metallic = metallic; }
//		[[nodiscard]] float GetMetallic() const { return m_Data.m_Metallic; }
//
//		void SetSpecular(float3 specular) { m_Data.m_Specular = specular; }
//		[[nodiscard]] float3 GetSpecular() const { return m_Data.m_Specular; }
//
//		void SetRoughness(float roughness) { m_Data.m_Roughness = roughness; }
//		[[nodiscard]] float GetRoughness() const { return m_Data.m_Roughness; }
//
//		void SetPipelineName(const std::string_view& name) { m_PipelineName = name; }
//		void SetPipelineName(int32_t index) { m_PipelineIndex = index; }
//
//		const std::string& GetPipelineName() const { return m_PipelineName; }
//		int32_t GetPipelineIndex() const { return m_PipelineIndex; }
//
//		[[nodiscard]] const SKTBDMaterialGPUData& GetMaterialData() const { return m_Data; }
//		[[nodiscard]] SKTBDMaterialGPUData& GetMaterialData() { return m_Data; }
//
//		SKTBDMaterialGPUData GetData() { return m_Data; };
//	private:
//		SKTBDMaterialGPUData m_Data;
//
//		std::string m_PipelineName;
//		int32_t m_PipelineIndex;
//	};
}

