#pragma once
#include "CommandBuffer.h"
#include "GPUResource.h"

namespace Skateboard
{
	class GraphicsCommandBuffer;
	class ComputeCommandBuffer;
	class CommandQueue;
	class Fence;
	class TextureBuffer;
	class Buffer;
	class Semaphore;

	typedef std::shared_ptr<GraphicsCommandBuffer> GraphicsCommandBufferRef;
	typedef std::shared_ptr<ComputeCommandBuffer> ComputeCommandBufferRef;
	typedef std::shared_ptr<CommandQueue> CommandQueueRef;
	typedef std::shared_ptr<Fence> FenceRef;
	typedef std::shared_ptr<Semaphore> SemaphoreRef;

	enum SKTBD_SYNC
	{
		SKTBD_SYNC_NONE = 0,
		SKTBD_SYNC_ALL = 0x1,
		SKTBD_SYNC_DRAW = 0x2,
		SKTBD_SYNC_INDEX_INPUT = 0x4,
		SKTBD_SYNC_VERTEX_SHADING = 0x8,
		SKTBD_SYNC_PIXEL_SHADING = 0x10,
		SKTBD_SYNC_DEPTH_STENCIL = 0x20,
		SKTBD_SYNC_RENDER_TARGET = 0x40,
		SKTBD_SYNC_COMPUTE_SHADING = 0x80,
		SKTBD_SYNC_RAYTRACING = 0x100,
		SKTBD_SYNC_COPY = 0x200,
		SKTBD_SYNC_RESOLVE = 0x400,
		SKTBD_SYNC_EXECUTE_INDIRECT = 0x800,
		SKTBD_SYNC_PREDICATION = 0x800,
		SKTBD_SYNC_ALL_SHADING = 0x1000,
		SKTBD_SYNC_NON_PIXEL_SHADING = 0x2000,
		SKTBD_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO = 0x4000,
		SKTBD_SYNC_CLEAR_UNORDERED_ACCESS_VIEW = 0x8000,
//		SKTBD_SYNC_VIDEO_DECODE = 0x100000,
//		SKTBD_SYNC_VIDEO_PROCESS = 0x200000,
//		SKTBD_SYNC_VIDEO_ENCODE = 0x400000,
		SKTBD_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE = 0x800000,
		SKTBD_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE = 0x1000000,
		SKTBD_SYNC_SPLIT = 0x80000000

	};
	ENUM_FLAG_OPERATORS(SKTBD_SYNC)

	enum SKTBD_ACCESS
	{
		SKTBD_ACCESS_COMMON = 0,
		SKTBD_ACCESS_VERTEX_BUFFER = 0x1,
		SKTBD_ACCESS_CONSTANT_BUFFER = 0x2,
		SKTBD_ACCESS_INDEX_BUFFER = 0x4,
		SKTBD_ACCESS_RENDER_TARGET = 0x8,
		SKTBD_ACCESS_UNORDERED_ACCESS = 0x10,
		SKTBD_ACCESS_DEPTH_STENCIL_WRITE = 0x20,
		SKTBD_ACCESS_DEPTH_STENCIL_READ = 0x40,
		SKTBD_ACCESS_SHADER_RESOURCE = 0x80,
		SKTBD_ACCESS_STREAM_OUTPUT = 0x100,
		SKTBD_ACCESS_INDIRECT_ARGUMENT = 0x200,
		SKTBD_ACCESS_PREDICATION = 0x200,
		SKTBD_ACCESS_COPY_DEST = 0x400,
		SKTBD_ACCESS_COPY_SOURCE = 0x800,
		SKTBD_ACCESS_RESOLVE_DEST = 0x1000,
		SKTBD_ACCESS_RESOLVE_SOURCE = 0x2000,
		SKTBD_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ = 0x4000,
		SKTBD_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE = 0x8000,
		SKTBD_ACCESS_SHADING_RATE_SOURCE = 0x10000,
//		SKTBD_ACCESS_VIDEO_DECODE_READ = 0x20000,
//		SKTBD_ACCESS_VIDEO_DECODE_WRITE = 0x40000,
//		SKTBD_ACCESS_VIDEO_PROCESS_READ = 0x80000,
//		SKTBD_ACCESS_VIDEO_PROCESS_WRITE = 0x100000,
//		SKTBD_ACCESS_VIDEO_ENCODE_READ = 0x200000,
//		SKTBD_ACCESS_VIDEO_ENCODE_WRITE = 0x400000,
		SKTBD_ACCESS_NO_ACCESS = 0x80000000
	};
	ENUM_FLAG_OPERATORS(SKTBD_ACCESS)

