#include <sktbdpch.h>
#include "AGCRenderingApi.h"

#include "AGC/Graphics/Resources/AGCCommandBuffer.h"

#define SKTBD_LOG_COMPONENT "AgcRenderingApi"
#include "Skateboard/Log.h"

#include <bitset>

#include "AGC/Graphics/Resources/AGCBuffer.h"
#include "AGC/Graphics/Resources/AGCCommandBuffer.h"

namespace Skateboard
{
	constexpr AGCGraphicsCommandBuffer* GetCB(CommandBuffer* cb) { return static_cast<AGCGraphicsCommandBuffer*>(cb); }

	//BindResources When BindingsHave Been altered
	inline auto BindGraphicsResources(AGCGraphicsPipeline& state, AGCGraphicsCommandBuffer& dcb, uint32_t InstanceOffset, uint32_t VertexOffset)
	{
			if (state.m_StatesToSet.test(AGCPipelineStates::GS))
			{
				sce::Agc::Core::IndirectStageBinder BinderGS;
				BinderGS.init().setShader(state.m_VertexShader.pShader);

				//Bind SRT and Vertex/instance data

				auto mem = static_cast<sce::Agc::ShRegister*>(dcb.m_DCB.allocateTopDown(BinderGS.getUserDataSizeInBytes(), sce::Agc::Alignment::kRegister));
				BinderGS.setUserDataPointer(mem);

				if (BinderGS.getUserDataLayout()->m_srtSizeInDwords)
				{
					BinderGS.setUserSrtBuffer(dcb.m_RootGraphics->SRT_DATA.data(), dcb.m_RootGraphics->SRT_DATA.size());
				}

				if (state.m_VATable.GetAttributeCount() > 0)
				{
					BinderGS.setVertexAttributeTable(state.m_VATable.GetVertexAttributeTable());
				}

				if (dcb.m_VertexBufferDataMustBeRebound && (BinderGS.getVertexBufferTableSizeInBytes() > 0))
				{
					auto Vb_mem = static_cast<sce::Agc::Core::Buffer*>(dcb.m_DCB.allocateTopDown(BinderGS.getVertexBufferTableSizeInBytes(), sce::Agc::Alignment::kBuffer));
					BinderGS.setVertexBufferTable(Vb_mem);
					BinderGS.setVertexBuffers(0, BinderGS.getVertexBufferTableSizeInElements(), dcb.m_VertexBuffers.data());
					dcb.m_VertexBufferDataMustBeRebound = false;
				}

				//Have to handle it here as well
				
				if(sce::Agc::getVertexOffsetUserDataSlot(BinderGS.getShader()->m_specials->m_drawModifier) != sce::Agc::UserDataLayout::kIllegalDirectOffset)
				BinderGS.setVertexOffset(VertexOffset);

				if(InstanceOffset)
				BinderGS.setInstanceOffset(InstanceOffset);

				dcb.m_DCB.setShRegistersIndirect(mem, BinderGS.getUserDataSizeInElements());
			}

			if (state.m_StatesToSet.test(AGCPipelineStates::PS))
			{
				/*sce::Agc::Core::StageBinder BinderPs;
				BinderPs.init(&dcb.m_DCB, &dcb.m_DCB).setShader(state.m_PixelShader.pShader).setUserSrtBuffer(layout.SRT_DATA.data(), layout.SRT_DATA.size());*/

				sce::Agc::Core::IndirectStageBinder BinderPS;
				BinderPS.init().setShader(state.m_PixelShader.pShader);

				auto mem = static_cast<sce::Agc::ShRegister*>(dcb.m_DCB.allocateTopDown(BinderPS.getUserDataSizeInBytes(), sce::Agc::Alignment::kRegister));
				BinderPS.setUserDataPointer(mem);

				if (BinderPS.getUserDataLayout()->m_srtSizeInDwords)
				{
					BinderPS.setUserSrtBuffer(dcb.m_RootGraphics->SRT_DATA.data(), dcb.m_RootGraphics->SRT_DATA.size());
				}

				dcb.m_DCB.setShRegistersIndirect(mem, BinderPS.getUserDataSizeInElements());
			}
	}

