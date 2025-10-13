#pragma once
#include "Skateboard/Graphics/Resources/CommonResources.h"
#include "Skateboard/Graphics/RHI/GraphicsContext.h"

namespace Skateboard
{
	// Forward declarations
	class RasterizationPipeline;
	class ComputePipeline;
	class RaytracingPipeline;

	enum SKTBD_DSVClearMode
	{
		DEPTH = 1,
		STENCIL = 2,
	};
	ENUM_FLAG_OPERATORS(SKTBD_DSVClearMode)

	struct DispatchRaysDesc
	{
		ShaderTable* RaygenRecord;
		ShaderTable* MissTable;
		ShaderTable* HitGroupTable;
		ShaderTable* CallableTable;

		uint32_t Width;
		uint32_t Height;
		uint32_t Depth;
	};

	class RenderCommand
	{
		friend class Application;
	public:
		static void SetViewport(Viewport* n_viewports,uint32_t count, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetViewport_(n_viewports,count, cb);
		}

		static void SetScissor(Rect* n_rects, uint32_t count, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetScissor_(n_rects,count,cb);
		}

		static void BeginCommandBuffer(CommandBuffer* cb)																																				
		{
			Api->BeginCommandBuffer_(cb);
		}

		static void EndCommandBuffer(CommandBuffer* cb)																																					
		{
			Api->EndCommandBuffer_(cb);
		}

		static void Barrier(BarrierGroup* barriers, uint32_t barrier_count, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->Barrier_(barriers, barrier_count, cb);
		}


		static void SetInputLayoutGraphics(ShaderInputLayout* layout, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetInputLayoutGraphics_(layout, cb);
		}

		static void SetInputLayoutCompute(ShaderInputLayout* layout, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetInputLayoutCompute_(layout, cb);
		}

		static void SetInline32bitDataGraphics(uint32_t InputSlot, void* Data, uint32_t Size, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetInline32bitDataGraphics_(InputSlot, Data, Size, cb);
		}

		static void SetInline32bitDataCompute(uint32_t InputSlot, void* Data, uint32_t Size, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetInline32bitDataCompute_(InputSlot, Data, Size, cb);
		}

		static void SetInlineResourceViewGraphics(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ type , GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetInlineResourceViewGraphics_(InputSlot, Buffer, desc, type, cb);
		}

		static void SetInlineResourceViewCompute(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ type, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetInlineResourceViewCompute_(InputSlot,Buffer,desc, type, cb);
		}

		static void SetDescriptorTableCompute(uint32_t InputSlot, const DescriptorTable& table, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())										
		{
			Api->SetDescriptorTableCompute_(InputSlot, table, cb);
		}

		static void SetDescriptorTableGraphics(uint32_t InputSlot, const DescriptorTable& table, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetDescriptorTableGraphics_(InputSlot, table, cb);
		}

		static void SetPipelineState(Pipeline* pipeline, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())						
		{
			Api->SetPipelineState_(pipeline, cb);
		}

		static void SetPipelineState(ComputePipeline* pipeline, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetPipelineState_(pipeline, cb);
		}

		static void SetVertexBuffer(const VertexBufferView* vbviews, uint8_t numViews, uint8_t startSlot = 0, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetVertexBuffer_(vbviews, numViews,startSlot, cb);
		}

		static void SetPrimitiveTopology(SKTBD_PRIMITIVE_TOPOLOGY Topology, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetPrimitiveTopology_(Topology, cb);
		}

		static void SetIndexBuffer(const IndexBufferView* ibview, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetIndexBuffer_(ibview, cb);
		}

		static void SetRenderTargets(RenderTargetView* views, uint32_t numViews, DepthStencilView* DepthRenderTarget, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->SetRenderTargets_(views, numViews, DepthRenderTarget, cb);
		}

		static void ClearRenderTargets(RenderTargetView* views, uint32_t numViews, const float4& colour, Rect* rects = nullptr, uint32_t numRects = 0, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->ClearRenderTargets_(views, numViews, colour, rects, numRects, cb);
		}

		static void ClearDepthStencil(DepthStencilView* view, ClearValue clearV, SKTBD_DSVClearMode mode, Rect* rects, uint32_t numRects, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->ClearDepthStencil_(view, clearV, mode, rects, numRects, cb);
		}

		static void Dispatch(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())											
		{
			Api->Dispatch_(X_groups, Y_groups, Z_groups, cb);
		}

		static void DispatchRays(const DispatchRaysDesc& desc, CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->DispatchRays_(desc, cb);
		}

		static void DispatchMesh(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->DispatchMesh_(X_groups, Y_groups, Z_groups, cb);
		};

		static void Draw(uint32_t StartingVertex, uint32_t VertexCount, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->Draw_(StartingVertex, VertexCount, cb);
		}

		static void DrawInstanced(uint32_t StartingVertex, uint32_t VertexCount, uint32_t InstanceCount, uint32_t StartingInstance, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->DrawInstanced_(StartingVertex, VertexCount, InstanceCount, StartingInstance, cb);
		}

		static void DrawIndexed(uint32_t StratingVertex, uint32_t StartingIndex, uint32_t IndexCount, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->DrawIndexed_(StratingVertex, StartingIndex, IndexCount, cb);
		}

		static void DrawIndexedInstanced(uint32_t StratingVertex, uint32_t StartingIndex, uint32_t StartingInstance, uint32_t IndexCount, uint32_t InstanceCount, GraphicsCommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			Api->DrawIndexedInstanced_( StratingVertex,  StartingIndex,  StartingInstance,  IndexCount,  InstanceCount, cb);
		}

