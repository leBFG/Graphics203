#include "AGCTypes.h"

#include "AGC/Graphics/AGCF.h"
using namespace sce::Agc::Gnmp::Extras;

#define SKTBD_LOG_COMPONENT "DATA TYPE MAPPINGS PLAYSTATION"
#include "Skateboard/Log.h"

namespace Skateboard
{

	sce::Agc::Core::Texture::Type SkateboardTextureDimensionToAGC(TextureDimension_ dimension)
	{
		switch (dimension)
		{
		case TextureDimension_Texture1D:					return sce::Agc::Core::Texture::Type::k1d;
		case TextureDimension_Texture2D:					return sce::Agc::Core::Texture::Type::k2d;
		case TextureDimension_Texture3D:					return sce::Agc::Core::Texture::Type::k3d;
		case TextureDimension_CubeMap:						return sce::Agc::Core::Texture::Type::kCubemap;
		case TextureDimension_CubeMap_ARRAY:				return sce::Agc::Core::Texture::Type::kCubemap;
		case TextureDimension_Texture1D_ARRAY:				return sce::Agc::Core::Texture::Type::k1dArray;
		case TextureDimension_Texture2D_ARRAY:				return sce::Agc::Core::Texture::Type::k2dArray;
		case TextureDimension_Texture2D_MS:					return sce::Agc::Core::Texture::Type::k2dMsaa;
		case TextureDimension_Texture2D_MS_ARRAY:			return sce::Agc::Core::Texture::Type::k2dArrayMsaa;
		}
	}

	TextureDimension_ AGCTextureDimensionToSkateboard(sce::Agc::Core::Texture::Type type)
	{
		switch (type)
		{
		case sce::Agc::Core::Texture::Type::k1d:            return  TextureDimension_Texture1D;
		case sce::Agc::Core::Texture::Type::k2d:			return	TextureDimension_Texture2D;
		case sce::Agc::Core::Texture::Type::k3d:			return	TextureDimension_Texture3D;
		case sce::Agc::Core::Texture::Type::kCubemap:		return	TextureDimension_CubeMap;
		case sce::Agc::Core::Texture::Type::k1dArray:		return	TextureDimension_Texture1D_ARRAY;
		case sce::Agc::Core::Texture::Type::k2dArray:		return	TextureDimension_Texture2D_ARRAY;
		case sce::Agc::Core::Texture::Type::k2dMsaa:		return	TextureDimension_Texture2D_MS;
		case sce::Agc::Core::Texture::Type::k2dArrayMsaa:	return	TextureDimension_Texture2D_MS_ARRAY;
		}
	}


	sce::Agc::Core::DataFormat ShaderDataTypeToAGC(ShaderDataType_ type)
	{
		return sce::Agc::Core::DataFormat();
	}

	sce::Agc::Core::DataFormat DepthStencilToSRVAGC(DataFormat_ format)
	{
		return sce::Agc::Core::DataFormat();
	}

