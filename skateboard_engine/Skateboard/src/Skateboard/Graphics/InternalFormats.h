 #pragma once
#include "sktbdpch.h"

namespace Skateboard
{

	/////////////////////////////////////////////////////////////////
	// GEOMTRY FORMATS
	/////////////////////////////////////////////////////////////////

	enum GeometryType_ : uint8_t	// Uint32 would overflow in raytracing -> only the 8 bottoms bits are used
	{
		GeometryType_None_NoTrace = 0,
		GeometryType_Triangles = (1 << 0),
		GeometryType_Procedural,
	//	GeometryType_Procedural_Analytics = (1 << 1),
	//	GeometryType_Procedural_Volumetrics = (1 << 2),
	//	GeometryType_Procedural_SDFs = (1 << 3),

	//	GeometryType_Procedural_Mask = GeometryType_Procedural_Analytics & GeometryType_Procedural_Volumetrics & GeometryType_Procedural_SDFs
	};
	ENUM_FLAG_OPERATORS(GeometryType_);

	enum GeometryFlags_
	{
		GeometryFlags_None = 0,
		GeometryFlags_Opaque
	};
	ENUM_FLAG_OPERATORS(GeometryFlags_)

	/////////////////////////////////////////////////////////////////
	// PIPELINE FORMATS
	/////////////////////////////////////////////////////////////////

	enum PrimitiveTopologyType_
	{
		PrimitiveTopologyType_none = 0,
		PrimitiveTopologyType_Point = 1,
		PrimitiveTopologyType_Line = 2,
		PrimitiveTopologyType_Triangle = 3,
		PrimitiveTopologyType_Patch = 4
	};

	enum RaytracingHitGroupType_
	{
		RaytracingHitGroupType_Triangles = 0,
		RaytracingHitGroupType_Procedural,
		RaytracingHitGroupType_Count
	};

	struct RaytracingASSizeInfo
	{
		size_t StorageSize;
		size_t ScratchSizeBuild;
		size_t ScratchSizeUpdate;
	};

	/////////////////////////////////////////////////////////////////
	// SHADER FORMATS
	/////////////////////////////////////////////////////////////////

	enum class  ShaderDataType_ : uint32_t
	{
		None = 0, Bool,
		Int, Int2, Int3, Int4,
		Uint, Uint2, Uint3, Uint4,
		Float, Float2, Float3, Float4,
	};

	// TODO: Do's >> Need to bind to specific stages for better performance!
	enum ShaderVisibility_ : uint8_t
	{
		ShaderVisibility_All =				0,
		ShaderVisibility_VertexShader =		1,
		ShaderVisibility_HullShader =		2,
		ShaderVisibility_DomainShader =		3,
		ShaderVisibility_GeometryShader =	4,
		ShaderVisibility_PixelShader =		5,
		ShaderVisibility_MeshShader =		6,
		ShaderVisibility_TaskShader =		7,

	//	ShaderVisibility_VertexShader = (1 << 0),
	//	ShaderVisibility_HullShader = (1 << 1),
	//	ShaderVisibility_DomainShader = (1 << 2),
	//	ShaderVisibility_GeometryShader = (1 << 3),
	//	ShaderVisibility_PixelShader = (1 << 4),
	//	ShaderVisibility_MeshShader = (1 << 5),
	//	ShaderVisibility_TaskShader = (1 << 6),
	//	ShaderVisibility_All = 0xFFFFFFFFu,

	/*	RaytracingShaderVisibility_Global = (1 << 7),
		RaytracingShaderVisibility_Local_RayGen = (1 << 8),
		RaytracingShaderVisibility_Local_Hitgroup = (1 << 9),
		RaytracingShaderVisibility_Local_Miss = (1 << 10),
		RaytracingShaderVisibility_Local_Callable = (1 << 11),
		RaytracingShaderVisibility_Mask = (RaytracingShaderVisibility_Global & RaytracingShaderVisibility_Local_RayGen &
											RaytracingShaderVisibility_Local_Hitgroup & RaytracingShaderVisibility_Local_Miss & RaytracingShaderVisibility_Local_Callable)*/
	};