	inline auto BindComputeResources(AGCComputePipeline & state, IAGCCommandBufferInterface & CommandBuffer) -> void
	{
		sce::Agc::Core::IndirectStageBinder BinderCS;
		BinderCS.init().setShader(state.m_ComputeShader.pShader);

		auto mem = static_cast<sce::Agc::ShRegister*>(CommandBuffer.GetCommandBuffer().allocateTopDown(BinderCS.getUserDataSizeInBytes(), sce::Agc::Alignment::kRegister));
		BinderCS.setUserDataPointer(mem);

		if (BinderCS.getUserDataLayout()->m_srtSizeInDwords)
		{
			const auto Layout = CommandBuffer.GetComputeLayout();

			BinderCS.setUserSrtBuffer(Layout->SRT_DATA.data(), Layout->SRT_DATA.size());
		}

		CommandBuffer.GetCommandBuffer().setShRegistersDirect(mem, BinderCS.getUserDataSizeInElements());
	}

	//max viewports, same as dx12 how to set all of them tho is unclear lol;
	#define SKTBD_PS5_VIEWPORT_COUNT 16 

	void AGCRenderingAPI::SetViewport(Viewport* Viewport, uint32_t count, GraphicsCommandBuffer* cb)
	{
		ASSERT(count < SKTBD_PS5_VIEWPORT_COUNT);

		sce::Agc::CxViewport viewport{};
		auto dcb = *GetCB(cb);

		for (uint i = 0; i < count; i++)
		{
			sce::Agc::Core::setViewport(&viewport, Viewport[i].Width, Viewport[i].Height, Viewport[i].TopLeftX, Viewport[i].TopLeftY, Viewport[i].MinZ, Viewport[i].MaxZ);
			viewport.setSlot(i);
			dcb.m_STB.setState(viewport);
		}
	}

	void AGCRenderingAPI::SetScissor(Rect* n_scissor, uint32_t count, GraphicsCommandBuffer* cb)
	{
		sce::Agc::CxScreenScissor screenScissor;

		screenScissor.init();
		auto dcb = *GetCB(cb);

		for (uint i = 0; i < count; count++)
		{
			//screenScissor.(i);
			dcb.m_STB.setState(screenScissor);
		}

		//sce::Agc::CxScissor scissor;
		//auto agccb = GetAGCCB(cb);
		//agccb.m_STB.setState(viewport);

		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
	}

	void AGCRenderingAPI::BeginCommandBuffer(CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
	}

	void AGCRenderingAPI::EndCommandBuffer(CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
	}

	//void AGCRenderingAPI::SubmitCommandBuffers(CommandBuffer** cb, uint32_t count)
	//{
	//	for(uint i = 0; i < count; i++)
	//	{
	//		//std::visit([](AGCGraphicsCommandBuffer& dcb) {SCE_AGC_ASSERT(sce::Agc::submitGraphics(sce::Agc::GraphicsQueue::kNormal, dcb.m_DCB.getSubmitPointer(), GetDCB(cb[i]).m_DCB.getSubmitSize())); });

	//		switch (cb[i]->GetType())
	//		{
	//		case CommandBufferType_Graphics:
	//			SCE_AGC_ASSERT(sce::Agc::submitGraphics(sce::Agc::GraphicsQueue::kNormal, GetDCB(cb[i]).m_DCB.getSubmitPointer(), GetDCB(cb[i]).m_DCB.getSubmitSize()));
	//			break;
	//		/*case CommandBufferType_Compute:
	//			sce::Agc::AsyncComputeQueue queue;
	//			queue.m_pipe = 0;
	//			queue.m_queue = 0;
	//			SCE_AGC_ASSERT(sce::Agc::submitAsyncCompute(queue, GetACB(cb[i]).m_ACB.getSubmitPointer(), GetDCB(cb[i]).m_DCB.getSubmitSize()));
	//			break;*/
	//		}
	//		
	//	}
	//}

	void AGCRenderingAPI::Barrier(BarrierGroup* barriers, uint32_t group_count, CommandBuffer* cb)
	{
		//Brute force Sync all
		sce::Agc::Core::gpuSyncEvent(&GetCB(cb)->m_DCB, sce::Agc::Core::SyncWaitMode::kDrainGraphicsAndCompute, sce::Agc::Core::SyncCacheOp::kClearAll);
	}

