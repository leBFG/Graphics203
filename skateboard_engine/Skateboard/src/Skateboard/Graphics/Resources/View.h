#pragma once
#include "sktbdpch.h"

#include "Buffer.h"
#include "Skateboard/Graphics/Resources/GPUResource.h"

namespace Skateboard
{
	class RenderTargetView;
	class DepthStencilView;

	class UnorderedAccessTextureView;
	class ShaderResourceTextureView;

	class ShaderResourceBufferView;
	class UnorderedAccessBufferView;
	class ConstantBufferView;

	class BottomLevelAccelerationStructure;
	class TopLevelAccelerationStructure;

	class AccelerationStructureView;

	//references

	typedef std::shared_ptr<ShaderResourceBufferView> BufferSRVRef;
	typedef std::shared_ptr<UnorderedAccessBufferView> BufferUAVRef;
	typedef std::shared_ptr<ConstantBufferView> BufferCBVRef;

	typedef std::shared_ptr<ShaderResourceTextureView> TextureSRVRef;
	typedef std::shared_ptr<UnorderedAccessTextureView> TextureUAVRef;

	typedef std::shared_ptr<DepthStencilView> DepthStencilViewRef;
	typedef std::shared_ptr<RenderTargetView> RenderTargetViewRef;

	typedef std::shared_ptr<AccelerationStructureView> AccelerationStructureViewRef;
	typedef AccelerationStructureViewRef ASViewRef;

	//weak references

	enum BufferType_
	{
		BufferType_ConstantBuffer,
		BufferType_ByteBuffer,
		BufferType_StructureBuffer,
		BufferType_FormattedBuffer,
	};

	enum BufferViewFlags_
	{
		BufferViewFlags_NONE,
		BufferViewFlags_AppendConsumeBuffer = 1,
	};

	struct BufferViewDesc
	{
		BufferType_ Type;
		uint64_t Offset;
		uint32_t ElementSize;
		uint32_t ElementCount;
		DataFormat_ Format;
		BufferViewFlags_ Flags;

		template<typename T>
		void InitAsStructuredBuffer(uint32_t OffsetInElements, uint32_t elementCount, bool appendconsume = false) {
			Type = BufferType_StructureBuffer;
			Offset = OffsetInElements;
			ElementCount = elementCount;
			ElementSize = sizeof(T);
			Format = DataFormat_UNKNOWN;
			Flags = (appendconsume) ? BufferViewFlags_AppendConsumeBuffer : BufferViewFlags_NONE;
		}

		template<typename T>
		void InitAsConstantBuffer(uint32_t offset) {
			Type = BufferType_ConstantBuffer;
			Offset = ROUND_UP(offset, GraphicsConstants::CONSTANT_BUFFER_ALIGNMENT);
			ElementCount = 1;
			ElementSize = ROUND_UP(sizeof(T), GraphicsConstants::CONSTANT_BUFFER_ALIGNMENT);
			Format = DataFormat_UNKNOWN;
			Flags = BufferViewFlags_NONE;
		}

		void InitAsByteBuffer(uint32_t offset, uint32_t elementCount) {
			Type = BufferType_ByteBuffer;
			Offset = offset;
			ElementCount = elementCount;
			ElementSize = 1;
			Format = DataFormat_UNKNOWN;
			Flags = BufferViewFlags_NONE;
		}

		void InitAsFormatedBuffer(uint32_t offset, uint32_t elementCount,  DataFormat_ format) {
			Type = BufferType_ByteBuffer;
			Offset = offset;
			ElementCount = elementCount;
			ElementSize = 0;
			Format = format;
			Flags = BufferViewFlags_NONE;
		}
	};

	struct TextureViewDesc
	{
		TextureDimension_ Dimension;
		DataFormat_ Format;
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
		float ResourceMinLodClamp;

		//for array
		uint32_t  FirstArraySlice;
		uint32_t  ArraySize;
		uint32_t  PlaneSlice;

		//for uav
		uint32_t MipSlice;
	};

	struct RenderTargetDesc
	{
		TextureDimension_ Dimension;
		DataFormat_ Format;
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
		float ResourceMinLodClamp;

		uint32_t MipSlice;

		//for array
		uint32_t  FirstArraySlice;
		uint32_t  ArraySize;
		uint32_t  PlaneSlice;

		uint32_t  FirstWSlice;
		uint32_t  Width;
	};

	enum DRT_Flags_
	{
		DRT_Flags_ReadOnly_Depth = 0,
		DRT_Flags_ReadOnly_Stencil = 1,
	};