	enum ShaderElementType_ : uint8_t
	{
		ShaderElementType_Unknown,
		ShaderElementType_RootConstant,
		ShaderElementType_ConstantBufferView,
		ShaderElementType_ShaderResourceView,
		ShaderElementType_UnorderedAccessView,
		ShaderElementType_DescriptorTable,
		ShaderElementType_Sampler,
	};

	enum ShaderDescriptorTableType_ : uint8_t
	{
		ShaderDescriptorTableType_None,
		ShaderDescriptorTableType_CBV_SRV_UAV,
		ShaderDescriptorTableType_Sampler,
	};

	/////////////////////////////////////////////////////////////////
	// RESOURCE FORMATS
	/////////////////////////////////////////////////////////////////

	enum ResourceAccessFlag_ : uint8_t
	{
		ResourceAccessFlag_GpuRead = 1,		// depending on the state of the other flags will create a resource that will be placed in the most suitable memory heap
		ResourceAccessFlag_GpuWrite = 2,		// will place bufffer into Default heap
		ResourceAccessFlag_CpuWrite = 4,		// will place buffer into Upload heap if gpu write is unset
		ResourceAccessFlag_CpuRead = 8,		// will create an allocation with Readback heap
		ResourceAccessFlag_DesktopPlatformPrimaryResidenceGPU = 32 // keeps the buffer in the default heap, upload is going to be managed via the staging buffer								
	};
	ENUM_FLAG_OPERATORS(ResourceAccessFlag_);

	enum ViewAccessType_ : uint8_t
	{
		ViewAccessType_GpuRead = 1,				//SRV - Read only data
		ViewAccessType_GpuReadWrite = 2,		//UAV - Read write data
		ViewAccessType_ConstantBuffer = 3,		//CBV - Special view that is Read and stored in GPU registers in special way (in SGPRs) on SOME GPU architechtures, generally good for consistently Read data that is shared (uniform) across all threads of execution that only changes between draws/dispatches
		ViewAccessType_DSV = 4,					//DSV - DepthStencilView
		ViewAccessType_RTV = 5,					//RTV - RenderTargetView
	};

	enum BufferUsage //unused atm
	{
		USAGE_STATIC,
		USAGE_DYNAMIC
	};