	void AGCRenderingAPI::SetInline32bitDataGraphics(uint32_t InputSlot, void* Data, uint32_t Size, GraphicsCommandBuffer* cb)
	{
		auto& dcb = *GetCB(cb);
		auto sig = static_cast<AGCShaderInputLayout*>(GetCB(cb)->m_RootGraphics);

		memcpy(&sig->SRT_DATA[sig->SRT_OFFSETS_IN_BYTES[InputSlot]], Data, Size);
		dcb.m_GraphicsResourcesMustBeRebound = true;
	}

	void AGCRenderingAPI::SetInline32bitDataCompute(uint32_t InputSlot, void* Data, uint32_t Size, CommandBuffer* cb)
	{
		auto dcb = dynamic_cast<IAGCCommandBufferInterface*>(cb);
		auto sig = dcb->GetGraphicsLayout();

		memcpy(&sig->SRT_DATA[sig->SRT_OFFSETS_IN_BYTES[InputSlot]], Data, Size);
		//dcb.m_ComputeResourcesMustBeRebound = true;
	}

	void AGCRenderingAPI::SetInlineResourceViewGraphics(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc,
		ViewAccessType_ Type, GraphicsCommandBuffer* cb)
	{
		auto& dcb = *GetCB(cb);
		auto sig = static_cast<AGCShaderInputLayout*>(GetCB(cb)->m_RootGraphics);
		auto buf = static_cast<AGCBuffer*>(Buffer);

		sce::Agc::Core::BufferSpec DataSpec;
		sce::Agc::Core::Buffer Data;

		switch (desc.Type)
		{
		case BufferType_ConstantBuffer:
			DataSpec.initAsConstantBuffer((uint8_t*)buf->BufferMemory.m_Data + desc.Offset, desc.ElementSize);
			break;
		case BufferType_ByteBuffer:
			DataSpec.initAsByteBuffer((uint8_t*)buf->BufferMemory.m_Data + desc.Offset, desc.ElementCount);
			break;
		case BufferType_StructureBuffer:
			DataSpec.initAsRegularBuffer((uint8_t*)buf->BufferMemory.m_Data + desc.Offset, desc.ElementSize, desc.ElementCount);
			break;
		case BufferType_FormattedBuffer:
			DataSpec.initAsDataBuffer((uint8_t*)buf->BufferMemory.m_Data + desc.Offset, SkateboardDataFormatToAGC(desc.Format), desc.ElementCount);
			SKTBD_LOG_WARN("RendeirngAPI", "DX12 doesnt support Formatted descriptors in root, AGC does");
			break;
		}

		SCE_AGC_ASSERT(sce::Agc::Core::initialize(&Data, &DataSpec)==SCE_OK);

		memcpy(&sig->SRT_DATA[sig->SRT_OFFSETS_IN_BYTES[InputSlot]], &Data, sizeof(sce::Agc::Core::Buffer));
		dcb.m_GraphicsResourcesMustBeRebound = true;
	}

	void AGCRenderingAPI::SetInlineResourceViewCompute(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc,
		ViewAccessType_ Type, CommandBuffer* cb)
	{
		auto& dcb = *GetCB(cb);
		auto sig = static_cast<AGCShaderInputLayout*>(GetCB(cb)->m_RootCompute);
		auto buf = static_cast<AGCBuffer*>(Buffer);

		sce::Agc::Core::BufferSpec DataSpec;
		sce::Agc::Core::Buffer Data;

		switch (desc.Type)
		{
		case BufferType_ConstantBuffer:
			DataSpec.initAsConstantBuffer((uint8_t*)buf->BufferMemory.m_Data + desc.Offset, desc.ElementSize);
			break;
		case BufferType_ByteBuffer:
			DataSpec.initAsByteBuffer((uint8_t*)buf->BufferMemory.m_Data + desc.Offset, desc.ElementCount);
			break;
		case BufferType_StructureBuffer:
			DataSpec.initAsRegularBuffer((uint8_t*)buf->BufferMemory.m_Data + desc.Offset, desc.ElementSize, desc.ElementCount);
			break;
		case BufferType_FormattedBuffer:
			DataSpec.initAsDataBuffer((uint8_t*)buf->BufferMemory.m_Data + desc.Offset, SkateboardDataFormatToAGC(desc.Format), desc.ElementCount);
			SKTBD_LOG_WARN("RendeirngAPI", "DX12 doesnt support Formatted descriptors in root, AGC does");
			break;
		}

		SCE_AGC_ASSERT(sce::Agc::Core::initialize(&Data, &DataSpec) == SCE_OK);

		memcpy(&sig->SRT_DATA[sig->SRT_OFFSETS_IN_BYTES[InputSlot]], &Data, sizeof(sce::Agc::Core::Buffer));
		dcb.m_GraphicsResourcesMustBeRebound = true;
	}