		static BottomLevelAccelerationStructure BuildBottomLevelAS(const BottomLevelAccelerationStructureDesc& Desc, const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const  uint64_t& Scratch_Offset = 0, const CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			return Api->BuildBottomLevelAS_(Desc, Flags, StorageBuffer, ScratchBuffer, Scratch_Offset, cb);
		}

		static TopLevelAccelerationStructure BuildTopLevelAS(const TopLevelAccelerationStructureDesc& Desc, const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const  uint64_t& Scratch_Offset = 0, const CommandBuffer* cb = GraphicsContext::GetDefaultCommandBuffer())
		{
			return Api->BuildTopLevelAS_(Desc, Flags, StorageBuffer, ScratchBuffer, Scratch_Offset, cb);
		}

	protected:

		virtual void SetViewport_(Viewport* Viewport, uint32_t count, GraphicsCommandBuffer* cb) = 0;
		virtual void SetScissor_(Rect* n_scissor, uint32_t count, GraphicsCommandBuffer* cb) = 0;

		// DRAW COMMANDS ///////////////////////////////////////////////////////////////////

		virtual void BeginCommandBuffer_(CommandBuffer* cb) = 0;
		virtual void EndCommandBuffer_(CommandBuffer* cb) = 0;
		
		virtual void Barrier_(BarrierGroup* barriers, uint32_t group_count, CommandBuffer* cb) = 0;

		virtual void SetInline32bitDataGraphics_(uint32_t InputSlot, void* Data, uint32_t Size, GraphicsCommandBuffer* cb) = 0;
		virtual void SetInline32bitDataCompute_(uint32_t InputSlot, void* Data, uint32_t Size, CommandBuffer* cb) = 0;

		virtual void SetInlineResourceViewGraphics_(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ Type, GraphicsCommandBuffer* cb) = 0;
		virtual void SetInlineResourceViewCompute_(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ Type, CommandBuffer* cb) = 0;

		virtual void SetDescriptorTableGraphics_(uint32_t InputSlot, const DescriptorTable& table, GraphicsCommandBuffer* cb) = 0;
		virtual void SetDescriptorTableCompute_(uint32_t InputSlot, const DescriptorTable& table, CommandBuffer* cb) = 0;

		virtual void SetInputLayoutCompute_(ShaderInputLayout* inputLayout, CommandBuffer* cb) = 0;
		virtual void SetInputLayoutGraphics_(ShaderInputLayout* inputLayout, GraphicsCommandBuffer* cb) = 0;

		virtual void SetPipelineState_(ComputePipeline* pipeline, CommandBuffer* cb) = 0;
		virtual void SetPipelineState_(Pipeline* pipeline, GraphicsCommandBuffer* dcb) = 0;

		virtual void SetVertexBuffer_(const VertexBufferView* vbviews, uint8_t numViews, uint8_t startSlot, GraphicsCommandBuffer* cb) = 0;
		virtual void SetPrimitiveTopology_(SKTBD_PRIMITIVE_TOPOLOGY topology, GraphicsCommandBuffer* cb) = 0;

		virtual void SetIndexBuffer_(const IndexBufferView* ibview, GraphicsCommandBuffer* cb) = 0;

		virtual void SetRenderTargets_(RenderTargetView* views, uint32_t numViews, DepthStencilView* DepthRenderTarget, GraphicsCommandBuffer* cb) = 0;
		virtual void ClearRenderTargets_(RenderTargetView* views, uint32_t numViews, const float4& colour, Rect* rects, uint32_t rect_count, CommandBuffer* cb) = 0;
		virtual void ClearDepthStencil_(DepthStencilView* view, ClearValue clearV, SKTBD_DSVClearMode mode, Rect* rects, uint32_t numRects, GraphicsCommandBuffer* cb) = 0;

		virtual void DispatchRays_(const DispatchRaysDesc& desc, CommandBuffer* cb) = 0;

		virtual void Dispatch_(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, CommandBuffer* cb) = 0;

		virtual void DispatchMesh_(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, GraphicsCommandBuffer* cb) = 0;

		virtual void Draw_(uint32_t StartingVertex, uint32_t VertexCount, GraphicsCommandBuffer* cb) = 0;
		virtual void DrawInstanced_(uint32_t StartingVertex, uint32_t VertexCount, uint32_t InstanceCount, uint32_t StartingInstance, GraphicsCommandBuffer* cb) = 0;
		virtual void DrawIndexed_(uint32_t StartingVertex, uint32_t StartingIndex, uint32_t IndexCount, GraphicsCommandBuffer* cb) = 0;
		virtual void DrawIndexedInstanced_(uint32_t StartingVertex, uint32_t StartingIndex, uint32_t StartingInstance, uint32_t IndexCount, uint32_t InstanceCount, GraphicsCommandBuffer* cb) = 0;

		//virtual void CopyBuffer() = 0;
		//virtual void CopyTexture() = 0;

		virtual BottomLevelAccelerationStructure BuildBottomLevelAS_(const BottomLevelAccelerationStructureDesc& Desc, const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const  uint64_t& Scratch_Offset, const CommandBuffer* cb) = 0;
		virtual TopLevelAccelerationStructure BuildTopLevelAS_(const TopLevelAccelerationStructureDesc& Desc, const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const  uint64_t& Scratch_Offset, const CommandBuffer* cb) = 0;

	private:
		static void RegisterRenderCommand(RenderCommand* PlatformApi) { Api = PlatformApi; };
		inline static RenderCommand* Api = nullptr;
	};

	
}