	//DataFormat_ AGCToBufferFormat(sce::Agc::Core::DataFormat type)
	//{
	//	switch (type.m_format)
	//	{
	//	case kDataFormatInvalid:					return DataFormat_UNKNOWN;
	//		//TypedFormat::kInvalid:				//return DataFormat_R32G32B32A32_TYPELESS;						
	//	case kDataFormatR32G32B32A32Float:			return DataFormat_R32G32B32A32_FLOAT;
	//	case kDataFormatR32G32B32A32Uint:			return DataFormat_R32G32B32A32_UINT;
	//	case kDataFormatR32G32B32A32Sint:			return DataFormat_R32G32B32A32_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R32G32B32_TYPELESS;							
	//	case kDataFormatR32G32B32Float:				return DataFormat_R32G32B32_FLOAT;
	//	case kDataFormatR32G32B32Uint:				return DataFormat_R32G32B32_UINT;
	//	case kDataFormatR32G32B32Sint:				return DataFormat_R32G32B32_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R16G16B16A16_TYPELESS;						
	//	case kDataFormatR16G16B16A16Float:			return DataFormat_R16G16B16A16_FLOAT;
	//	case kDataFormatR16G16B16A16Unorm:			return DataFormat_R16G16B16A16_UNORM;
	//	case kDataFormatR16G16B16A16Uint:			return DataFormat_R16G16B16A16_UINT;
	//	case kDataFormatR16G16B16A16Snorm:			return DataFormat_R16G16B16A16_SNORM;
	//	case kDataFormatR16G16B16A16Sint:			return DataFormat_R16G16B16A16_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R32G32_TYPELESS;							
	//	case kDataFormatR32G32Float:				return DataFormat_R32G32_FLOAT;
	//	case kDataFormatR32G32Uint:					return DataFormat_R32G32_UINT;
	//	case kDataFormatR32G32Sint:					return DataFormat_R32G32_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R32G8X24_TYPELESS;							
	//		//TypedFormat::kInvalid:				//return DataFormat_D32_FLOAT_S8X24_UINT;						
	//		//TypedFormat::kInvalid:				//return DataFormat_R32_FLOAT_X8X24_TYPELESS;					
	//		//TypedFormat::kInvalid:				//return DataFormat_X32_TYPELESS_G8X24_UINT;					
	//		//TypedFormat::kInvalid:				//return DataFormat_R10G10B10A2_TYPELESS;						
	//	case kDataFormatR10G10B10A2Unorm:			return DataFormat_R10G10B10A2_UNORM;
	//	case kDataFormatR10G10B10A2Uint:			return DataFormat_R10G10B10A2_UINT;
	//	case kDataFormatR11G11B10Float:				return DataFormat_R11G11B10_FLOAT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R8G8B8A8_TYPELESS;							
	//	case kDataFormatR8G8B8A8Unorm:				return DataFormat_R8G8B8A8_UNORM;
	//	case kDataFormatR8G8B8A8UnormSrgb:			return DataFormat_R8G8B8A8_UNORM_SRGB;
	//	case kDataFormatR8G8B8A8Uint:				return DataFormat_R8G8B8A8_UINT;
	//	case kDataFormatR8G8B8A8Snorm:				return DataFormat_R8G8B8A8_SNORM;
	//	case kDataFormatR8G8B8A8Sint:				return DataFormat_R8G8B8A8_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R16G16_TYPELESS;							
	//	case kDataFormatR16G16Float:				return DataFormat_R16G16_FLOAT;
	//	case kDataFormatR16G16Unorm:				return DataFormat_R16G16_UNORM;
	//	case kDataFormatR16G16Uint:					return DataFormat_R16G16_UINT;
	//	case kDataFormatR16G16Snorm:				return DataFormat_R16G16_SNORM;
	//	case kDataFormatR16G16Sint:					return DataFormat_R16G16_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R32_TYPELESS;								
	//		//TypedFormat::kInvalid:				//return DataFormat_D32_FLOAT;									
	//	case kDataFormatR32Float:					return DataFormat_R32_FLOAT;
	//	case kDataFormatR32Uint:					return DataFormat_R32_UINT;
	//	case kDataFormatR32Sint:					return DataFormat_R32_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R24G8_TYPELESS;								
	//		//TypedFormat::kInvalid:				//return DataFormat_D24_UNORM_S8_UINT;							
	//		//TypedFormat::kInvalid:				//return DataFormat_R24_UNORM_X8_TYPELESS;						
	//		//TypedFormat::kInvalid:				//return DataFormat_X24_TYPELESS_G8_UINT;						
	//		//TypedFormat::kInvalid:				//return DataFormat_R8G8_TYPELESS;								
	//	case kDataFormatR8G8Unorm:					return DataFormat_R8G8_UNORM;
	//	case kDataFormatR8G8Uint:					return DataFormat_R8G8_UINT;
	//	case kDataFormatR8G8Snorm:					return DataFormat_R8G8_SNORM;
	//	case kDataFormatR8G8Sint:					return DataFormat_R8G8_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R16_TYPELESS;								
	//	case kDataFormatR16Float:					return DataFormat_R16_FLOAT;
	//		//TypedFormat::kInvalid:				//return DataFormat_D16_UNORM;									
	//	case kDataFormatR16Unorm:					return DataFormat_R16_UNORM;
	//	case kDataFormatR16Uint:					return DataFormat_R16_UINT;
	//	case kDataFormatR16Snorm:					return DataFormat_R16_SNORM;
	//	case kDataFormatR16Sint:					return DataFormat_R16_SINT;
	//		//TypedFormat::kInvalid:				//return DataFormat_R8_TYPELESS;								
	//	case kDataFormatR8Unorm:					return DataFormat_R8_UNORM;
	//	case kDataFormatR8Uint:						return DataFormat_R8_UINT;
	//	case kDataFormatR8Snorm:					return DataFormat_R8_SNORM;
	//	case kDataFormatR8Sint:						return DataFormat_R8_SINT;
	//	case kDataFormatA8Unorm:					return DataFormat_A8_UNORM;
	//	case kDataFormatR1Unorm:					return DataFormat_R1_UNORM;
	//	case kDataFormatR9G9B9E5Float:				return DataFormat_R9G9B9E5_SHAREDEXP;
	//	case kDataFormatR8G8B8G8Unorm:				return DataFormat_R8G8_B8G8_UNORM;
	//	case kDataFormatG8R8G8B8Unorm:				return DataFormat_G8R8_G8B8_UNORM;
	//		//TypedFormat::kInvalid:				//return DataFormat_BC1_TYPELESS;								
	//	case kDataFormatBc1Unorm:					return DataFormat_BC1_UNORM;
	//	case kDataFormatBc1UnormSrgb:				return DataFormat_BC1_UNORM_SRGB;
	//		//TypedFormat::kInvalid:				//return DataFormat_BC2_TYPELESS;								
	//	case kDataFormatBc2Unorm:					return DataFormat_BC2_UNORM;
	//	case kDataFormatBc2UnormSrgb:				return DataFormat_BC2_UNORM_SRGB;
	//		//TypedFormat::kInvalid:				//return DataFormat_BC3_TYPELESS;								
	//	case kDataFormatBc3Unorm:					return DataFormat_BC3_UNORM;
	//	case kDataFormatBc3UnormSrgb:				return DataFormat_BC3_UNORM_SRGB;
	//		//TypedFormat::kInvalid:				//return DataFormat_BC4_TYPELESS;								
	//	case kDataFormatBc4Unorm:					return DataFormat_BC4_UNORM;
	//	case kDataFormatBc4Snorm:					return DataFormat_BC4_SNORM;
	//		//TypedFormat::kInvalid:				//return DataFormat_BC5_TYPELESS;								
	//	case kDataFormatBc5Unorm:					return DataFormat_BC5_UNORM;
	//	case kDataFormatBc5Snorm:					return DataFormat_BC5_SNORM;
	//	case kDataFormatB5G6R5Unorm:				return DataFormat_B5G6R5_UNORM;
	//	case kDataFormatB5G5R5A1Unorm:				return DataFormat_B5G5R5A1_UNORM;
	//	case kDataFormatB8G8R8A8Unorm:				return DataFormat_B8G8R8A8_UNORM;
	//	case kDataFormatB8G8R8X8Unorm:				return DataFormat_B8G8R8X8_UNORM;
	//		//kDataFormatR10G10B10A2Unorm:			//return DataFormat_R10G10B10_XR_BIAS_A2_UNORM;					
	//		//TypedFormat::kInvalid:				//return DataFormat_B8G8R8A8_TYPELESS;							
	//	case kDataFormatB8G8R8A8UnormSrgb:			return DataFormat_B8G8R8A8_UNORM_SRGB;
	//		//TypedFormat::kInvalid:				//return DataFormat_B8G8R8X8_TYPELESS;							
	//	case kDataFormatB8G8R8X8UnormSrgb:			return DataFormat_B8G8R8X8_UNORM_SRGB;
	//		//TypedFormat::kInvalid:				//return DataFormat_BC6H_TYPELESS;								
	//	case kDataFormatBc6Uf16:					return DataFormat_BC6H_UF16;
	//	case kDataFormatBc6Sf16:					return DataFormat_BC6H_SF16;
	//		//TypedFormat::kInvalid:				//return DataFormat_BC7_TYPELESS;								
	//	case kDataFormatBc7Unorm:					return DataFormat_BC7_UNORM;
	//	case kDataFormatBc7UnormSrgb:				return DataFormat_BC7_UNORM_SRGB;
	//		//TypedFormat::kInvalid:				//return DataFormat_AYUV ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_Y410 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_Y416 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_NV12 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_P010 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_P016 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_420_OPAQUE ;								
	//		//TypedFormat::kInvalid:				//return DataFormat_YUY2 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_Y210 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_Y216 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_NV11 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_AI44 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_IA44 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_P8 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_A8P8 ;										
	//	case kDataFormatB4G4R4A4Unorm:				return DataFormat_B4G4R4A4_UNORM;