	enum SKTBD_LAYOUT
	{
		SKTBD_LAYOUT_UNDEFINED = 0xffffffff,
		SKTBD_LAYOUT_COMMON = 0,
		SKTBD_LAYOUT_PRESENT = 0,
		SKTBD_LAYOUT_GENERIC_READ = (SKTBD_LAYOUT_PRESENT + 1),
		SKTBD_LAYOUT_RENDER_TARGET = (SKTBD_LAYOUT_GENERIC_READ + 1),
		SKTBD_LAYOUT_UNORDERED_ACCESS = (SKTBD_LAYOUT_RENDER_TARGET + 1),
		SKTBD_LAYOUT_DEPTH_STENCIL_WRITE = (SKTBD_LAYOUT_UNORDERED_ACCESS + 1),
		SKTBD_LAYOUT_DEPTH_STENCIL_READ = (SKTBD_LAYOUT_DEPTH_STENCIL_WRITE + 1),
		SKTBD_LAYOUT_SHADER_RESOURCE = (SKTBD_LAYOUT_DEPTH_STENCIL_READ + 1),
		SKTBD_LAYOUT_COPY_SOURCE = (SKTBD_LAYOUT_SHADER_RESOURCE + 1),
		SKTBD_LAYOUT_COPY_DEST = (SKTBD_LAYOUT_COPY_SOURCE + 1),
		SKTBD_LAYOUT_RESOLVE_SOURCE = (SKTBD_LAYOUT_COPY_DEST + 1),
		SKTBD_LAYOUT_RESOLVE_DEST = (SKTBD_LAYOUT_RESOLVE_SOURCE + 1),
		SKTBD_LAYOUT_SHADING_RATE_SOURCE = (SKTBD_LAYOUT_RESOLVE_DEST + 1),
		SKTBD_LAYOUT_VIDEO_DECODE_READ = (SKTBD_LAYOUT_SHADING_RATE_SOURCE + 1),
		SKTBD_LAYOUT_VIDEO_DECODE_WRITE = (SKTBD_LAYOUT_VIDEO_DECODE_READ + 1),
		SKTBD_LAYOUT_VIDEO_PROCESS_READ = (SKTBD_LAYOUT_VIDEO_DECODE_WRITE + 1),
		SKTBD_LAYOUT_VIDEO_PROCESS_WRITE = (SKTBD_LAYOUT_VIDEO_PROCESS_READ + 1),
		SKTBD_LAYOUT_VIDEO_ENCODE_READ = (SKTBD_LAYOUT_VIDEO_PROCESS_WRITE + 1),
		SKTBD_LAYOUT_VIDEO_ENCODE_WRITE = (SKTBD_LAYOUT_VIDEO_ENCODE_READ + 1),
		SKTBD_LAYOUT_DIRECT_QUEUE_COMMON = (SKTBD_LAYOUT_VIDEO_ENCODE_WRITE + 1),
		SKTBD_LAYOUT_DIRECT_QUEUE_GENERIC_READ = (SKTBD_LAYOUT_DIRECT_QUEUE_COMMON + 1),
		SKTBD_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS = (SKTBD_LAYOUT_DIRECT_QUEUE_GENERIC_READ + 1),
		SKTBD_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE = (SKTBD_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS + 1),
		SKTBD_LAYOUT_DIRECT_QUEUE_COPY_SOURCE = (SKTBD_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE + 1),
		SKTBD_LAYOUT_DIRECT_QUEUE_COPY_DEST = (SKTBD_LAYOUT_DIRECT_QUEUE_COPY_SOURCE + 1),
		SKTBD_LAYOUT_COMPUTE_QUEUE_COMMON = (SKTBD_LAYOUT_DIRECT_QUEUE_COPY_DEST + 1),
		SKTBD_LAYOUT_COMPUTE_QUEUE_GENERIC_READ = (SKTBD_LAYOUT_COMPUTE_QUEUE_COMMON + 1),
		SKTBD_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS = (SKTBD_LAYOUT_COMPUTE_QUEUE_GENERIC_READ + 1),
		SKTBD_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE = (SKTBD_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS + 1),
		SKTBD_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE = (SKTBD_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE + 1),
		SKTBD_LAYOUT_COMPUTE_QUEUE_COPY_DEST = (SKTBD_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE + 1),
		SKTBD_LAYOUT_VIDEO_QUEUE_COMMON = (SKTBD_LAYOUT_COMPUTE_QUEUE_COPY_DEST + 1)
	};

	//indicates which subresources to barrier, Defaults constructor indicates -> ALL Subresources, Based on D3D12 Enhanced Barrier API https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_barrier_subresource_range
	struct TextureSubresourceRange
	{
		uint32_t IndexOrFirstMipLevel = 0xffffffff;
		uint32_t NumMipLevels = 0;
		uint32_t FirstArraySlice = 0;
		uint32_t NumArraySlices = 0;
		uint32_t FirstPlane = 0;
		uint32_t NumPlanes = 0;
	};

