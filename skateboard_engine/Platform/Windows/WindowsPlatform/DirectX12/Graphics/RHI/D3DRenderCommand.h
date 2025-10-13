#pragma once
#include "Skateboard/Graphics/RHI/RenderCommand.h"

namespace Skateboard
{
	class D3DGraphicsContext;
	class Scene;

	// <summary>
	class D3DRenderCommand final : public RenderCommand
	{
	public:

		virtual void SetViewport_(Viewport* Viewport, uint32_t count, GraphicsCommandBuffer* cb) final override;
		virtual void SetScissor_(Rect* n_scissor, uint32_t count, GraphicsCommandBuffer* cb) final override;

//		void ResizeBackBuffers(uint32_t width, uint32_t height) final override;
		virtual void DispatchRays_(const DispatchRaysDesc& desc, CommandBuffer* cb) override;
		virtual void Dispatch_(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, CommandBuffer* cb) final override;

		virtual void DispatchMesh_(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, GraphicsCommandBuffer* cb) final override;

		virtual void Draw_(uint32_t StartingVertex, uint32_t VertexCount, GraphicsCommandBuffer* cb) final override;
		virtual void DrawInstanced_(uint32_t StartingVertex, uint32_t VertexCount, uint32_t InstanceCount, uint32_t StartingInstance, GraphicsCommandBuffer* cb) final override;
		virtual void DrawIndexed_(uint32_t StratingVertex, uint32_t StartingIndex, uint32_t IndexCount, GraphicsCommandBuffer* cb) final override;
		virtual void DrawIndexedInstanced_(uint32_t StratingVertex, uint32_t StartingIndex, uint32_t StartingInstance, uint32_t IndexCount, uint32_t InstanceCount, GraphicsCommandBuffer* cb) final override;

		virtual void BeginCommandBuffer_(CommandBuffer* cb)																						final override;		
		virtual void EndCommandBuffer_(CommandBuffer* cb)																						final override;
		//virtual void SubmitCommandBuffers(CommandBuffer** cb, uint32_t count)																	final override;

		virtual void Barrier_(BarrierGroup* barriers, uint32_t group_count, CommandBuffer* cb) override;

		virtual void SetInline32bitDataGraphics_(uint32_t InputSlot, void* Data, uint32_t size, GraphicsCommandBuffer* cb)								final override;
		virtual void SetInline32bitDataCompute_(uint32_t InputSlot, void* Data, uint32_t size, CommandBuffer* cb)								final override;

		virtual void SetInlineResourceViewGraphics_(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ Type, GraphicsCommandBuffer* cb) final override;
		virtual void SetInlineResourceViewCompute_(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ Type, CommandBuffer* cb)  final override;

		virtual void SetDescriptorTableGraphics_(uint32_t InputSlot, const DescriptorTable& table, GraphicsCommandBuffer* cb)							final override;
		virtual void SetDescriptorTableCompute_(uint32_t InputSlot, const DescriptorTable& table, CommandBuffer* cb)								final override;
												
		virtual void SetInputLayoutCompute_(ShaderInputLayout* inputLayout, CommandBuffer* cb)													final override;
		virtual void SetInputLayoutGraphics_(ShaderInputLayout* inputLayout, GraphicsCommandBuffer* cb) 													final override;

		virtual void SetPipelineState_(ComputePipeline* pipeline, CommandBuffer* cb)																	final override;
		virtual void SetPipelineState_(Pipeline* pipeline, GraphicsCommandBuffer* cb)																	final override;

		virtual void SetVertexBuffer_(const VertexBufferView* vbview, uint8_t numViews, uint8_t startView, GraphicsCommandBuffer* cb)							final override;

		virtual void SetPrimitiveTopology_(SKTBD_PRIMITIVE_TOPOLOGY, GraphicsCommandBuffer* cb)															final override;


		virtual void SetIndexBuffer_(const IndexBufferView* ibview, GraphicsCommandBuffer* cb)																	final override;
		virtual void SetRenderTargets_(RenderTargetView* views, uint32_t numViews, DepthStencilView* DepthRenderTarget, GraphicsCommandBuffer* cb)		final override;
		virtual void ClearRenderTargets_(RenderTargetView* views, uint32_t numViews, const float4& colour, Rect* rects, uint32_t numRects, CommandBuffer* cb)		final override;

		virtual void ClearDepthStencil_(DepthStencilView* view, ClearValue clearV,SKTBD_DSVClearMode mode, Rect* rects, uint32_t numRects, GraphicsCommandBuffer* cb) final override;

		//virtual void WaitForCommandBufer(CommandBuffer* cb)																						final override;

		virtual BottomLevelAccelerationStructure BuildBottomLevelAS_(const BottomLevelAccelerationStructureDesc& Desc, const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer,const  uint64_t& Scratch_Offset, const CommandBuffer* cb) override;
		virtual TopLevelAccelerationStructure BuildTopLevelAS_(const TopLevelAccelerationStructureDesc& Desc, const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const uint64_t& Scratch_Offset, const CommandBuffer* cb) override;

	};
}