	//		//TypedFormat::kInvalid:				//return DataFormat_P208 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_V208 ;										
	//		//TypedFormat::kInvalid:				//return DataFormat_V408 ;										

	//		//TypedFormat::kInvalid:				//return DataFormat_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE ;			
	//		//TypedFormat::kInvalid:				//return DataFormat_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE ;	

	//	case kDataFormatR32Uint:					return DataFormat_FORCE_UINT;
	//	//defaukDataFormatInvalid:
	//	}
	//}

	sce::Agc::Core::DataFormat SkateboardDataFormatToAGC(DataFormat_ format)
	{
		switch (format)
		{
		case DataFormat_UNKNOWN:										return kDataFormatInvalid;
			//case DataFormat_R32G32B32A32_TYPELESS:							return TypedFormat::kInvalid;
		case DataFormat_R32G32B32A32_FLOAT:								return kDataFormatR32G32B32A32Float;
		case DataFormat_R32G32B32A32_UINT:								return kDataFormatR32G32B32A32Uint;
		case DataFormat_R32G32B32A32_SINT:								return kDataFormatR32G32B32A32Sint;
			//case DataFormat_R32G32B32_TYPELESS:								return TypedFormat::kInvalid;
		case DataFormat_R32G32B32_FLOAT:									return kDataFormatR32G32B32Float;
		case DataFormat_R32G32B32_UINT:									return kDataFormatR32G32B32Uint;
		case DataFormat_R32G32B32_SINT:									return kDataFormatR32G32B32Sint;
			//case DataFormat_R16G16B16A16_TYPELESS:							return TypedFormat::kInvalid;
		case DataFormat_R16G16B16A16_FLOAT:								return kDataFormatR16G16B16A16Float;
		case DataFormat_R16G16B16A16_UNORM:								return kDataFormatR16G16B16A16Unorm;
		case DataFormat_R16G16B16A16_UINT:								return kDataFormatR16G16B16A16Uint;
		case DataFormat_R16G16B16A16_SNORM:								return kDataFormatR16G16B16A16Snorm;
		case DataFormat_R16G16B16A16_SINT:								return kDataFormatR16G16B16A16Sint;
			//case DataFormat_R32G32_TYPELESS:									return TypedFormat::kInvalid;
		case DataFormat_R32G32_FLOAT:										return kDataFormatR32G32Float;
		case DataFormat_R32G32_UINT:										return kDataFormatR32G32Uint;
		case DataFormat_R32G32_SINT:										return kDataFormatR32G32Sint;
			//case DataFormat_R32G8X24_TYPELESS:								return TypedFormat::kInvalid;
			//case DataFormat_D32_FLOAT_S8X24_UINT:								return TypedFormat::kInvalid;
			//case DataFormat_R32_FLOAT_X8X24_TYPELESS:							return TypedFormat::kInvalid;
			//case DataFormat_X32_TYPELESS_G8X24_UINT:							return TypedFormat::kInvalid;
			//case DataFormat_R10G10B10A2_TYPELESS:								return TypedFormat::kInvalid;
		case DataFormat_R10G10B10A2_UNORM:								return kDataFormatR10G10B10A2Unorm;
		case DataFormat_R10G10B10A2_UINT:									return kDataFormatR10G10B10A2Uint;
		case DataFormat_R11G11B10_FLOAT:									return kDataFormatR11G11B10Float;
			//case DataFormat_R8G8B8A8_TYPELESS:								return TypedFormat::kInvalid;
		case DataFormat_R8G8B8A8_UNORM:									return kDataFormatR8G8B8A8Unorm;
		case DataFormat_R8G8B8A8_UNORM_SRGB:								return kDataFormatR8G8B8A8UnormSrgb;
		case DataFormat_R8G8B8A8_UINT:									return kDataFormatR8G8B8A8Uint;
		case DataFormat_R8G8B8A8_SNORM:									return kDataFormatR8G8B8A8Snorm;
		case DataFormat_R8G8B8A8_SINT:									return kDataFormatR8G8B8A8Sint;
			//case DataFormat_R16G16_TYPELESS:									return TypedFormat::kInvalid;
		case DataFormat_R16G16_FLOAT:										return kDataFormatR16G16Float;
		case DataFormat_R16G16_UNORM:										return kDataFormatR16G16Unorm;
		case DataFormat_R16G16_UINT:										return kDataFormatR16G16Uint;
		case DataFormat_R16G16_SNORM:										return kDataFormatR16G16Snorm;
		case DataFormat_R16G16_SINT:										return kDataFormatR16G16Sint;
			//case DataFormat_R32_TYPELESS:										return TypedFormat::kInvalid;
			//case DataFormat_D32_FLOAT:										return TypedFormat::kInvalid;
		case DataFormat_R32_FLOAT:										return kDataFormatR32Float;
		case DataFormat_R32_UINT:											return kDataFormatR32Uint;
		case DataFormat_R32_SINT:											return kDataFormatR32Sint;
			//case DataFormat_R24G8_TYPELESS:									return TypedFormat::kInvalid;
			//case DataFormat_D24_UNORM_S8_UINT:								return TypedFormat::kInvalid;
			//case DataFormat_R24_UNORM_X8_TYPELESS:							return TypedFormat::kInvalid;
			//case DataFormat_X24_TYPELESS_G8_UINT:								return TypedFormat::kInvalid;
			//case DataFormat_R8G8_TYPELESS:									return TypedFormat::kInvalid;
		case DataFormat_R8G8_UNORM:										return kDataFormatR8G8Unorm;
		case DataFormat_R8G8_UINT:										return kDataFormatR8G8Uint;
		case DataFormat_R8G8_SNORM:										return kDataFormatR8G8Snorm;
		case DataFormat_R8G8_SINT:										return kDataFormatR8G8Sint;
			//case DataFormat_R16_TYPELESS:										return TypedFormat::kInvalid;
		case DataFormat_R16_FLOAT:										return kDataFormatR16Float;
			//case DataFormat_D16_UNORM:										return TypedFormat::kInvalid;
		case DataFormat_R16_UNORM:										return kDataFormatR16Unorm;
		case DataFormat_R16_UINT:											return kDataFormatR16Uint;
		case DataFormat_R16_SNORM:										return kDataFormatR16Snorm;
		case DataFormat_R16_SINT:											return kDataFormatR16Sint;
			//case DataFormat_R8_TYPELESS:										return TypedFormat::kInvalid;
		case DataFormat_R8_UNORM:											return kDataFormatR8Unorm;
		case DataFormat_R8_UINT:											return kDataFormatR8Uint;
		case DataFormat_R8_SNORM:											return kDataFormatR8Snorm;
		case DataFormat_R8_SINT:											return kDataFormatR8Sint;
		case DataFormat_A8_UNORM:											return kDataFormatA8Unorm;
		case DataFormat_R1_UNORM:											return kDataFormatR1Unorm;
		case DataFormat_R9G9B9E5_SHAREDEXP:								return kDataFormatR9G9B9E5Float;
		case DataFormat_R8G8_B8G8_UNORM:									return kDataFormatR8G8B8G8Unorm;
		case DataFormat_G8R8_G8B8_UNORM:									return kDataFormatG8R8G8B8Unorm;
			//case DataFormat_BC1_TYPELESS:										return TypedFormat::kInvalid;
		case DataFormat_BC1_UNORM:										return kDataFormatBc1Unorm;
		case DataFormat_BC1_UNORM_SRGB:									return kDataFormatBc1UnormSrgb;
			//case DataFormat_BC2_TYPELESS:										return TypedFormat::kInvalid;
		case DataFormat_BC2_UNORM:										return kDataFormatBc2Unorm;
		case DataFormat_BC2_UNORM_SRGB:									return kDataFormatBc2UnormSrgb;
			//case DataFormat_BC3_TYPELESS:										return TypedFormat::kInvalid;
		case DataFormat_BC3_UNORM:										return kDataFormatBc3Unorm;
		case DataFormat_BC3_UNORM_SRGB:									return kDataFormatBc3UnormSrgb;
			//case DataFormat_BC4_TYPELESS:										return TypedFormat::kInvalid;
		case DataFormat_BC4_UNORM:										return kDataFormatBc4Unorm;
		case DataFormat_BC4_SNORM:										return kDataFormatBc4Snorm;
			//case DataFormat_BC5_TYPELESS:										return TypedFormat::kInvalid;
		case DataFormat_BC5_UNORM:										return kDataFormatBc5Unorm;
		case DataFormat_BC5_SNORM:										return kDataFormatBc5Snorm;
		case DataFormat_B5G6R5_UNORM:										return kDataFormatB5G6R5Unorm;
		case DataFormat_B5G5R5A1_UNORM:									return kDataFormatB5G5R5A1Unorm;
		case DataFormat_B8G8R8A8_UNORM:									return kDataFormatB8G8R8A8Unorm;
		case DataFormat_B8G8R8X8_UNORM:									return kDataFormatB8G8R8X8Unorm;
			//case DataFormat_R10G10B10_XR_BIAS_A2_UNORM:						return kDataFormatR10G10B10A2Unorm;
			//case DataFormat_B8G8R8A8_TYPELESS:								return TypedFormat::kInvalid;
		case DataFormat_B8G8R8A8_UNORM_SRGB:								return kDataFormatB8G8R8A8UnormSrgb;
			//case DataFormat_B8G8R8X8_TYPELESS:								return TypedFormat::kInvalid;
		case DataFormat_B8G8R8X8_UNORM_SRGB:								return kDataFormatB8G8R8X8UnormSrgb;
			//case DataFormat_BC6H_TYPELESS:									return TypedFormat::kInvalid;
		case DataFormat_BC6H_UF16:										return kDataFormatBc6Uf16;
		case DataFormat_BC6H_SF16:										return kDataFormatBc6Sf16;
			//case DataFormat_BC7_TYPELESS:										return TypedFormat::kInvalid;
		case DataFormat_BC7_UNORM:										return kDataFormatBc7Unorm;
		case DataFormat_BC7_UNORM_SRGB:									return kDataFormatBc7UnormSrgb;
			//case DataFormat_AYUV :											return TypedFormat::kInvalid;
			//case DataFormat_Y410 :											return TypedFormat::kInvalid;
			//case DataFormat_Y416 :											return TypedFormat::kInvalid;
			//case DataFormat_NV12 :											return TypedFormat::kInvalid;
			//case DataFormat_P010 :											return TypedFormat::kInvalid;
			//case DataFormat_P016 :											return TypedFormat::kInvalid;
			//case DataFormat_420_OPAQUE :										return TypedFormat::kInvalid;
			//case DataFormat_YUY2 :											return TypedFormat::kInvalid;
			//case DataFormat_Y210 :											return TypedFormat::kInvalid;
			//case DataFormat_Y216 :											return TypedFormat::kInvalid;
			//case DataFormat_NV11 :											return TypedFormat::kInvalid;
			//case DataFormat_AI44 :											return TypedFormat::kInvalid;
			//case DataFormat_IA44 :											return TypedFormat::kInvalid;
			//case DataFormat_P8 :												return TypedFormat::kInvalid;
			//case DataFormat_A8P8 :											return TypedFormat::kInvalid;
		case DataFormat_B4G4R4A4_UNORM:									return kDataFormatB4G4R4A4Unorm;

			//case DataFormat_P208 :											return TypedFormat::kInvalid;
			//case DataFormat_V208 :											return TypedFormat::kInvalid;
			//case DataFormat_V408 :											return TypedFormat::kInvalid;

			//case DataFormat_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE :					return TypedFormat::kInvalid;
			//case DataFormat_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE :			return TypedFormat::kInvalid;

		case DataFormat_FORCE_UINT:										return kDataFormatR32Uint;
		default:															return kDataFormatInvalid;
		}
	}