	enum TextureBarrierFlags
	{
		TextureBarrierFlags_None,
		TextureBarrierFlags_Discard
	};
	ENUM_FLAG_OPERATORS(TextureBarrierFlags)

	struct TextureBarrier
	{
		SKTBD_SYNC SyncBefore;
		SKTBD_SYNC SyncAfter;
		SKTBD_ACCESS AccessBefore;
		SKTBD_ACCESS AccessAfter;
		SKTBD_LAYOUT LayoutBefore;
		SKTBD_LAYOUT LayoutAfter;
		TextureBuffer* Resource;
		TextureSubresourceRange SubresourceRange;
		TextureBarrierFlags Flags;
	};

	struct BufferBarrier
	{
		SKTBD_SYNC SyncBefore;
		SKTBD_SYNC SyncAfter;
		SKTBD_ACCESS AccessBefore;
		SKTBD_ACCESS AccessAfter;
		Buffer* Resource;
		uint64_t Offset;
		uint64_t Size;
	};

	struct GlobalBarrier
	{
		SKTBD_SYNC SyncBefore;
		SKTBD_SYNC SyncAfter;
		SKTBD_ACCESS AccessBefore;
		SKTBD_ACCESS AccessAfter;
	};

	enum BarrierType
	{
		GLOBAL_BARRIER,	 //work cvhanges for all resources used by the Command Buffer
		TEXTURE_BARRIER, //work and layout changes for specific texture resource
		BUFFER_BARRIER	 //work changes for a specific buffer resource
	};
	
	struct BarrierGroup
	{
		//template<typename BarrierType, template<> typename Barriers>
		//BarrierGroup(const Barriers<>& barrierArray)
		//{
		//	if constexpr (std::is_same_v<BarrierType, GlobalBarrier>)
		//	{
		//		m_Type = GLOBAL_BARRIER;
		//	}
		//	else if constexpr (std::is_same_v<BarrierType, TextureBarrier>)
		//	{
		//		m_Type = TEXTURE_BARRIER;
		//	}
		//	else if constexpr (std::is_same_v<BarrierType, BufferBarrier>)
		//	{
		//		m_Type = BUFFER_BARRIER;
		//	}

		//	m_barrierCount = barrierArray.size();
		//	m_Barriers = barrierArray.data();
		//};

		template<typename BarrierType>
		BarrierGroup(BarrierType* barrier, uint32_t Count = 1)
		{
			if constexpr (std::is_same_v<BarrierType, GlobalBarrier>)
			{
				m_Type = GLOBAL_BARRIER;
			}
			else if constexpr (std::is_same_v<BarrierType, TextureBarrier>)
			{
				m_Type = TEXTURE_BARRIER;
			}
			else if constexpr (std::is_same_v<BarrierType, BufferBarrier>)
			{
				m_Type = BUFFER_BARRIER;
			}

			m_barrierCount = Count;
			m_Barriers = barrier;
		}

		BarrierType m_Type;
		uint32_t m_barrierCount;
		std::variant
			<
			TextureBarrier*,
			BufferBarrier*,
			GlobalBarrier*
			> m_Barriers;
	};

	class Fence : public GPUResource
	{
	protected:
		Fence(uint64_t initial_value) {};
	public:
		GPUResourceType_ GetResourceType() override { return GpuResourceType_Fence; }
		virtual void Signal(uint64_t NewValue) = 0;
		virtual uint64_t GetValue() const = 0;
	};

	/*class Semaphore: public GPUResource
	{
		GPUResourceType_ GetResourceType() override; { return GpuResourceType_Semaphore}
	};*/

	class CommandBuffer : public GPUResource
	{
	protected:
		CommandBuffer(CommandBufferPriority_ prior) : m_Priority(prior)
		{
		};

	public:
		//CommandBufferType_ GetType() const { return m_Type; }
		CommandBufferPriority_ GetPriority() const { return m_Priority; }
		GPUResourceType_ GetResourceType() override { return GpuResourceType_CommandBuffer; };

	protected:
	//	CommandBufferType_ m_Type;
		CommandBufferPriority_ m_Priority;
	};

	class ComputeCommandBuffer : public CommandBuffer
	{
	protected:
		ComputeCommandBuffer(CommandBufferPriority_ prior) : CommandBuffer(prior)
		{
		//	m_Type = CommandBufferType_::CommandBufferType_Compute;
		};
	};

	class GraphicsCommandBuffer : public CommandBuffer
	{
	protected:
		GraphicsCommandBuffer(CommandBufferPriority_ prior) : CommandBuffer(prior)
		{
		//	m_Type = CommandBufferType_::CommandBufferType_Graphics;
		};
	};
}