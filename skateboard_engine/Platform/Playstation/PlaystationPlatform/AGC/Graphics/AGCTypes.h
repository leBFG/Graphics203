#pragma once

#include "Skateboard/Graphics/InternalFormats.h"

namespace sce {
	namespace Agc {
		namespace Core
		{
			struct DataFormat;
		}
	}
}

#include <agc.h>
#include <shader.h>

namespace Skateboard
{

sce::Agc::Core::Texture::Type SkateboardTextureDimensionToAGC(TextureDimension_ dimension);
TextureDimension_ AGCTextureDimensionToSkateboard(sce::Agc::Core::Texture::Type type);
//D3D12_RAYTRACING_GEOMETRY_TYPE GeometryTypeToAGC(GeometryType_ type);
//D3D12_HIT_GROUP_TYPE RaytracingHitGroupTypeToAGC(RaytracingHitGroupType_ type);
sce::Agc::Core::DataFormat ShaderDataTypeToAGC(ShaderDataType_ type);
//D3D12_SHADER_VISIBILITY ShaderVisibilityToAGC(ShaderVisibility_ v);
//D3D12_DESCRIPTOR_RANGE_TYPE ShaderDescriptorTableTypeToAGC(ShaderDescriptorTableType_ type);
//D3D12_FILTER SamplerFilterToAGC(SamplerFilter_ filter);
//D3D12_TEXTURE_ADDRESS_MODE SamplerModeToAGC(SamplerMode_ mode);
//D3D12_COMPARISON_FUNC SamplerComparisonFunctionToAGC(SamplerComparisonFunction_ function);
//D3D12_STATIC_BORDER_COLOR SamplerBorderColourToAGC(SamplerBorderColour_ colour);
sce::Agc::Core::DataFormat DepthStencilToSRVAGC(DataFormat_ format);
sce::Agc::Core::DataFormat SkateboardDataFormatToAGC(DataFormat_ format);
sce::Agc::CxRenderTarget::Dimension SkateboardTextureDimensionToAGCRenderTarget(TextureDimension_);
DataFormat_ AGCBufferFormatToSkateboard(sce::Agc::Core::DataFormat format);
//D3D12_RESOURCE_STATES ResourceTypeToStateAGC(GPUResourceType_ type);
SceShaderResourceClass ShaderElementTypeToAGCResourceClass(ShaderElementType_ type);
ShaderElementType_ AGCResourceClassToShaderElementType(SceShaderResourceClass type);
sce::Agc::Core::Sampler::WrapMode SamplerModeToAGC(SamplerMode_ mode);
sce::Agc::Core::Sampler::DepthCompare SamplerComparisonFunctionToAGC(SamplerComparisonFunction_ function);
sce::Agc::Core::Sampler::BorderColor SamplerBorderColourToAGC(SamplerBorderColour_ colour);
void SetFilterMode(sce::Agc::Core::Sampler& inout_sampler, SamplerFilter_ filter, SamplerComparisonFunction_ comparisonFunc);

}