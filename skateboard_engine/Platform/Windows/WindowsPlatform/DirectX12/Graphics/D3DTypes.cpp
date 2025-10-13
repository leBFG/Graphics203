#include "sktbdpch.h"
#include "D3DTypes.h"

#define SKTBD_LOG_COMPONENT "DATA TYPE MAPPINGS DX12"
#include "Skateboard/Log.h"

namespace Skateboard
{

	D3D12_COMPARISON_FUNC SamplerComparisonFunctionToD3D(SamplerComparisonFunction_ function)
	{
		switch (function)
		{
		case SamplerComparisonFunction_Never:			return D3D12_COMPARISON_FUNC_NEVER;
		case SamplerComparisonFunction_Less:			return D3D12_COMPARISON_FUNC_LESS;
		case SamplerComparisonFunction_Equal:			return D3D12_COMPARISON_FUNC_EQUAL;
		case SamplerComparisonFunction_Less_Equal:		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case SamplerComparisonFunction_Greater:			return D3D12_COMPARISON_FUNC_GREATER;
		case SamplerComparisonFunction_Not_Equal:		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case SamplerComparisonFunction_Greater_Equal:	return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case SamplerComparisonFunction_Always:			return D3D12_COMPARISON_FUNC_ALWAYS;
		default:										return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		}
	}

	D3D12_HIT_GROUP_TYPE RaytracingHitGroupTypeToD3D(RaytracingHitGroupType_ type)
	{
		switch (type)
		{
		case RaytracingHitGroupType_Triangles:		return D3D12_HIT_GROUP_TYPE_TRIANGLES;
		case RaytracingHitGroupType_Procedural:		return D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
		default:	SKTBD_LOG_ASSERT(false, "Invalid type supplied on this hit group!"); return D3D12_HIT_GROUP_TYPE_TRIANGLES;
		}
	}

	D3D12_RESOURCE_DIMENSION SkateboardTextureDimensionToD3D(TextureDimension_ dimension)
	{
		switch (dimension)
		{
		case Skateboard::TextureDimension_Texture1D:
		case Skateboard::TextureDimension_Texture1D_ARRAY:
			return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			break;
		case Skateboard::TextureDimension_Texture2D:
		case Skateboard::TextureDimension_Texture2D_MS:
		case Skateboard::TextureDimension_Texture2D_ARRAY:
		case Skateboard::TextureDimension_Texture2D_MS_ARRAY:
			return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		case Skateboard::TextureDimension_Texture3D:
			return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		case Skateboard::TextureDimension_CubeMap:
			return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		default:
			break;
		}
	}

	D3D12_RAYTRACING_GEOMETRY_TYPE GeometryTypeToD3D(GeometryType_ type)
	{
		switch (type)
		{
		case GeometryType_Triangles:
			return D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		//case GeometryType_Procedural_Analytics:
		//case GeometryType_Procedural_Volumetrics:
		//case GeometryType_Procedural_SDFs:
		case GeometryType_Procedural:
			return D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
		default:
			SKTBD_LOG_ASSERT(false, "Unknown geometry type. Triangles assumed.");
			return D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		}
	}

	D3D12_RAYTRACING_GEOMETRY_FLAGS GeometryFlagsToD3D(GeometryFlags_ type)
	{
		switch (type)
		{
		case Skateboard::GeometryFlags_None:			return D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		case Skateboard::GeometryFlags_Opaque:			return D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		default:
			SKTBD_LOG_ASSERT(false, "Unknown geometry flag. none assumed.");
			return D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		}
	}