	// Pretty much stolen from the DXGI formats
	enum DataFormat_ : uint32_t
	{
		DataFormat_UNKNOWN = 0,
		DataFormat_R32G32B32A32_TYPELESS = 1,
		DataFormat_R32G32B32A32_FLOAT = 2,
		DataFormat_R32G32B32A32_UINT = 3,
		DataFormat_R32G32B32A32_SINT = 4,
		DataFormat_R32G32B32_TYPELESS = 5,
		DataFormat_R32G32B32_FLOAT = 6,
		DataFormat_R32G32B32_UINT = 7,
		DataFormat_R32G32B32_SINT = 8,
		DataFormat_R16G16B16A16_TYPELESS = 9,
		DataFormat_R16G16B16A16_FLOAT = 10,
		DataFormat_R16G16B16A16_UNORM = 11,
		DataFormat_R16G16B16A16_UINT = 12,
		DataFormat_R16G16B16A16_SNORM = 13,
		DataFormat_R16G16B16A16_SINT = 14,
		DataFormat_R32G32_TYPELESS = 15,
		DataFormat_R32G32_FLOAT = 16,
		DataFormat_R32G32_UINT = 17,
		DataFormat_R32G32_SINT = 18,
		DataFormat_R32G8X24_TYPELESS = 19,
		DataFormat_D32_FLOAT_S8X24_UINT = 20,
		DataFormat_R32_FLOAT_X8X24_TYPELESS = 21,
		DataFormat_X32_TYPELESS_G8X24_UINT = 22,
		DataFormat_R10G10B10A2_TYPELESS = 23,
		DataFormat_R10G10B10A2_UNORM = 24,
		DataFormat_R10G10B10A2_UINT = 25,
		DataFormat_R11G11B10_FLOAT = 26,
		DataFormat_R8G8B8A8_TYPELESS = 27,
		DataFormat_R8G8B8A8_UNORM = 28,
		DataFormat_R8G8B8A8_UNORM_SRGB = 29,
		DataFormat_R8G8B8A8_UINT = 30,
		DataFormat_R8G8B8A8_SNORM = 31,
		DataFormat_R8G8B8A8_SINT = 32,
		DataFormat_R16G16_TYPELESS = 33,
		DataFormat_R16G16_FLOAT = 34,
		DataFormat_R16G16_UNORM = 35,
		DataFormat_R16G16_UINT = 36,
		DataFormat_R16G16_SNORM = 37,
		DataFormat_R16G16_SINT = 38,
		DataFormat_R32_TYPELESS = 39,
		DataFormat_D32_FLOAT = 40,
		DataFormat_R32_FLOAT = 41,
		DataFormat_R32_UINT = 42,
		DataFormat_R32_SINT = 43,
		DataFormat_R24G8_TYPELESS = 44,
		DataFormat_D24_UNORM_S8_UINT = 45,
		DataFormat_R24_UNORM_X8_TYPELESS = 46,
		DataFormat_X24_TYPELESS_G8_UINT = 47,
		DataFormat_R8G8_TYPELESS = 48,
		DataFormat_R8G8_UNORM = 49,
		DataFormat_R8G8_UINT = 50,
		DataFormat_R8G8_SNORM = 51,
		DataFormat_R8G8_SINT = 52,
		DataFormat_R16_TYPELESS = 53,
		DataFormat_R16_FLOAT = 54,
		DataFormat_D16_UNORM = 55,
		DataFormat_R16_UNORM = 56,
		DataFormat_R16_UINT = 57,
		DataFormat_R16_SNORM = 58,
		DataFormat_R16_SINT = 59,
		DataFormat_R8_TYPELESS = 60,
		DataFormat_R8_UNORM = 61,
		DataFormat_R8_UINT = 62,
		DataFormat_R8_SNORM = 63,
		DataFormat_R8_SINT = 64,
		DataFormat_A8_UNORM = 65,
		DataFormat_R1_UNORM = 66,
		DataFormat_R9G9B9E5_SHAREDEXP = 67,
		DataFormat_R8G8_B8G8_UNORM = 68,
		DataFormat_G8R8_G8B8_UNORM = 69,
		DataFormat_BC1_TYPELESS = 70,
		DataFormat_BC1_UNORM = 71,
		DataFormat_BC1_UNORM_SRGB = 72,
		DataFormat_BC2_TYPELESS = 73,
		DataFormat_BC2_UNORM = 74,
		DataFormat_BC2_UNORM_SRGB = 75,
		DataFormat_BC3_TYPELESS = 76,
		DataFormat_BC3_UNORM = 77,
		DataFormat_BC3_UNORM_SRGB = 78,
		DataFormat_BC4_TYPELESS = 79,
		DataFormat_BC4_UNORM = 80,
		DataFormat_BC4_SNORM = 81,
		DataFormat_BC5_TYPELESS = 82,
		DataFormat_BC5_UNORM = 83,
		DataFormat_BC5_SNORM = 84,
		DataFormat_B5G6R5_UNORM = 85,
		DataFormat_B5G5R5A1_UNORM = 86,
		DataFormat_B8G8R8A8_UNORM = 87,
		DataFormat_B8G8R8X8_UNORM = 88,
		DataFormat_R10G10B10_XR_BIAS_A2_UNORM = 89,
		DataFormat_B8G8R8A8_TYPELESS = 90,
		DataFormat_B8G8R8A8_UNORM_SRGB = 91,
		DataFormat_B8G8R8X8_TYPELESS = 92,
		DataFormat_B8G8R8X8_UNORM_SRGB = 93,
		DataFormat_BC6H_TYPELESS = 94,
		DataFormat_BC6H_UF16 = 95,
		DataFormat_BC6H_SF16 = 96,
		DataFormat_BC7_TYPELESS = 97,
		DataFormat_BC7_UNORM = 98,
		DataFormat_BC7_UNORM_SRGB = 99,
		DataFormat_AYUV = 100,
		DataFormat_Y410 = 101,
		DataFormat_Y416 = 102,
		DataFormat_NV12 = 103,
		DataFormat_P010 = 104,
		DataFormat_P016 = 105,
		DataFormat_420_OPAQUE = 106,
		DataFormat_YUY2 = 107,
		DataFormat_Y210 = 108,
		DataFormat_Y216 = 109,
		DataFormat_NV11 = 110,
		DataFormat_AI44 = 111,
		DataFormat_IA44 = 112,
		DataFormat_P8 = 113,
		DataFormat_A8P8 = 114,
		DataFormat_B4G4R4A4_UNORM = 115,

