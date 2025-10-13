#include "sktbdpch.h"
#include "Pipeline.h"

#define SKTBD_LOG_COMPONENT "PIPELINE"
#include "Skateboard/Log.h"

namespace Skateboard
{
	SamplerDesc SamplerDesc::InitAsDefaultTextureSampler()
	{
		SamplerDesc desc = {};
		desc.Filter = SamplerFilter_Anisotropic;
		desc.ModeU = SamplerMode_Wrap;
		desc.ModeV = SamplerMode_Wrap;
		desc.ModeW = SamplerMode_Wrap;
		desc.MipMapLevelOffset = 0.f;
		desc.MipMapMinSampleLevel = 0.f;
		desc.MipMapMaxSampleLevel = FLT_MAX;
		desc.MaxAnisotropy = 16u;
		desc.ComparisonFunction = SamplerComparisonFunction_Never;
		desc.BorderColour = SamplerBorderColour_White;
		desc.Flags = SamplerFlags_NONE;
		return desc;
	}

	SamplerDesc SamplerDesc::InitAsDefaultShadowSampler()
	{
		SamplerDesc desc = {};
		desc.Filter = SamplerFilter_Min_Mag_Linear_Mip_Point;
		desc.ModeU = SamplerMode_Border;
		desc.ModeV = SamplerMode_Border;
		desc.ModeW = SamplerMode_Border;
		desc.MipMapLevelOffset = 0.f;
		desc.MipMapMinSampleLevel = 0.f;
		desc.MipMapMaxSampleLevel = FLT_MAX;
		desc.MaxAnisotropy = 16u;
		desc.ComparisonFunction = SamplerComparisonFunction_Less_Equal;
		desc.BorderColour = SamplerBorderColour_Black;
		return desc;
	}

	void ShaderInputLayoutDesc::AddRootConstant(uint32_t num32bitvalues, uint32_t shaderRegister, uint32_t shaderRegisterSpace, ShaderVisibility_ shaderVisibility)
	{
		ShaderResourceDesc desc;
		desc.ShaderElementType = ShaderElementType_RootConstant;
		desc.Constant.Num32BitValues = num32bitvalues;
		desc.Constant.ShaderRegister = shaderRegister;
		desc.Constant.RegisterSpace = shaderRegisterSpace;
		desc.ShaderVisibility = shaderVisibility;
		vPipelineInputs.emplace_back(std::move(desc));
	}


	void ShaderInputLayoutDesc::AddConstantBufferView(uint32_t shaderRegister, uint32_t shaderRegisterSpace, ShaderVisibility_ shaderVisibility)
	{
		ShaderResourceDesc desc;

		desc.ShaderElementType = ShaderElementType_ConstantBufferView;
		desc.Descriptor.ShaderRegister = shaderRegister;
		desc.Descriptor.RegisterSpace = shaderRegisterSpace;
		desc.ShaderVisibility = shaderVisibility;

		vPipelineInputs.emplace_back(desc);
	}

	void ShaderInputLayoutDesc::AddShaderResourceView(uint32_t shaderRegister, uint32_t shaderRegisterSpace, ShaderVisibility_ shaderVisibility)
	{
		ShaderResourceDesc desc;
		desc.ShaderElementType = ShaderElementType_ShaderResourceView;
		desc.Descriptor.ShaderRegister = shaderRegister;
		desc.Descriptor.RegisterSpace = shaderRegisterSpace;
		desc.ShaderVisibility = shaderVisibility;
		vPipelineInputs.emplace_back(desc);
	}

	void ShaderInputLayoutDesc::AddUnorderedAccessView(uint32_t shaderRegister, uint32_t shaderRegisterSpace, ShaderVisibility_ shaderVisibility)
	{
		ShaderResourceDesc desc;
		desc.ShaderElementType = ShaderElementType_UnorderedAccessView;
		desc.Descriptor.ShaderRegister = shaderRegister;
		desc.Descriptor.RegisterSpace = shaderRegisterSpace;
		desc.ShaderVisibility = shaderVisibility;
		vPipelineInputs.emplace_back(desc);
	}

	void ShaderInputLayoutDesc::AddDescriptorTable(DescriptorTableLayout DescTable, ShaderVisibility_ shaderVisibility)
	{
		ShaderResourceDesc desc = {};
		desc.ShaderElementType = ShaderElementType_DescriptorTable;
		desc.ShaderVisibility = shaderVisibility;
		desc.DescriptorTable = DescTable;
		vPipelineInputs.emplace_back(std::move(desc));
	}

	void ShaderInputLayoutDesc::AddStaticSampler(const SamplerDesc& desc, uint32_t ShaderRegister, uint32_t ShaderRegisterSpace, ShaderVisibility_ ShaderVisibility)
	{
		SamplerSlotDesc Slot = { desc, ShaderRegister, ShaderRegisterSpace, ShaderVisibility };

		vStaticSamplers.emplace_back(Slot);
	}

	void RaytracingPipelineDesc::SetRaytracingLibrary(const wchar_t* libraryFilename, const wchar_t* raygenEntryPoint)
	{
		RaytracingShaders.FileName = libraryFilename;
		RaytracingShaders.RayGenShaderEntryPoint = raygenEntryPoint;
	}
	void RaytracingPipelineDesc::AddHitGroup(const wchar_t* hitGroupName, const wchar_t* anyHitEntryPoint, const wchar_t* closestHitEntryPoint, const wchar_t* intersectionEntryPoint, RaytracingHitGroupType_ type)
	{
		RaytracingHitGroup group = {};
		group.HitGroupName = hitGroupName;
		group.AnyHitShaderEntryPoint = anyHitEntryPoint;
		group.ClosestHitShaderEntryPoint = closestHitEntryPoint;
		group.IntersectionShaderEntryPoint = intersectionEntryPoint;
		group.Type = type;
		RaytracingShaders.HitGroups.emplace_back(std::move(group));
	}
	void RaytracingPipelineDesc::AddMissShader(const wchar_t* missEntryPoint)
	{
		RaytracingShaders.MissShaderEntryPoints.push_back(missEntryPoint);
	}
	void RaytracingPipelineDesc::AddCallableShader(const wchar_t* shaderEntryPoint)
	{
		RaytracingShaders.CallableShaders.push_back(shaderEntryPoint);
	}
	void RaytracingPipelineDesc::SetConfig(uint32_t maxPayloadSize, uint32_t maxAttributeSize, uint32_t maxRecursionDepth, uint32_t maxCallableShaderRecursionDepth)
	{
		MaxPayloadSize = maxPayloadSize;
		MaxAttributeSize = maxAttributeSize;
		MaxTraceRecursionDepth = maxRecursionDepth;
		MaxCallableShaderRecursionDepth = maxCallableShaderRecursionDepth;
	}

}