	void AGCRenderingAPI::SetDescriptorTableGraphics(uint32_t InputSlot, const DescriptorTable& table,
		GraphicsCommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
	}

	void AGCRenderingAPI::SetDescriptorTableCompute(uint32_t InputSlot, const DescriptorTable& table, CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
	}

	void AGCRenderingAPI::SetInputLayoutCompute(ShaderInputLayout* inputLayout, CommandBuffer* cb)
	{
		GetCB(cb)->SetShaderInputCompute(inputLayout);
	}

	void AGCRenderingAPI::SetInputLayoutGraphics(ShaderInputLayout* inputLayout, GraphicsCommandBuffer* cb)
	{
		GetCB(cb)->SetShaderInputGraphics(inputLayout);
	}

	void AGCRenderingAPI::SetPipelineState(Pipeline* pipeline, GraphicsCommandBuffer* cb)
	{
		auto agccb = GetCB(cb);

		agccb->SetCurrentPipeline(pipeline);
	}

	void AGCRenderingAPI::SetPipelineState(ComputePipeline* pipeline, CommandBuffer* cb)
	{
		auto CommandBuffer = dynamic_cast<IAGCCommandBufferInterface*>(cb);
		CommandBuffer->SetCurrentPipeline(pipeline);
	}

	void AGCRenderingAPI::SetVertexBuffer(const VertexBufferView* vbviews, uint8_t numViews, uint8_t startSlot, GraphicsCommandBuffer* cb)
	{
		auto agccb = GetCB(cb);
		agccb->m_VertexBufferDataMustBeRebound = true;

		SKTBD_LOG_ASSERT(numViews+startSlot < SKTBD_MAX_VERTEX_BUFFER_COUNT, "AGCRenderingApi", "max supported vertex buffer count is 32, you are attempting to change VBS outside of this range")

		for (uint i = startSlot; i < numViews; i++)
		{
			SCE_AGC_ASSERT(sce::Agc::Core::initializeVertexBuffer(
				&agccb->m_VertexBuffers[i],
				static_cast<uint8_t*>(static_cast<AGCBuffer*>(vbviews[i].m_ParentResource.get())->BufferMemory.m_Data) + vbviews[i].m_Offset,
				sce::Agc::Gnmp::Extras::kDataFormatR32G32B32A32Float,
				vbviews[i].m_VertexStride,
				vbviews[i].m_VertexCount
			) == SCE_OK);
		}

		//GS/HS Binder
	}

	void AGCRenderingAPI::SetPrimitiveTopology(SKTBD_PRIMITIVE_TOPOLOGY topology, GraphicsCommandBuffer* cb)
	{
		//SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");

		// Shader Linkage update, requires change to CX and Uc states thus a context roll
	}

	void AGCRenderingAPI::SetIndexBuffer(const IndexBufferView* ibview, GraphicsCommandBuffer* cb)
	{
		auto IB_Address = static_cast<uint8_t*>(static_cast<AGCBuffer*>(ibview->m_ParentResource.get())->BufferMemory.m_Data) + ibview->m_Offset;

		GetCB(cb)->m_DCB.setIndexBuffer(IB_Address);
		GetCB(cb)->m_DCB.setIndexSize((ibview->m_Format == IndexFormat::bit32) ? sce::Agc::IndexSize::k32 : (ibview->m_Format == IndexFormat::bit16) ? sce::Agc::IndexSize::k16 : sce::Agc::IndexSize::k8);
	}

	void AGCRenderingAPI::SetRenderTargets(RenderTargetView* views, uint32_t numViews,
		DepthStencilView* DepthRenderTarget, GraphicsCommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
	}

	void AGCRenderingAPI::ClearRenderTargets(RenderTargetView* views, uint32_t numViews, const float4& colour,
		Rect* rects, uint32_t rect_count, CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
		//	sce::Agc::Toolkit::clearRenderTargetCs()
	}

	void AGCRenderingAPI::ClearDepthStencil(DepthStencilView* view, ClearValue clearV, SKTBD_DSVClearMode mode,
		Rect* rects, uint32_t numRects, GraphicsCommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
		//sce::Agc::Toolkit::clearDepthRenderTargetCs()
	}