		DataFormat_P208 = 130,
		DataFormat_V208 = 131,
		DataFormat_V408 = 132,

		DataFormat_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE = 189,
		DataFormat_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE = 190,

		DataFormat_DEFAULT_BACKBUFFER = DataFormat_R8G8B8A8_UNORM,
		DataFormat_DEFAULT_DEPTHSTENCIL = DataFormat_D32_FLOAT,
		DataFormat_FORCE_UINT = 0xffffffff
	};

	enum GPUResourceType_ : uint32_t
	{
		GpuResourceType_Buffer,
		GpuResourceType_Texture,

		GpuResourceType_ShaderView,
		GpuResourceType_AccelerationStructureHandle,
		GpuResourceType_ShaderTableHandle,
		GpuResourceType_DescriptorTable,
		GpuResourceType_RenderTargetView,
		GpuResourceType_DepthStencilView,

		GpuResourceType_SamplerState,

//		GpuResourceType_VertexView,
//		GpuResourceType_IndexView,

//		GpuResourceType_CommandQueue,	// queueueueue
//		GpuResourceType_Semaphore,		// Lable / fence GPU <-> GPU ??? do i need this ? DX12 does everything with Fences ps5 explicitly reccomends not to use lables for both, vulkan doesnt 
		GpuResourceType_Fence,			// label / fence CPU <-> GPU

		GpuResourceType_ShaderInputsLayout,

		GpuResourceType_PipelineState,

		GpuResourceType_CommandBuffer, // command buffer command list

	};

	enum BufferFlags_
	{
		BufferFlags_NONE = 0,
		BufferFlags_RaytacingStructures = 1,
	};
	ENUM_FLAG_OPERATORS(BufferFlags_);

	enum TextureType_
	{
		TextureType_Default = 0,
		TextureType_RenderTarget,
		TextureType_DepthStencil,
	};

	enum TextureDimension_ : uint8_t
	{
		TextureDimension_Texture1D,
		TextureDimension_Texture1D_ARRAY,
		TextureDimension_Texture2D,
		TextureDimension_Texture2D_MS,
		TextureDimension_Texture2D_ARRAY,
		TextureDimension_Texture2D_MS_ARRAY,
		TextureDimension_Texture3D,
		TextureDimension_CubeMap,
		TextureDimension_CubeMap_ARRAY,

	};

	enum CommandBufferType_ : uint8_t
	{
		CommandBufferType_Graphics,
		CommandBufferType_Compute,
	};

	enum CommandBufferPriority_ : uint8_t
	{
		CommandBufferPriority_Primary,
		CommandBufferPriority_Secondary,
	};

	enum ResourceViewType_
	{
		View_Buffer,				 //SRV/UAV/CBV

		View_VertexBuffer,			 //Address 
		View_IndexBuffer,			 //Address	

		View_Texture,				 //SRV/UAV

		View_RenderTarget,			 //RTV	
		View_DepthRenderTarget,		 //DSV	

		View_BottomLevelAccelerationStructure, //SRV/UAV
		View_TopLevelAccelerationStructure     //SRV/UAV
	};

	/////////////////////////////////////////////////////////////////
	// SAMPLER FORMATS
	/////////////////////////////////////////////////////////////////