	DXGI_FORMAT ShaderDataTypeToD3D(ShaderDataType_ type)
	{
		// Can this be done in a much more elegant way?
		switch (type)
		{
		case ShaderDataType_::None:			return DXGI_FORMAT_UNKNOWN;
		case ShaderDataType_::Bool:			return DXGI_FORMAT_R8_UNORM;
		case ShaderDataType_::Int:			return DXGI_FORMAT_R32_SINT;
		case ShaderDataType_::Int2:			return DXGI_FORMAT_R32G32_SINT;
		case ShaderDataType_::Int3:			return DXGI_FORMAT_R32G32B32_SINT;
		case ShaderDataType_::Int4:			return DXGI_FORMAT_R32G32B32A32_SINT;
		case ShaderDataType_::Uint:			return DXGI_FORMAT_R32_UINT;
		case ShaderDataType_::Uint2:		return DXGI_FORMAT_R32G32_UINT;
		case ShaderDataType_::Uint3:		return DXGI_FORMAT_R32G32B32_UINT;
		case ShaderDataType_::Uint4:		return DXGI_FORMAT_R32G32B32A32_UINT;
		case ShaderDataType_::Float:		return DXGI_FORMAT_R32_FLOAT;
		case ShaderDataType_::Float2:		return DXGI_FORMAT_R32G32_FLOAT;
		case ShaderDataType_::Float3:		return DXGI_FORMAT_R32G32B32_FLOAT;
		case ShaderDataType_::Float4:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default:							SKTBD_LOG_ASSERT(false, "Impossible shader data type supplied in pipeline!"); return DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D12_SHADER_VISIBILITY ShaderVisibilityToD3D(ShaderVisibility_ v)
	{
		switch (v)
		{
		case ShaderVisibility_All:				return D3D12_SHADER_VISIBILITY_ALL;
		case ShaderVisibility_VertexShader:		return D3D12_SHADER_VISIBILITY_VERTEX;
		case ShaderVisibility_HullShader:		return D3D12_SHADER_VISIBILITY_HULL;
		case ShaderVisibility_DomainShader:		return D3D12_SHADER_VISIBILITY_DOMAIN;
		case ShaderVisibility_GeometryShader:	return D3D12_SHADER_VISIBILITY_GEOMETRY;
		case ShaderVisibility_PixelShader:		return D3D12_SHADER_VISIBILITY_PIXEL;
		default:								return D3D12_SHADER_VISIBILITY_ALL;
		}
	}

	D3D12_DESCRIPTOR_HEAP_TYPE ShaderDescriptorTableTypeToD3D(ShaderDescriptorTableType_ type)
	{
		switch (type)
		{
		case ShaderDescriptorTableType_CBV_SRV_UAV:		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case ShaderDescriptorTableType_Sampler:			return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		default:
			SKTBD_MSG_WARN("Invalid Descriptor Range type supplied. Assuming SRV.");
			return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		}
	}

	D3D12_DESCRIPTOR_RANGE_TYPE ShaderElementTypeToD3DDescriptorRange(ShaderElementType_ type)
	{
		switch (type)
		{
		case Skateboard::ShaderElementType_ConstantBufferView:			return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		case Skateboard::ShaderElementType_ShaderResourceView:			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		case Skateboard::ShaderElementType_UnorderedAccessView:		return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		default:
			SKTBD_LOG_ASSERT(false, "Invalid range. This element cannot be supplied as a descriptor range in D3D. Ensure you are using one of the above.");
			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		}
	}

	D3D12_FILTER SamplerFilterToD3D(SamplerFilter_ filter)
	{
		switch (filter)
		{
		case SamplerFilter_Min_Mag_Mip_Point:								return D3D12_FILTER_MIN_MAG_MIP_POINT;
		case SamplerFilter_Min_Mag_Point_Mip_Linear:						return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		case SamplerFilter_Min_Point_Mag_Linear_Mip_Point:					return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Min_Point_Mag_Mip_Linear:						return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case SamplerFilter_Min_Linear_Mag_Mip_Point:						return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case SamplerFilter_Min_Linear_Mag_Point_Mip_Linear:					return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case SamplerFilter_Min_Mag_Linear_Mip_Point:						return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Min_Mag_Mip_Linear:								return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		case SamplerFilter_Anisotropic:										return D3D12_FILTER_ANISOTROPIC;
		case SamplerFilter_Comaprison_Min_Mag_Mip_Point:					return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		case SamplerFilter_Comparions_Min_Mag_Point_Mip_Linear:				return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		case SamplerFilter_Comparison_Min_Point_Mag_Linear_Mip_Point:		return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Comparison_Min_Point_Mag_Mip_Linear:				return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		case SamplerFilter_Comaprison_Min_Linear_Mag_Mip_Point:				return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		case SamplerFilter_Comparison_Min_Linear_Mag_Point_Mip_Linear:		return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case SamplerFilter_Comparison_Min_Mag_Linear_Mip_Point:				return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Comparison_Min_Mag_Mip_Linear:					return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		case SamplerFilter_Comparison_Anisotropic:							return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		case SamplerFilter_Minimum_Min_Mag_Mip_Point:						return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
		case SamplerFilter_Minimum_Min_Mag_Point_Mip_Linear:				return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
		case SamplerFilter_Minimum_Min_Point_Mag_Linear_Mip_Point:			return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Minimum_Min_Point_Mag_Mip_Linear:				return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
		case SamplerFilter_Minimum_Min_Linear_Mag_Mip_Point:				return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
		case SamplerFilter_Minimum_Min_Linear_Mag_Point_Mip_Linear:			return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case SamplerFilter_Minimum_Min_Mag_Linear_Mip_Point:				return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Minimum_Min_Mag_Mip_Linear:						return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
		case SamplerFilter_Minimum_Anisotropic:								return D3D12_FILTER_MINIMUM_ANISOTROPIC;
		case SamplerFilter_Maximum_Min_Mag_Mip_Point:						return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
		case SamplerFilter_Maximum_Min_Mag_Point_Mip_Linear:				return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
		case SamplerFilter_Maximum_Min_Point_Mag_Linear_Mip_Point:			return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Maximum_Min_Point_Mag_Mip_Linear:				return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
		case SamplerFilter_Maximum_Min_Point_Mag_Mip_Point:					return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
		case SamplerFilter_Maximum_Min_Linear_Mag_Point_Mip_Linear:			return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case SamplerFilter_Maximum_Min_Mag_Linear_Mip_Point:				return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Maximum_Min_Mag_Mip_Linear:						return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
		case SamplerFilter_Maximum_Anisotropic:								return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
		default:															return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
	}

	D3D12_TEXTURE_ADDRESS_MODE SamplerModeToD3D(SamplerMode_ mode)
	{
		switch (mode)
		{
		case SamplerMode_Wrap:			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case SamplerMode_Mirror:		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case SamplerMode_Clamp:			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case SamplerMode_Border:		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case SamplerMode_Mirror_Once:	return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		default:						return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
	}

	D3D12_STATIC_BORDER_COLOR SamplerBorderColourToD3D(SamplerBorderColour_ colour)
	{
		switch (colour)
		{
		case SamplerBorderColour_TransparentBlack:	return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		case SamplerBorderColour_White:				return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		case SamplerBorderColour_Black:				return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		default:									return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		}
	}

	DXGI_FORMAT DepthStencilToSRVD3D(DataFormat_ format)
	{
		switch (format)
		{
		case DataFormat_D32_FLOAT_S8X24_UINT:	return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case DataFormat_D32_FLOAT:			return DXGI_FORMAT_R32_FLOAT;
		case DataFormat_D24_UNORM_S8_UINT:	return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DataFormat_D16_UNORM:			return DXGI_FORMAT_R16_UNORM;
		default:
			SKTBD_LOG_ASSERT(false, "Invalid depth/stencil buffer format.");
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT SkateboardBufferFormatToD3D(DataFormat_ format)
	{
		switch (format)
		{
		case DataFormat_UNKNOWN:									return DXGI_FORMAT_UNKNOWN;
		case DataFormat_R32G32B32A32_TYPELESS:					return DXGI_FORMAT_R32G32B32A32_TYPELESS;
		case DataFormat_R32G32B32A32_FLOAT:						return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case DataFormat_R32G32B32A32_UINT:						return DXGI_FORMAT_R32G32B32A32_UINT;
		case DataFormat_R32G32B32A32_SINT:						return DXGI_FORMAT_R32G32B32A32_SINT;
		case DataFormat_R32G32B32_TYPELESS:						return DXGI_FORMAT_R32G32B32_TYPELESS;
		case DataFormat_R32G32B32_FLOAT:							return DXGI_FORMAT_R32G32B32_FLOAT;
		case DataFormat_R32G32B32_UINT:							return DXGI_FORMAT_R32G32B32_UINT;
		case DataFormat_R32G32B32_SINT:							return DXGI_FORMAT_R32G32B32_SINT;
		case DataFormat_R16G16B16A16_TYPELESS:					return DXGI_FORMAT_R16G16B16A16_TYPELESS;
		case DataFormat_R16G16B16A16_FLOAT:						return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case DataFormat_R16G16B16A16_UNORM:						return DXGI_FORMAT_R16G16B16A16_UNORM;
		case DataFormat_R16G16B16A16_UINT:						return DXGI_FORMAT_R16G16B16A16_UINT;
		case DataFormat_R16G16B16A16_SNORM:						return DXGI_FORMAT_R16G16B16A16_SNORM;
		case DataFormat_R16G16B16A16_SINT:						return DXGI_FORMAT_R16G16B16A16_SINT;
		case DataFormat_R32G32_TYPELESS:							return DXGI_FORMAT_R32G32_TYPELESS;
		case DataFormat_R32G32_FLOAT:								return DXGI_FORMAT_R32G32_FLOAT;
		case DataFormat_R32G32_UINT:								return DXGI_FORMAT_R32G32_UINT;
		case DataFormat_R32G32_SINT:								return DXGI_FORMAT_R32G32_SINT;
		case DataFormat_R32G8X24_TYPELESS:						return DXGI_FORMAT_R32G8X24_TYPELESS;
		case DataFormat_D32_FLOAT_S8X24_UINT:						return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case DataFormat_R32_FLOAT_X8X24_TYPELESS:					return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case DataFormat_X32_TYPELESS_G8X24_UINT:					return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		case DataFormat_R10G10B10A2_TYPELESS:						return DXGI_FORMAT_R10G10B10A2_TYPELESS;
		case DataFormat_R10G10B10A2_UNORM:						return DXGI_FORMAT_R10G10B10A2_UNORM;
		case DataFormat_R10G10B10A2_UINT:							return DXGI_FORMAT_R10G10B10A2_UINT;
		case DataFormat_R11G11B10_FLOAT:							return DXGI_FORMAT_R11G11B10_FLOAT;
		case DataFormat_R8G8B8A8_TYPELESS:						return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		case DataFormat_R8G8B8A8_UNORM:							return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DataFormat_R8G8B8A8_UNORM_SRGB:						return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case DataFormat_R8G8B8A8_UINT:							return DXGI_FORMAT_R8G8B8A8_UINT;
		case DataFormat_R8G8B8A8_SNORM:							return DXGI_FORMAT_R8G8B8A8_SNORM;
		case DataFormat_R8G8B8A8_SINT:							return DXGI_FORMAT_R8G8B8A8_SINT;
		case DataFormat_R16G16_TYPELESS:							return DXGI_FORMAT_R16G16_TYPELESS;
		case DataFormat_R16G16_FLOAT:								return DXGI_FORMAT_R16G16_FLOAT;
		case DataFormat_R16G16_UNORM:								return DXGI_FORMAT_R16G16_UNORM;
		case DataFormat_R16G16_UINT:								return DXGI_FORMAT_R16G16_UINT;
		case DataFormat_R16G16_SNORM:								return DXGI_FORMAT_R16G16_SNORM;
		case DataFormat_R16G16_SINT:								return DXGI_FORMAT_R16G16_SINT;
		case DataFormat_R32_TYPELESS:								return DXGI_FORMAT_R32_TYPELESS;
		case DataFormat_D32_FLOAT:								return DXGI_FORMAT_D32_FLOAT;
		case DataFormat_R32_FLOAT:								return DXGI_FORMAT_R32_FLOAT;
		case DataFormat_R32_UINT:									return DXGI_FORMAT_R32_UINT;
		case DataFormat_R32_SINT:									return DXGI_FORMAT_R32_SINT;
		case DataFormat_R24G8_TYPELESS:							return DXGI_FORMAT_R24G8_TYPELESS;
		case DataFormat_D24_UNORM_S8_UINT:						return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DataFormat_R24_UNORM_X8_TYPELESS:					return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DataFormat_X24_TYPELESS_G8_UINT:						return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		case DataFormat_R8G8_TYPELESS:							return DXGI_FORMAT_R8G8_TYPELESS;
		case DataFormat_R8G8_UNORM:								return DXGI_FORMAT_R8G8_UNORM;
		case DataFormat_R8G8_UINT:								return DXGI_FORMAT_R8G8_UINT;
		case DataFormat_R8G8_SNORM:								return DXGI_FORMAT_R8G8_SNORM;
		case DataFormat_R8G8_SINT:								return DXGI_FORMAT_R8G8_SINT;
		case DataFormat_R16_TYPELESS:								return DXGI_FORMAT_R16_TYPELESS;
		case DataFormat_R16_FLOAT:								return DXGI_FORMAT_R16_FLOAT;
		case DataFormat_D16_UNORM:								return DXGI_FORMAT_D16_UNORM;
		case DataFormat_R16_UNORM:								return DXGI_FORMAT_R16_UNORM;
		case DataFormat_R16_UINT:									return DXGI_FORMAT_R16_UINT;
		case DataFormat_R16_SNORM:								return DXGI_FORMAT_R16_SNORM;
		case DataFormat_R16_SINT:									return DXGI_FORMAT_R16_SINT;
		case DataFormat_R8_TYPELESS:								return DXGI_FORMAT_R8_TYPELESS;
		case DataFormat_R8_UNORM:									return DXGI_FORMAT_R8_UNORM;
		case DataFormat_R8_UINT:									return DXGI_FORMAT_R8_UINT;
		case DataFormat_R8_SNORM:									return DXGI_FORMAT_R8_SNORM;
		case DataFormat_R8_SINT:									return DXGI_FORMAT_R8_SINT;
		case DataFormat_A8_UNORM:									return DXGI_FORMAT_A8_UNORM;
		case DataFormat_R1_UNORM:									return DXGI_FORMAT_R1_UNORM;
		case DataFormat_R9G9B9E5_SHAREDEXP:						return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		case DataFormat_R8G8_B8G8_UNORM:							return DXGI_FORMAT_R8G8_B8G8_UNORM;
		case DataFormat_G8R8_G8B8_UNORM:							return DXGI_FORMAT_G8R8_G8B8_UNORM;
		case DataFormat_BC1_TYPELESS:								return DXGI_FORMAT_BC1_TYPELESS;
		case DataFormat_BC1_UNORM:								return DXGI_FORMAT_BC1_UNORM;
		case DataFormat_BC1_UNORM_SRGB:							return DXGI_FORMAT_BC1_UNORM_SRGB;
		case DataFormat_BC2_TYPELESS:								return DXGI_FORMAT_BC2_TYPELESS;
		case DataFormat_BC2_UNORM:								return DXGI_FORMAT_BC2_UNORM;
		case DataFormat_BC2_UNORM_SRGB:							return DXGI_FORMAT_BC2_UNORM_SRGB;
		case DataFormat_BC3_TYPELESS:								return DXGI_FORMAT_BC3_TYPELESS;
		case DataFormat_BC3_UNORM:								return DXGI_FORMAT_BC3_UNORM;
		case DataFormat_BC3_UNORM_SRGB:							return DXGI_FORMAT_BC3_UNORM_SRGB;
		case DataFormat_BC4_TYPELESS:								return DXGI_FORMAT_BC4_TYPELESS;
		case DataFormat_BC4_UNORM:								return DXGI_FORMAT_BC4_UNORM;
		case DataFormat_BC4_SNORM:								return DXGI_FORMAT_BC4_SNORM;
		case DataFormat_BC5_TYPELESS:								return DXGI_FORMAT_BC5_TYPELESS;
		case DataFormat_BC5_UNORM:								return DXGI_FORMAT_BC5_UNORM;
		case DataFormat_BC5_SNORM:								return DXGI_FORMAT_BC5_SNORM;
		case DataFormat_B5G6R5_UNORM:								return DXGI_FORMAT_B5G6R5_UNORM;
		case DataFormat_B5G5R5A1_UNORM:							return DXGI_FORMAT_B5G5R5A1_UNORM;
		case DataFormat_B8G8R8A8_UNORM:							return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DataFormat_B8G8R8X8_UNORM:							return DXGI_FORMAT_B8G8R8X8_UNORM;
		case DataFormat_R10G10B10_XR_BIAS_A2_UNORM:				return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
		case DataFormat_B8G8R8A8_TYPELESS:						return DXGI_FORMAT_B8G8R8A8_TYPELESS;
		case DataFormat_B8G8R8A8_UNORM_SRGB:						return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case DataFormat_B8G8R8X8_TYPELESS:						return DXGI_FORMAT_B8G8R8X8_TYPELESS;
		case DataFormat_B8G8R8X8_UNORM_SRGB:						return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
		case DataFormat_BC6H_TYPELESS:							return DXGI_FORMAT_BC6H_TYPELESS;
		case DataFormat_BC6H_UF16:								return DXGI_FORMAT_BC6H_UF16;
		case DataFormat_BC6H_SF16:								return DXGI_FORMAT_BC6H_SF16;
		case DataFormat_BC7_TYPELESS:								return DXGI_FORMAT_BC7_TYPELESS;
		case DataFormat_BC7_UNORM:								return DXGI_FORMAT_BC7_UNORM;
		case DataFormat_BC7_UNORM_SRGB:							return DXGI_FORMAT_BC7_UNORM_SRGB;
		case DataFormat_AYUV:										return DXGI_FORMAT_AYUV;
		case DataFormat_Y410:										return DXGI_FORMAT_Y410;
		case DataFormat_Y416:										return DXGI_FORMAT_Y416;
		case DataFormat_NV12:										return DXGI_FORMAT_NV12;
		case DataFormat_P010:										return DXGI_FORMAT_P010;
		case DataFormat_P016:										return DXGI_FORMAT_P016;
		case DataFormat_420_OPAQUE:								return DXGI_FORMAT_420_OPAQUE;
		case DataFormat_YUY2:										return DXGI_FORMAT_YUY2;
		case DataFormat_Y210:										return DXGI_FORMAT_Y210;
		case DataFormat_Y216:										return DXGI_FORMAT_Y216;
		case DataFormat_NV11:										return DXGI_FORMAT_NV11;
		case DataFormat_AI44:										return DXGI_FORMAT_AI44;
		case DataFormat_IA44:										return DXGI_FORMAT_IA44;
		case DataFormat_P8:										return DXGI_FORMAT_P8;
		case DataFormat_A8P8:										return DXGI_FORMAT_A8P8;
		case DataFormat_B4G4R4A4_UNORM:							return DXGI_FORMAT_B4G4R4A4_UNORM;
		case DataFormat_P208:										return DXGI_FORMAT_P208;
		case DataFormat_V208:										return DXGI_FORMAT_V208;
		case DataFormat_V408:										return DXGI_FORMAT_V408;
		case DataFormat_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:			return DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE;
		case DataFormat_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:	return DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE;
		default:													return DXGI_FORMAT_FORCE_UINT;
		}
	}

	DataFormat_ D3DBufferFormatToSkateboard(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_UNKNOWN:									return DataFormat_UNKNOWN;
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:						return DataFormat_R32G32B32A32_TYPELESS;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:						return DataFormat_R32G32B32A32_FLOAT;
		case DXGI_FORMAT_R32G32B32A32_UINT:							return DataFormat_R32G32B32A32_UINT;
		case DXGI_FORMAT_R32G32B32A32_SINT:							return DataFormat_R32G32B32A32_SINT;
		case DXGI_FORMAT_R32G32B32_TYPELESS:						return DataFormat_R32G32B32_TYPELESS;
		case DXGI_FORMAT_R32G32B32_FLOAT:							return DataFormat_R32G32B32_FLOAT;
		case DXGI_FORMAT_R32G32B32_UINT:							return DataFormat_R32G32B32_UINT;
		case DXGI_FORMAT_R32G32B32_SINT:							return DataFormat_R32G32B32_SINT;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:						return DataFormat_R16G16B16A16_TYPELESS;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:						return DataFormat_R16G16B16A16_FLOAT;
		case DXGI_FORMAT_R16G16B16A16_UNORM:						return DataFormat_R16G16B16A16_UNORM;
		case DXGI_FORMAT_R16G16B16A16_UINT:							return DataFormat_R16G16B16A16_UINT;
		case DXGI_FORMAT_R16G16B16A16_SNORM:						return DataFormat_R16G16B16A16_SNORM;
		case DXGI_FORMAT_R16G16B16A16_SINT:							return DataFormat_R16G16B16A16_SINT;
		case DXGI_FORMAT_R32G32_TYPELESS:							return DataFormat_R32G32_TYPELESS;
		case DXGI_FORMAT_R32G32_FLOAT:								return DataFormat_R32G32_FLOAT;
		case DXGI_FORMAT_R32G32_UINT:								return DataFormat_R32G32_UINT;
		case DXGI_FORMAT_R32G32_SINT:								return DataFormat_R32G32_SINT;
		case DXGI_FORMAT_R32G8X24_TYPELESS:							return DataFormat_R32G8X24_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:						return DataFormat_D32_FLOAT_S8X24_UINT;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:					return DataFormat_R32_FLOAT_X8X24_TYPELESS;
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:					return DataFormat_X32_TYPELESS_G8X24_UINT;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:						return DataFormat_R10G10B10A2_TYPELESS;
		case DXGI_FORMAT_R10G10B10A2_UNORM:							return DataFormat_R10G10B10A2_UNORM;
		case DXGI_FORMAT_R10G10B10A2_UINT:							return DataFormat_R10G10B10A2_UINT;
		case DXGI_FORMAT_R11G11B10_FLOAT:							return DataFormat_R11G11B10_FLOAT;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:							return DataFormat_R8G8B8A8_TYPELESS;
		case DXGI_FORMAT_R8G8B8A8_UNORM:							return DataFormat_R8G8B8A8_UNORM;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:						return DataFormat_R8G8B8A8_UNORM_SRGB;
		case DXGI_FORMAT_R8G8B8A8_UINT:								return DataFormat_R8G8B8A8_UINT;
		case DXGI_FORMAT_R8G8B8A8_SNORM:							return DataFormat_R8G8B8A8_SNORM;
		case DXGI_FORMAT_R8G8B8A8_SINT:								return DataFormat_R8G8B8A8_SINT;
		case DXGI_FORMAT_R16G16_TYPELESS:							return DataFormat_R16G16_TYPELESS;
		case DXGI_FORMAT_R16G16_FLOAT:								return DataFormat_R16G16_FLOAT;
		case DXGI_FORMAT_R16G16_UNORM:								return DataFormat_R16G16_UNORM;
		case DXGI_FORMAT_R16G16_UINT:								return DataFormat_R16G16_UINT;
		case DXGI_FORMAT_R16G16_SNORM:								return DataFormat_R16G16_SNORM;
		case DXGI_FORMAT_R16G16_SINT:								return DataFormat_R16G16_SINT;
		case DXGI_FORMAT_R32_TYPELESS:								return DataFormat_R32_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT:									return DataFormat_D32_FLOAT;
		case DXGI_FORMAT_R32_FLOAT:									return DataFormat_R32_FLOAT;
		case DXGI_FORMAT_R32_UINT:									return DataFormat_R32_UINT;
		case DXGI_FORMAT_R32_SINT:									return DataFormat_R32_SINT;
		case DXGI_FORMAT_R24G8_TYPELESS:							return DataFormat_R24G8_TYPELESS;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:							return DataFormat_D24_UNORM_S8_UINT;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:						return DataFormat_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:						return DataFormat_X24_TYPELESS_G8_UINT;
		case DXGI_FORMAT_R8G8_TYPELESS:								return DataFormat_R8G8_TYPELESS;
		case DXGI_FORMAT_R8G8_UNORM:								return DataFormat_R8G8_UNORM;
		case DXGI_FORMAT_R8G8_UINT:									return DataFormat_R8G8_UINT;
		case DXGI_FORMAT_R8G8_SNORM:								return DataFormat_R8G8_SNORM;
		case DXGI_FORMAT_R8G8_SINT:									return DataFormat_R8G8_SINT;
		case DXGI_FORMAT_R16_TYPELESS:								return DataFormat_R16_TYPELESS;
		case DXGI_FORMAT_R16_FLOAT:									return DataFormat_R16_FLOAT;
		case DXGI_FORMAT_D16_UNORM:									return DataFormat_D16_UNORM;
		case DXGI_FORMAT_R16_UNORM:									return DataFormat_R16_UNORM;
		case DXGI_FORMAT_R16_UINT:									return DataFormat_R16_UINT;
		case DXGI_FORMAT_R16_SNORM:									return DataFormat_R16_SNORM;
		case DXGI_FORMAT_R16_SINT:									return DataFormat_R16_SINT;
		case DXGI_FORMAT_R8_TYPELESS:								return DataFormat_R8_TYPELESS;
		case DXGI_FORMAT_R8_UNORM:									return DataFormat_R8_UNORM;
		case DXGI_FORMAT_R8_UINT:									return DataFormat_R8_UINT;
		case DXGI_FORMAT_R8_SNORM:									return DataFormat_R8_SNORM;
		case DXGI_FORMAT_R8_SINT:									return DataFormat_R8_SINT;
		case DXGI_FORMAT_A8_UNORM:									return DataFormat_A8_UNORM;
		case DXGI_FORMAT_R1_UNORM:									return DataFormat_R1_UNORM;
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:						return DataFormat_R9G9B9E5_SHAREDEXP;
		case DXGI_FORMAT_R8G8_B8G8_UNORM:							return DataFormat_R8G8_B8G8_UNORM;
		case DXGI_FORMAT_G8R8_G8B8_UNORM:							return DataFormat_G8R8_G8B8_UNORM;
		case DXGI_FORMAT_BC1_TYPELESS:								return DataFormat_BC1_TYPELESS;
		case DXGI_FORMAT_BC1_UNORM:									return DataFormat_BC1_UNORM;
		case DXGI_FORMAT_BC1_UNORM_SRGB:							return DataFormat_BC1_UNORM_SRGB;
		case DXGI_FORMAT_BC2_TYPELESS:								return DataFormat_BC2_TYPELESS;
		case DXGI_FORMAT_BC2_UNORM:									return DataFormat_BC2_UNORM;
		case DXGI_FORMAT_BC2_UNORM_SRGB:							return DataFormat_BC2_UNORM_SRGB;
		case DXGI_FORMAT_BC3_TYPELESS:								return DataFormat_BC3_TYPELESS;
		case DXGI_FORMAT_BC3_UNORM:									return DataFormat_BC3_UNORM;
		case DXGI_FORMAT_BC3_UNORM_SRGB:							return DataFormat_BC3_UNORM_SRGB;
		case DXGI_FORMAT_BC4_TYPELESS:								return DataFormat_BC4_TYPELESS;
		case DXGI_FORMAT_BC4_UNORM:									return DataFormat_BC4_UNORM;
		case DXGI_FORMAT_BC4_SNORM:									return DataFormat_BC4_SNORM;
		case DXGI_FORMAT_BC5_TYPELESS:								return DataFormat_BC5_TYPELESS;
		case DXGI_FORMAT_BC5_UNORM:									return DataFormat_BC5_UNORM;
		case DXGI_FORMAT_BC5_SNORM:									return DataFormat_BC5_SNORM;
		case DXGI_FORMAT_B5G6R5_UNORM:								return DataFormat_B5G6R5_UNORM;
		case DXGI_FORMAT_B5G5R5A1_UNORM:							return DataFormat_B5G5R5A1_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM:							return DataFormat_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM:							return DataFormat_B8G8R8X8_UNORM;
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:				return DataFormat_R10G10B10_XR_BIAS_A2_UNORM;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:							return DataFormat_B8G8R8A8_TYPELESS;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:						return DataFormat_B8G8R8A8_UNORM_SRGB;
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:							return DataFormat_B8G8R8X8_TYPELESS;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:						return DataFormat_B8G8R8X8_UNORM_SRGB;
		case DXGI_FORMAT_BC6H_TYPELESS:								return DataFormat_BC6H_TYPELESS;
		case DXGI_FORMAT_BC6H_UF16:									return DataFormat_BC6H_UF16;
		case DXGI_FORMAT_BC6H_SF16:									return DataFormat_BC6H_SF16;
		case DXGI_FORMAT_BC7_TYPELESS:								return DataFormat_BC7_TYPELESS;
		case DXGI_FORMAT_BC7_UNORM:									return DataFormat_BC7_UNORM;
		case DXGI_FORMAT_BC7_UNORM_SRGB:							return DataFormat_BC7_UNORM_SRGB;
		case DXGI_FORMAT_AYUV:										return DataFormat_AYUV;
		case DXGI_FORMAT_Y410:										return DataFormat_Y410;
		case DXGI_FORMAT_Y416:										return DataFormat_Y416;
		case DXGI_FORMAT_NV12:										return DataFormat_NV12;
		case DXGI_FORMAT_P010:										return DataFormat_P010;
		case DXGI_FORMAT_P016:										return DataFormat_P016;
		case DXGI_FORMAT_420_OPAQUE:								return DataFormat_420_OPAQUE;
		case DXGI_FORMAT_YUY2:										return DataFormat_YUY2;
		case DXGI_FORMAT_Y210:										return DataFormat_Y210;
		case DXGI_FORMAT_Y216:										return DataFormat_Y216;
		case DXGI_FORMAT_NV11:										return DataFormat_NV11;
		case DXGI_FORMAT_AI44:										return DataFormat_AI44;
		case DXGI_FORMAT_IA44:										return DataFormat_IA44;
		case DXGI_FORMAT_P8:										return DataFormat_P8;
		case DXGI_FORMAT_A8P8:										return DataFormat_A8P8;
		case DXGI_FORMAT_B4G4R4A4_UNORM:							return DataFormat_B4G4R4A4_UNORM;
		case DXGI_FORMAT_P208:										return DataFormat_P208;
		case DXGI_FORMAT_V208:										return DataFormat_V208;
		case DXGI_FORMAT_V408:										return DataFormat_V408;
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:			return DataFormat_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE;
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:	return DataFormat_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE;
		default:													return DataFormat_FORCE_UINT;
		}
	}

	//D3D12_RESOURCE_STATES ResourceTypeToStateD3D(GPUResourceType_ type)
	//{
	//	switch (type)
	//	{
	//	case GPUResourceType_DefaultBuffer:
	//		return D3D12_RESOURCE_STATE_GENERIC_READ;
	//	case GPUResourceType_Texture2D:
	//	case GPUResourceType_Texture3D:
	//		return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
	//	case GPUResourceType_UnorderedAccessBuffer:
	//		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	//	default:
	//		SKTBD_LOG_ASSERT(false, "This resource type cannot or should not be switched to a state. Illegal state.");
	//		return D3D12_RESOURCE_STATE_COMMON;
	//	}
	//}

}