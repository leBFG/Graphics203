#pragma once
#include "AGC/Graphics/AGCF.h"
#include "Skateboard/Graphics/RHI/RenderingApi.h"

namespace Skateboard
{
	class AGCRenderingAPI final : public RenderingApi
	{
	public:
		void SetViewport(Viewport* Viewport, uint32_t count, GraphicsCommandBuffer* cb) override;
		void SetScissor(Rect* n_scissor, uint32_t count, GraphicsCommandBuffer* cb) override;

		void BeginCommandBuffer(CommandBuffer* cb) override;
		void EndCommandBuffer(CommandBuffer* cb) override;
		//void SubmitCommandBuffers(CommandBuffer**, uint32_t count) override;

		void Barrier(BarrierGroup* barriers, uint32_t group_count, CommandBuffer* cb) override;

		void SetInline32bitDataGraphics(uint32_t InputSlot, void* Data, uint32_t Size, GraphicsCommandBuffer* cb) override;
		void SetInline32bitDataCompute(uint32_t InputSlot, void* Data, uint32_t Size, CommandBuffer* cb) override;
		void SetInlineResourceViewGraphics(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc,ViewAccessType_ Type, GraphicsCommandBuffer* cb) override;
		void SetInlineResourceViewCompute(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ Type, CommandBuffer* cb) override;
		void SetDescriptorTableGraphics(uint32_t InputSlot, const DescriptorTable& table, GraphicsCommandBuffer* cb) override;
		void SetDescriptorTableCompute(uint32_t InputSlot, const DescriptorTable& table, CommandBuffer* cb) override;
		void SetInputLayoutGraphics(ShaderInputLayout* inputLayout, GraphicsCommandBuffer* cb) override;
		void SetInputLayoutCompute(ShaderInputLayout* inputLayout, CommandBuffer* cb) override;

		void SetPipelineState(Pipeline* pipeline, GraphicsCommandBuffer* cb) override;
		void SetPipelineState(ComputePipeline* pipeline, CommandBuffer* cb) override;

		void SetVertexBuffer(const VertexBufferView* vbviews, uint8_t numViews, uint8_t startSlot, GraphicsCommandBuffer* cb) override;
		void SetPrimitiveTopology(SKTBD_PRIMITIVE_TOPOLOGY topology, GraphicsCommandBuffer* cb) override;
		void SetIndexBuffer(const IndexBufferView* ibview, GraphicsCommandBuffer* cb) override;
		void SetRenderTargets(RenderTargetView* views, uint32_t numViews, DepthStencilView* DepthRenderTarget, GraphicsCommandBuffer* cb) override;
		void ClearRenderTargets(RenderTargetView* views, uint32_t numViews, const float4& colour, Rect* rects, uint32_t rect_count, CommandBuffer* cb) override;
		void ClearDepthStencil(DepthStencilView* view, ClearValue clearV, SKTBD_DSVClearMode mode, Rect* rects, uint32_t numRects, GraphicsCommandBuffer* cb) override;
		//void WaitForCommandBufer(CommandBuffer* cb) override;

		void DispatchRays(const DispatchRaysDesc& desc, CommandBuffer* cb) override;
		void Dispatch(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, CommandBuffer* cb) override;
		void Draw(uint32_t StartingVertex, uint32_t VertexCount, GraphicsCommandBuffer* cb) override;

		void DrawInstanced(uint32_t StartingVertex, uint32_t VertexCount, uint32_t InstanceCount, uint32_t StartingInstance, GraphicsCommandBuffer* cb) override;
		void DrawIndexed(uint32_t StartingVertex, uint32_t StartingIndex, uint32_t IndexCount, GraphicsCommandBuffer* cb) override;
		void DrawIndexedInstanced(uint32_t StartingVertex, uint32_t StartingIndex, uint32_t StartingInstance, uint32_t IndexCount, uint32_t InstanceCount, GraphicsCommandBuffer* cb) override;


		BottomLevelAccelerationStructure BuildBottomLevelAS(const BottomLevelAccelerationStructureDesc& Desc,  const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const uint64_t& Scratch_Offset, const CommandBuffer* cb) override;
		TopLevelAccelerationStructure BuildTopLevelAS(const TopLevelAccelerationStructureDesc& Desc,const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer,const Buffer* ScratchBuffer, const uint64_t& Scratch_Offset, const CommandBuffer* cb) override;
	};
}