	enum SamplerFilter_ : uint32_t
	{
		SamplerFilter_Min_Mag_Mip_Point = 0,
		SamplerFilter_Min_Mag_Point_Mip_Linear,
		SamplerFilter_Min_Point_Mag_Linear_Mip_Point,
		SamplerFilter_Min_Point_Mag_Mip_Linear,
		SamplerFilter_Min_Linear_Mag_Mip_Point,
		SamplerFilter_Min_Linear_Mag_Point_Mip_Linear,
		SamplerFilter_Min_Mag_Linear_Mip_Point,
		SamplerFilter_Min_Mag_Mip_Linear,
		SamplerFilter_Anisotropic,
		SamplerFilter_Comaprison_Min_Mag_Mip_Point,
		SamplerFilter_Comparions_Min_Mag_Point_Mip_Linear,
		SamplerFilter_Comparison_Min_Point_Mag_Linear_Mip_Point,
		SamplerFilter_Comparison_Min_Point_Mag_Mip_Linear,
		SamplerFilter_Comaprison_Min_Linear_Mag_Mip_Point,
		SamplerFilter_Comparison_Min_Linear_Mag_Point_Mip_Linear,
		SamplerFilter_Comparison_Min_Mag_Linear_Mip_Point,
		SamplerFilter_Comparison_Min_Mag_Mip_Linear,
		SamplerFilter_Comparison_Anisotropic,
		SamplerFilter_Minimum_Min_Mag_Mip_Point,
		SamplerFilter_Minimum_Min_Mag_Point_Mip_Linear,
		SamplerFilter_Minimum_Min_Point_Mag_Linear_Mip_Point,
		SamplerFilter_Minimum_Min_Point_Mag_Mip_Linear,
		SamplerFilter_Minimum_Min_Linear_Mag_Mip_Point,
		SamplerFilter_Minimum_Min_Linear_Mag_Point_Mip_Linear,
		SamplerFilter_Minimum_Min_Mag_Linear_Mip_Point,
		SamplerFilter_Minimum_Min_Mag_Mip_Linear,
		SamplerFilter_Minimum_Anisotropic,
		SamplerFilter_Maximum_Min_Mag_Mip_Point,
		SamplerFilter_Maximum_Min_Mag_Point_Mip_Linear,
		SamplerFilter_Maximum_Min_Point_Mag_Linear_Mip_Point,
		SamplerFilter_Maximum_Min_Point_Mag_Mip_Linear,
		SamplerFilter_Maximum_Min_Point_Mag_Mip_Point,
		SamplerFilter_Maximum_Min_Linear_Mag_Point_Mip_Linear,
		SamplerFilter_Maximum_Min_Mag_Linear_Mip_Point,
		SamplerFilter_Maximum_Min_Mag_Mip_Linear,
		SamplerFilter_Maximum_Anisotropic
	};

	enum SamplerMode_ : uint32_t
	{
		SamplerMode_Wrap = 0,
		SamplerMode_Mirror,
		SamplerMode_Clamp,
		SamplerMode_Border,
		SamplerMode_Mirror_Once
	};

	enum SamplerComparisonFunction_ : uint32_t
	{
		SamplerComparisonFunction_Never = 0,
		SamplerComparisonFunction_Less,
		SamplerComparisonFunction_Equal,
		SamplerComparisonFunction_Less_Equal,
		SamplerComparisonFunction_Greater,
		SamplerComparisonFunction_Not_Equal,
		SamplerComparisonFunction_Greater_Equal,
		SamplerComparisonFunction_Always
	};

	enum SamplerBorderColour_ : uint32_t
	{
		SamplerBorderColour_TransparentBlack = 0,
		SamplerBorderColour_White,
		SamplerBorderColour_Black,
	};

	/////////////////////////////////////////////////////////////////
	// HELPER FUNCTIONS
	/////////////////////////////////////////////////////////////////

	uint32_t ShaderDataTypeSizeInBytes(ShaderDataType_ type);

}