	sce::Agc::CxRenderTarget::Dimension SkateboardTextureDimensionToAGCRenderTarget(TextureDimension_ type)
	{
		switch (type)
		{
		case Skateboard::TextureDimension_Texture1D:
			return sce::Agc::CxRenderTarget::Dimension::k1d;
			break;
		case Skateboard::TextureDimension_Texture2D:
		case Skateboard::TextureDimension_Texture2D_MS:
			return sce::Agc::CxRenderTarget::Dimension::k2d;
			break;
		case Skateboard::TextureDimension_Texture3D:
			return sce::Agc::CxRenderTarget::Dimension::k3d;
			break;
		default:
			return sce::Agc::CxRenderTarget::Dimension::k2d;
			break;
		}
	}

	DataFormat_ AGCBufferFormatToSkateboard(sce::Agc::Core::DataFormat format)
	{
		return DataFormat_UNKNOWN;
	}

	SceShaderResourceClass ShaderElementTypeToAGCResourceClass(ShaderElementType_ type)
	{
		switch (type)
		{
		case ShaderElementType_RootConstant:				return SceShaderCb;
		case ShaderElementType_ConstantBufferView:			return SceShaderCb;
		case ShaderElementType_ShaderResourceView:			return SceShaderSrv;
		case ShaderElementType_UnorderedAccessView:			return SceShaderUav;
		case ShaderElementType_DescriptorTable:				return SceShaderSrt;
		default:											return SceShaderUnknownResource;
		}
	}