	/*void AGCRenderingAPI::WaitForCommandBufer(CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
	}*/

	void AGCRenderingAPI::DispatchRays(const DispatchRaysDesc& desc, CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
	}

	void AGCRenderingAPI::Dispatch(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, CommandBuffer* cb)
	{
		//SKTBD_LOG_ASSERT(false, "RendeirngAPI", "UNIMPLEMENTED");
		auto& dcb = *GetCB(cb);

		SKTBD_LOG_ASSERT(dcb.m_CurrentPipeline->GetType() == PipelineType_Compute, "Current PipelineState Must be Compute");

		SKTBD_ASSERT(dcb.m_CurrentPipeline->GetType() == PipelineType_Compute)
		dcb.m_DCB.dispatch(X_groups, Y_groups, Z_groups, static_cast<AGCComputePipeline*>(dcb.m_CurrentPipeline)->m_ComputeShader.pShader->m_specials->m_dispatchModifier);

	}

	void AGCRenderingAPI::Draw(uint32_t StartingVertex, uint32_t VertexCount, GraphicsCommandBuffer* cb)
	{
		DrawInstanced(StartingVertex, VertexCount, 1, 0, cb);
	}

	void AGCRenderingAPI::DrawInstanced(uint32_t StartingVertex, uint32_t VertexCount, uint32_t InstanceCount,
		uint32_t StartingInstance, GraphicsCommandBuffer* cb)
	{
		auto CL = *GetCB(cb);

		SKTBD_LOG_ASSERT(CL.m_CurrentPipeline->GetType() == PipelineType_Graphics, "Pipeline type is not graphics type");

		auto pipeline = static_cast<AGCGraphicsPipeline*>(CL.m_CurrentPipeline);

		BindGraphicsResources(*pipeline, CL, StartingInstance, StartingVertex);

		CL.m_DCB.setNumInstances(InstanceCount);
		CL.m_DCB.drawIndexAuto(VertexCount, pipeline->m_VertexShader.pShader->m_specials->m_drawModifier);
		CL.m_STB.postDraw();
	}

	void AGCRenderingAPI::DrawIndexed(uint32_t StartingVertex, uint32_t StartingIndex, uint32_t IndexCount,
		GraphicsCommandBuffer* cb)
	{
		DrawIndexedInstanced(StartingVertex, StartingIndex, 0, IndexCount, 1, cb);
	}

	void AGCRenderingAPI::DrawIndexedInstanced(uint32_t StartingVertex, uint32_t StartingIndex,
		uint32_t StartingInstance, uint32_t IndexCount, uint32_t InstanceCount, GraphicsCommandBuffer* cb)
	{
		auto& CL = *GetCB(cb);

		SKTBD_LOG_ASSERT(CL.m_CurrentPipeline->GetType() == PipelineType_Graphics, "Pipeline type is not graphics type");

		auto& pipeline = *static_cast<AGCGraphicsPipeline*>(CL.m_CurrentPipeline);

		BindGraphicsResources(pipeline, CL, StartingInstance, StartingVertex);

		//stage binder is responsible for index off
		CL.m_DCB.setNumInstances(InstanceCount);
		CL.m_DCB.drawIndexOffset(StartingIndex, IndexCount, pipeline.m_VertexShader.pShader->m_specials->m_drawModifier);
		CL.m_STB.postDraw();
	}

	BottomLevelAccelerationStructure AGCRenderingAPI::BuildBottomLevelAS(
		const BottomLevelAccelerationStructureDesc& Desc, const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags,
		const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const uint64_t& Scratch_Offset,
		const CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "UNIMPLEMENTED");
		return BottomLevelAccelerationStructure();

		//sce::Psr::Gpu::buildBottomLevelBvh()
		//sce::Psr::cpu::buildBottomLevelBvh()
	}

	TopLevelAccelerationStructure AGCRenderingAPI::BuildTopLevelAS(const TopLevelAccelerationStructureDesc& Desc,
		const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer,
		const uint64_t& Scratch_Offset, const CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "UNIMPLEMENTED");
		return TopLevelAccelerationStructure();

		//sce::Psr::GPU::buildBottomLevelBvh()
		//sce::Psr::cpu::buildBottomLevelBvh()
	}
}