	struct DepthStencilDesc
	{
		TextureDimension_ Dimension;
		DataFormat_ Format;
		uint32_t MipLevels;
		float ResourceMinLodClamp;

		uint32_t MipSlice;

		//for array
		uint32_t  FirstArraySlice;
		uint32_t  ArraySize;
		uint32_t  PlaneSlice;

		DRT_Flags_ Flags;
	};

	class IShaderView
	{
		DISABLE_COPY_AND_MOVE(IShaderView)
	protected:
		IShaderView() = default;
	public:
		virtual ~IShaderView() = default;
		virtual uint64_t GetViewIndex() = 0;

		constexpr virtual ViewAccessType_ GetViewType() = 0;
	};
	
	class BufferView : public GPUResource, public IShaderView
	{
	public:
		BufferType_ GetBufferViewType() const { return Type; }
		uint32_t GetSize() const { return Size; }
		uint64_t GetOffset() const { return Offset; }
		BufferRef GetParentResource() const { return m_ParentResource; }

		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_ShaderView; }

	protected:
		BufferView(const BufferViewDesc& desc)
		{
			Type = desc.Type;
			Offset = desc.Offset;
			Size = desc.ElementSize * desc.ElementCount;
		}

		BufferType_ Type;
		uint64_t Offset;
		uint32_t Size;