	ShaderElementType_ AGCResourceClassToShaderElementType(SceShaderResourceClass type)
	{
		switch (type)
		{
		case SceShaderCb:									return ShaderElementType_ConstantBufferView;
		case SceShaderSrv:									return ShaderElementType_ShaderResourceView;
		case SceShaderUav:									return ShaderElementType_UnorderedAccessView;
		case SceShaderSrt:									return ShaderElementType_DescriptorTable;
		case SceShaderSamplerState:							return ShaderElementType_Sampler;
		default:											return ShaderElementType_Unknown;
		}
	}

	sce::Agc::Core::Sampler::WrapMode SamplerModeToAGC(SamplerMode_ mode)
	{
		switch (mode)
		{
		case Skateboard::SamplerMode_Wrap:			return sce::Agc::Core::Sampler::WrapMode::kWrap;				break;
		case Skateboard::SamplerMode_Mirror:		return sce::Agc::Core::Sampler::WrapMode::kMirror;				break;
		case Skateboard::SamplerMode_Clamp:			return sce::Agc::Core::Sampler::WrapMode::kClampLastTexel;		break;
		case Skateboard::SamplerMode_Border:		return sce::Agc::Core::Sampler::WrapMode::kClampBorder;			break;
		case Skateboard::SamplerMode_Mirror_Once:	return sce::Agc::Core::Sampler::WrapMode::kMirrorOnceLastTexel;	break;
		default:									return sce::Agc::Core::Sampler::WrapMode::kWrap;				break;
		}
	}

	sce::Agc::Core::Sampler::DepthCompare SamplerComparisonFunctionToAGC(SamplerComparisonFunction_ function)
	{
		switch (function)
		{
		case Skateboard::SamplerComparisonFunction_Never:				return sce::Agc::Core::Sampler::DepthCompare::kNever;
		case Skateboard::SamplerComparisonFunction_Less:				return sce::Agc::Core::Sampler::DepthCompare::kLess;
		case Skateboard::SamplerComparisonFunction_Equal:				return sce::Agc::Core::Sampler::DepthCompare::kEqual;
		case Skateboard::SamplerComparisonFunction_Less_Equal:			return sce::Agc::Core::Sampler::DepthCompare::kLessEqual;
		case Skateboard::SamplerComparisonFunction_Greater:				return sce::Agc::Core::Sampler::DepthCompare::kGreater;
		case Skateboard::SamplerComparisonFunction_Not_Equal:			return sce::Agc::Core::Sampler::DepthCompare::kNotEqual;
		case Skateboard::SamplerComparisonFunction_Greater_Equal:		return sce::Agc::Core::Sampler::DepthCompare::kGreaterEqual;
		case Skateboard::SamplerComparisonFunction_Always:				return sce::Agc::Core::Sampler::DepthCompare::kAlways;
		default:														return sce::Agc::Core::Sampler::DepthCompare::kNever;
		}
	}