		BufferRef m_ParentResource;
	};

	class ShaderResourceBufferView : public BufferView
	{
	protected:
		ShaderResourceBufferView(const BufferViewDesc& desc) : BufferView(desc) {}
	public:
		constexpr ViewAccessType_ GetViewType() override { return ViewAccessType_GpuRead; }
	};

	class UnorderedAccessBufferView : public BufferView
	{
	protected:
		UnorderedAccessBufferView(const BufferViewDesc& desc) : BufferView(desc) {}
	public:
		constexpr ViewAccessType_ GetViewType() override { return ViewAccessType_GpuReadWrite; }
	};

	class ConstantBufferView : public BufferView
	{
	protected:
		ConstantBufferView(const BufferViewDesc& desc) : BufferView(desc) {}
	public:
		constexpr ViewAccessType_ GetViewType() override { return ViewAccessType_ConstantBuffer; }
	};

	using BufferSRV = ShaderResourceBufferView;
	using BufferUAV = UnorderedAccessBufferView;
	using BufferCBV = ConstantBufferView;


	class TextureView : public GPUResource, public IShaderView
	{
	protected:
		TextureView(const TextureViewDesc& desc)
		{
			m_Format = desc.Format;
			m_Dimension = desc.Dimension;
			m_MostDetailedMip = desc.MostDetailedMip;
			m_ResourceMinLodClamp = desc.ResourceMinLodClamp;
			m_MipSlice = desc.MipSlice;
			m_FirstArraySlice = desc.FirstArraySlice;
			m_ArraySize = desc.ArraySize;
			m_PlaneSlice = desc.PlaneSlice;
		};

	public:
		uint64_t GetWidth()    const { return (m_ParentResource) ? m_ParentResource->GetWidth()    : 0; }
		uint32_t GetDepth()    const { return (m_ParentResource) ? m_ParentResource->GetDepth()    : 0; }
		uint32_t GetHeight()   const { return (m_ParentResource) ? m_ParentResource->GetHeight()   : 0; }
		uint32_t GetMipCount() const { return (m_ParentResource) ? m_ParentResource->GetMipCount() : 0; }

		DataFormat_ GetFormat() const { return (m_ParentResource) ? m_Format : DataFormat_UNKNOWN; }
		uint32_t GetMostDetailedMip() const { return (m_ParentResource) ? m_MostDetailedMip : 0; }
		float GetResourceMinLodClamp() const { return (m_ParentResource) ? m_ResourceMinLodClamp : 0; }
		uint32_t GetMipSlice() const { return (m_ParentResource) ? m_MipSlice : 0; }
		uint32_t GetFirstArraySlice() const { return (m_ParentResource) ? m_FirstArraySlice : 0; }
		uint32_t GetArraySize() const { return (m_ParentResource) ? m_ArraySize : 0; }
		uint32_t GetPlaneSlice() const { return (m_ParentResource) ? m_PlaneSlice : 0; }

		TextureBufferRef GetParent() { return m_ParentResource; }
		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_ShaderView; }

		virtual ImTextureID GetImTextureID() = 0;

	protected:
		TextureDimension_ m_Dimension;
		DataFormat_ m_Format;
		uint32_t m_MostDetailedMip;
		float m_ResourceMinLodClamp;

		uint32_t m_MipSlice;

		//for array
		uint32_t  m_FirstArraySlice;
		uint32_t  m_ArraySize;
		uint32_t  m_PlaneSlice;

		TextureBufferRef m_ParentResource;
	};

	class ShaderResourceTextureView : public TextureView
	{
	protected:
		ShaderResourceTextureView(const TextureViewDesc& desc) : TextureView(desc) {}
	public:
		constexpr ViewAccessType_ GetViewType() override { return ViewAccessType_GpuRead; }
	};

	class UnorderedAccessTextureView : public TextureView
	{
	protected:
		UnorderedAccessTextureView(const TextureViewDesc& desc) : TextureView(desc) {}
	public:
		constexpr ViewAccessType_ GetViewType() override { return ViewAccessType_GpuReadWrite; }
	};

	class RenderTargetView : public GPUResource, public IShaderView
	{
	public:
		uint64_t GetWidth()    const { return (m_ParentResource) ? m_ParentResource->GetWidth() : 0; }
		uint32_t GetDepth()    const { return (m_ParentResource) ? m_ParentResource->GetDepth() : 0; }
		uint32_t GetHeight()   const { return (m_ParentResource) ? m_ParentResource->GetHeight() : 0; }
		uint32_t GetMipCount() const { return (m_ParentResource) ? m_ParentResource->GetMipCount() : 0; }
		DataFormat_ GetFormat() const { return Format; }
		glm::vec4 GetRTClearValue() const { return (m_ParentResource) ? m_ParentResource->RTClearValue() : GRAPHICS_FRAMEBUFFER_DEFAULT_CLEAR_COLOUR; }
		ClearValue GetClearValue() const { return(m_ParentResource) ? m_ParentResource->GetClearValue() : ClearValue{ .RenderClearValue = GRAPHICS_FRAMEBUFFER_DEFAULT_CLEAR_COLOUR }; }


		TextureBufferRef GetParent() { return m_ParentResource; }
		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_RenderTargetView; }
		constexpr ViewAccessType_ GetViewType() override { return ViewAccessType_RTV; }

	protected:
		RenderTargetView(const RenderTargetDesc* desc)
		{
			if (desc)
			{
				Format = desc->Format;
			}
		}
	protected:
		DataFormat_ Format;
		TextureBufferRef m_ParentResource;
	};

	class DepthStencilView : public GPUResource, public IShaderView
	{
	public:
		DataFormat_ GetFormat() const { return Format; }
		DRT_Flags_ GetFlags() const { return Flags; }

		ClearValue GetClearValue() const { return(m_ParentResource) ? m_ParentResource->GetClearValue() : ClearValue{.Depth=GRAPHICS_DEPTH_DEFAULT_CLEAR_COLOUR, .Stencil=GRAPHICS_STENCIL_DEFAULT_CLEAR_COLOUR }; }

		float GetDepthClearValue() const { return(m_ParentResource) ? m_ParentResource->DepthClearValue() : GRAPHICS_DEPTH_DEFAULT_CLEAR_COLOUR; }
		uint32_t GetStencilClearValue() const { return(m_ParentResource) ? m_ParentResource->StencilClearValue() : GRAPHICS_STENCIL_DEFAULT_CLEAR_COLOUR; }

		uint64_t GetWidth()    const { return (m_ParentResource) ? m_ParentResource->GetWidth() : 0; }
		uint32_t GetDepth()    const { return (m_ParentResource) ? m_ParentResource->GetDepth() : 0; }
		uint32_t GetHeight()   const { return (m_ParentResource) ? m_ParentResource->GetHeight() : 0; }
		uint32_t GetMipCount() const { return (m_ParentResource) ? m_ParentResource->GetMipCount() : 0; }

		TextureBufferRef GetParent() { return m_ParentResource; }

		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_DepthStencilView; }
		constexpr ViewAccessType_ GetViewType() override { return ViewAccessType_DSV; }

	protected:
		DepthStencilView(const DepthStencilDesc* desc) 
		{
			if (desc)
			{
				Format = desc->Format;
				Flags = desc->Flags;
			}
		}

		DataFormat_ Format;
		DRT_Flags_ Flags;

		TextureBufferRef m_ParentResource;
	};

	enum IndexFormat : uint8_t
	{
		bit32 = 4,
		bit16 = 2,
		bit8 = 1
	};

	//=========================== Index and Vertex Buffers ====================================
	// as they are a hint for the 

	struct IndexBufferView
	{
		IndexFormat m_Format;
		size_t m_Offset;
		uint32_t m_IndexCount;

		BufferRef m_ParentResource;
	};

	enum SKTBD_PRIMITIVE_TOPOLOGY : uint8_t
	{
		SKTBD_PRIMITIVE_TOPOLOGY_UNDEFINED = 0,
		SKTBD_PRIMITIVE_TOPOLOGY_POINTLIST = 1,
		SKTBD_PRIMITIVE_TOPOLOGY_LINELIST = 2,
		SKTBD_PRIMITIVE_TOPOLOGY_LINESTRIP = 3,
		SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4,
		SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5,
		SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLEFAN,
		SKTBD_PRIMITIVE_TOPOLOGY_LINELIST_ADJ = 10,
		SKTBD_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ = 11,
		SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ = 12,
		SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ = 13,
		SKTBD_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST = 33,
		SKTBD_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST = 34,
		SKTBD_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST = 35,
		SKTBD_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST = 36,
		SKTBD_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST = 37,
		SKTBD_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST = 38,
		SKTBD_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST = 39,
		SKTBD_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST = 40,
		SKTBD_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST = 41,
		SKTBD_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST = 42,
		SKTBD_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST = 43,
		SKTBD_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST = 44,
		SKTBD_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST = 45,
		SKTBD_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST = 46,
		SKTBD_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST = 47,
		SKTBD_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST = 48,
		SKTBD_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST = 49,
		SKTBD_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST = 50,
		SKTBD_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST = 51,
		SKTBD_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST = 52,
		SKTBD_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST = 53,
		SKTBD_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST = 54,
		SKTBD_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST = 55,
		SKTBD_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST = 56,
		SKTBD_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST = 57,
		SKTBD_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST = 58,
		SKTBD_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST = 59,
		SKTBD_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST = 60,
		SKTBD_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST = 61,
		SKTBD_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST = 62,
		SKTBD_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST = 63,
		SKTBD_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST = 64,
	};

	struct VertexBufferView 
	{
		uint32_t m_Offset;
		uint32_t m_VertexCount;
		uint32_t m_VertexStride;

		BufferRef m_ParentResource;
	};
	// ======================================================= Acceleration structures ==============================================

	enum SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS
	{
		SKTBD_ACCELERATION_STRUCT_BUILD_FLAG_NONE = 0,
		SKTBD_ACCELERATION_STRUCT_BUILD_FLAG_ALLOW_UPDATE = 0x1,
		SKTBD_ACCELERATION_STRUCT_BUILD_FLAG_ALLOW_COMPACTION = 0x2,
		SKTBD_ACCELERATION_STRUCT_BUILD_FLAG_PREFER_FAST_TRACE = 0x4,
		SKTBD_ACCELERATION_STRUCT_BUILD_FLAG_PREFER_FAST_BUILD = 0x8,
		SKTBD_ACCELERATION_STRUCT_BUILD_FLAG_MINIMIZE_MEMORY = 0x10,
		SKTBD_ACCELERATION_STRUCT_BUILD_FLAG_PERFORM_UPDATE = 0x20
	};
	ENUM_FLAG_OPERATORS(SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS)


	// =========================== BLAS STRUCTURES ==================================================================================

	struct GeometryTriangleDesc
	{
		DataFormat_ VertexPositionFormat;
		VertexBufferView* pVertexBuffer;
		uint32_t StartVertexLocation;
		uint32_t VertexCount;
		IndexBufferView* pIndexBuffer;
		uint32_t StartIndexLocation;
		uint32_t IndexCount;
	};

	struct GeometryAABBDesc
	{
		uint64_t AABBCount;
		struct
		{
			uint64_t Offset;
			Buffer* pProceduralGeometryAABBBuffer;
		}BufferAABB;
	};

	struct GeometryDesc
	{
		GeometryType_ Type;
		GeometryFlags_ Flags;
		union
		{
			GeometryTriangleDesc Triangles;
			GeometryAABBDesc Procedurals;
		};
	} ;

	struct BottomLevelAccelerationStructureDesc
	{
		uint64_t BufferOffset;
		uint32_t NumGeometries;
		GeometryDesc* Geometries;
	};

	// =========================== BLAS GPU Resource ==================================================================================

	typedef uint64_t AccelerationStructureHandle;

	struct AccelerationStructureData
	{
		BufferRef m_ParentResource;
		uint64_t m_Offset;
		AccelerationStructureHandle m_Address;
	};

	class BottomLevelAccelerationStructure : public GPUResource
	{
	public:
		BottomLevelAccelerationStructure() : m_Handle(AccelerationStructureData())
		{
		}

		BottomLevelAccelerationStructure(AccelerationStructureData Handle) : m_Handle(std::move(Handle))
		{
		}

	public:
		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_AccelerationStructureHandle; }

		BufferRef GetParent() const { return m_Handle.m_ParentResource; }
		uint64_t GetOffset() const { return m_Handle.m_Offset; }

		AccelerationStructureHandle GetHandle() const  { return m_Handle.m_Address; }
		AccelerationStructureData GetData() const { return m_Handle; }

	protected:
		AccelerationStructureData m_Handle;
	};

	// =========================== TLAS Structres ==================================================================================

	enum TLASInstanceFlags_ : uint8_t
	{
		SKTBD_INSTANCE_FLAG_NONE = 0,
		SKTBD_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 1 << 0,
		SKTBD_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 1<<1,
		SKTBD_INSTANCE_FLAG_FORCE_OPAQUE = 1<<2,
		SKTBD_INSTANCE_FLAG_FORCE_NON_OPAQUE = 1<<3,

		SKTBD_INSTANCE_FLAG_ENABLE_REBRAIDING = 1<<4,
		SKTBD_INSTANCE_FLAG_DISABLE_REBRAIDING = 1<<5,
	};

	struct TLASInstanceData
	{
		glm::mat4x3 Transform;
		unsigned InstanceID : 24;
		unsigned InstanceMask : 8;
		unsigned InstanceContributionToHitGroupIndex : 24;
		TLASInstanceFlags_ TlasFlags : 8;
		AccelerationStructureHandle BottomASHandle;
	};

	struct TopLevelAccelerationStructureDesc
	{
		uint64_t BufferOffset;

		uint32_t NumInstances;
		uint32_t FirstInstanceOffset;
		Buffer* InstanceDataBuffer;
	};

	// =========================== TLAS GPU Resource ==================================================================================

	class TopLevelAccelerationStructure : public GPUResource
	{
	public:
		TopLevelAccelerationStructure() : m_Handle(AccelerationStructureData())
		{
		}

		TopLevelAccelerationStructure(AccelerationStructureData Handle) : m_Handle(std::move(Handle))
		{
		}

	public:
		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_AccelerationStructureHandle; }
		
		BufferRef GetParent() const { return m_Handle.m_ParentResource; }
		uint64_t GetOffset() const { return m_Handle.m_Offset; }

		AccelerationStructureHandle GetHandle() const { return m_Handle.m_Address; }
		AccelerationStructureData GetData() const { return m_Handle; }

	protected:
		AccelerationStructureData m_Handle;
	};

	enum AccelerationStructureType_ : uint8_t
	{
		BottomLevel,
		TopLevel
	};

	class AccelerationStructureView : public GPUResource, public IShaderView
	{
	protected:
		AccelerationStructureView(AccelerationStructureData Handle) : m_StructureHandle(std::move(Handle))
		{
		}

	public:
		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_ShaderView; }
		constexpr ViewAccessType_ GetViewType() override { return ViewAccessType_GpuRead; }

	protected:
		AccelerationStructureData m_StructureHandle;
	};

	struct AccelerationStructureDesc
	{
		template<typename ASDesc>
		AccelerationStructureDesc(ASDesc desc) : m_Desc(std::move(desc))
		{
			if constexpr (std::is_same_v<BottomLevelAccelerationStructureDesc, ASDesc>)
			{
				m_Type = BottomLevel;
			}
			else
			{
				m_Type = TopLevel;
			}
		}

		AccelerationStructureType_ m_Type;
		std::variant<BottomLevelAccelerationStructureDesc, TopLevelAccelerationStructureDesc> m_Desc;
	};

	struct AccelerationStructure
	{
		template<typename AS>
		AccelerationStructure(AS desc) : m_Handle(std::move(desc))
		{
			if constexpr (std::is_same_v<BottomLevelAccelerationStructure, AS>)
			{
				m_Type = BottomLevel;
			}
			else
			{
				m_Type = TopLevel;
			}
		}

		AccelerationStructureType_ m_Type;
		std::variant<BottomLevelAccelerationStructure, TopLevelAccelerationStructure> m_Handle;
	};

	typedef uint64_t ShaderTableStartAddress;

	enum ShaderTableGroup
	{
		Hit,
		Miss,
		Callable,
		Raygen
	};

	struct ShaderTable
	{
		uint64_t Offset;
		uint64_t Stride;
		uint64_t Size;
		BufferRef m_StorageBuffer;
		ShaderTableStartAddress m_Address;
	};

	//typedef uint64_t ShaderID;

	//struct ShaderRecord
	//{
	//	ShaderID m_ID;
	//	//uint64_t m_Constant;
	//};

}