	sce::Agc::Core::Sampler::BorderColor SamplerBorderColourToAGC(SamplerBorderColour_ colour)
	{
		switch (colour)
		{
		case Skateboard::SamplerBorderColour_TransparentBlack:	return sce::Agc::Core::Sampler::BorderColor::kTransBlack;
		case Skateboard::SamplerBorderColour_White:				return sce::Agc::Core::Sampler::BorderColor::kOpaqueWhite;
		case Skateboard::SamplerBorderColour_Black:				return sce::Agc::Core::Sampler::BorderColor::kOpaqueBlack;
		default:												return sce::Agc::Core::Sampler::BorderColor::kOpaqueBlack;
		}
	}

	void SetFilterMode(sce::Agc::Core::Sampler& inout_sampler, SamplerFilter_ filter, SamplerComparisonFunction_ comparisonFunc)
	{
		using namespace sce::Agc::Core;

		switch (filter)
		{
		case Skateboard::SamplerFilter_Min_Mag_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kPoint);
			break;
		case Skateboard::SamplerFilter_Min_Mag_Point_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kLinear);
			break;
		case Skateboard::SamplerFilter_Min_Point_Mag_Linear_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kPoint);
			break;
		case Skateboard::SamplerFilter_Min_Point_Mag_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kLinear);
			break;
		case Skateboard::SamplerFilter_Min_Linear_Mag_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kPoint);
			break;
		case Skateboard::SamplerFilter_Min_Linear_Mag_Point_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear);
			break;
		case Skateboard::SamplerFilter_Min_Mag_Linear_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kPoint);
			break;
		case Skateboard::SamplerFilter_Min_Mag_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear);
			break;
		case Skateboard::SamplerFilter_Anisotropic:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kAnisoBilinear, Sampler::FilterMode::kAnisoBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear);
			break;
		case Skateboard::SamplerFilter_Comaprison_Min_Mag_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Comparions_Min_Mag_Point_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Comparison_Min_Point_Mag_Linear_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Comparison_Min_Point_Mag_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Comaprison_Min_Linear_Mag_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Comparison_Min_Linear_Mag_Point_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Comparison_Min_Mag_Linear_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Comparison_Min_Mag_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Comparison_Anisotropic:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kAnisoBilinear, Sampler::FilterMode::kAnisoBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setDepthCompareFunction(SamplerComparisonFunctionToAGC(comparisonFunc));
			break;
		case Skateboard::SamplerFilter_Minimum_Min_Mag_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Minimum_Min_Mag_Point_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Minimum_Min_Point_Mag_Linear_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Minimum_Min_Point_Mag_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Minimum_Min_Linear_Mag_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Minimum_Min_Linear_Mag_Point_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Minimum_Min_Mag_Linear_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Minimum_Min_Mag_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Minimum_Anisotropic:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kAnisoBilinear, Sampler::FilterMode::kAnisoBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMin);
			break;
		case Skateboard::SamplerFilter_Maximum_Min_Mag_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		case Skateboard::SamplerFilter_Maximum_Min_Mag_Point_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		case Skateboard::SamplerFilter_Maximum_Min_Point_Mag_Linear_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		case Skateboard::SamplerFilter_Maximum_Min_Point_Mag_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kPoint).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		case Skateboard::SamplerFilter_Maximum_Min_Point_Mag_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		case Skateboard::SamplerFilter_Maximum_Min_Linear_Mag_Point_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kPoint, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		case Skateboard::SamplerFilter_Maximum_Min_Mag_Linear_Mip_Point:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kPoint)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		case Skateboard::SamplerFilter_Maximum_Min_Mag_Mip_Linear:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kBilinear, Sampler::FilterMode::kBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		case Skateboard::SamplerFilter_Maximum_Anisotropic:
			inout_sampler.setXyFilterMode(Sampler::FilterMode::kAnisoBilinear, Sampler::FilterMode::kAnisoBilinear).setMipFilterMode(Sampler::MipFilterMode::kLinear)
				.setFilterReductionMode(Sampler::FilterReductionMode::kMax);
			break;
		default:
			SKTBD_LOG_ASSERT(false, "Invalid sampler filtering.");
			break;
		}
	}
}