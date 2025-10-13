#include "sktbdpch.h"
#include "D3DRenderCommand.h"

//#include "D3DGraphicsContext.h"
#include "Graphics/Resources/D3DBuffer.h"
#include "Graphics/Resources/D3DCommandBuffer.h"
#include "Graphics/Resources/D3DPipeline.h"
#include "Graphics/Resources/D3DView.h"

namespace Skateboard
{

	void D3DRenderCommand::SetViewport_(Viewport* viewports, uint32_t count, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);
		std::array<D3D12_VIEWPORT, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE> VIEWPORTs;
		std::transform(&viewports[0], &viewports[count], VIEWPORTs.begin(), [](Viewport port) ->D3D12_VIEWPORT { return {  port.TopLeftX,port.TopLeftY,port.Width,port.Height,port.MinZ,port.MaxZ }; });
		handle->m_CommandList->RSSetViewports(count, VIEWPORTs.data());
	}

	void D3DRenderCommand::SetScissor_(Rect* n_scissor, uint32_t count, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);
		std::array<D3D12_RECT, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE> rects;
		std::transform(&n_scissor[0], &n_scissor[count], rects.begin(), [](Rect rec) ->D3D12_RECT { return { rec.left, rec.top, rec.right, rec.bottom }; });
		handle->m_CommandList->RSSetScissorRects(count, rects.data());
	}

	void D3DRenderCommand::DispatchRays_(const DispatchRaysDesc& desc, CommandBuffer* cb)
	{
		D3D12_DISPATCH_RAYS_DESC Dispatch{};
		Dispatch.Depth = desc.Depth;
		Dispatch.Width = desc.Width;
		Dispatch.Height = desc.Height;

		Dispatch.RayGenerationShaderRecord = (desc.RaygenRecord) ?  D3D12_GPU_VIRTUAL_ADDRESS_RANGE{ desc.RaygenRecord->m_Address, desc.RaygenRecord->Size } : D3D12_GPU_VIRTUAL_ADDRESS_RANGE();
		Dispatch.CallableShaderTable = (desc.CallableTable) ?		D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE{ desc.CallableTable->m_Address ,desc.CallableTable->Size,desc.CallableTable->Stride } : D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE();
		Dispatch.MissShaderTable = (desc.MissTable) ?				D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE{ desc.MissTable->m_Address ,desc.MissTable->Size,desc.MissTable->Stride } : D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE();
		Dispatch.HitGroupTable = (desc.HitGroupTable) ?				D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE{ desc.HitGroupTable->m_Address ,desc.HitGroupTable->Size,desc.HitGroupTable->Stride } : D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE();

		auto const commandList = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();
		commandList->DispatchRays(&Dispatch);
	}

	void D3DRenderCommand::Dispatch_(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups, CommandBuffer* cb)
	{
		// Validate the graphics command list before recording any further instructions.
		auto const commandList = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();
		SKTBD_LOG_ASSERT(commandList, "Null command list!");

		// Adds a dispatch api call to the graphics command list.
		// Please note, the kernel does not execute at this stage. Use the ExecuteCommandList() api appropriately to do so.
		commandList->Dispatch(X_groups, Y_groups, Z_groups);
	}

	void D3DRenderCommand::DispatchMesh_(uint32_t X_groups, uint32_t Y_groups, uint32_t Z_groups,
		GraphicsCommandBuffer* cb)
	{
		static_cast<D3DGraphicsCommandBuffer*>(cb)->GetCommandList()->DispatchMesh(X_groups, Y_groups, Z_groups);
	}

	void D3DRenderCommand::Draw_(uint32_t StartingVertex, uint32_t VertexCount, GraphicsCommandBuffer* cb)
	{
		DrawInstanced_(StartingVertex, VertexCount, 1, 0, cb);
	}

	void D3DRenderCommand::DrawInstanced_(uint32_t StartingVertex, uint32_t VertexCount, uint32_t InstanceCount, uint32_t StartingInstance, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);
		handle->m_CommandList->DrawInstanced(VertexCount, InstanceCount, StartingVertex, StartingInstance);
	}

	void D3DRenderCommand::DrawIndexed_(uint32_t StratingVertex, uint32_t StartingIndex, uint32_t IndexCount, GraphicsCommandBuffer* cb)
	{
		DrawIndexedInstanced_(StratingVertex, StartingIndex, 0, IndexCount, 1, cb);
	}

	void D3DRenderCommand::DrawIndexedInstanced_(uint32_t StratingVertex, uint32_t StartingIndex, uint32_t StartingInstance, uint32_t IndexCount, uint32_t InstanceCount, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);
		handle->m_CommandList->DrawIndexedInstanced(IndexCount, InstanceCount, StartingIndex, StratingVertex, StartingInstance);
	}

	void D3DRenderCommand::BeginCommandBuffer_(CommandBuffer* cb)
	{
		auto& handles = *dynamic_cast<ID3DCommandBufferInterface*>(cb);

		handles.GetCommandAllocator()->Reset();
		handles.GetCommandList()->Reset(handles.GetCommandAllocator(), nullptr);
	}

	void D3DRenderCommand::EndCommandBuffer_(CommandBuffer* cb)
	{
		auto& handles = *dynamic_cast<ID3DCommandBufferInterface*>(cb);
		handles.GetCommandList()->Close();
	}

	void D3DRenderCommand::Barrier_(BarrierGroup* barriers, uint32_t group_count, CommandBuffer* cb)
	{
		auto handle = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();

		std::vector<D3D12_GLOBAL_BARRIER> Global;
		std::vector<D3D12_TEXTURE_BARRIER> Texture;
		std::vector<D3D12_BUFFER_BARRIER> Buffer;

		for (uint32_t i = 0; i < group_count; i++)
		{
			auto group = barriers[i];

			switch (group.m_Type)
			{
			case GLOBAL_BARRIER:
				for (uint32_t j = 0; j < group.m_barrierCount; j++)
				{
					auto bar = std::get<GlobalBarrier*>(group.m_Barriers)[j];
					Global.push_back(D3D12_GLOBAL_BARRIER((D3D12_BARRIER_SYNC)bar.SyncBefore, (D3D12_BARRIER_SYNC)bar.SyncAfter, (D3D12_BARRIER_ACCESS)bar.AccessBefore, (D3D12_BARRIER_ACCESS)bar.AccessAfter));
				}

				break;
			case TEXTURE_BARRIER:
				for (uint32_t j = 0; j < group.m_barrierCount; j++)
				{
					auto bar = std::get<TextureBarrier*>(group.m_Barriers)[j];
					Texture.push_back(D3D12_TEXTURE_BARRIER(
						(D3D12_BARRIER_SYNC)bar.SyncBefore,
						(D3D12_BARRIER_SYNC)bar.SyncAfter,
						(D3D12_BARRIER_ACCESS)bar.AccessBefore,
						(D3D12_BARRIER_ACCESS)bar.AccessAfter,
						(D3D12_BARRIER_LAYOUT)bar.LayoutBefore,
						(D3D12_BARRIER_LAYOUT)bar.LayoutAfter,
						static_cast<D3DTextureBuffer*>(bar.Resource)->m_TextureResource->GetResource(),
						{ bar.SubresourceRange.IndexOrFirstMipLevel,bar.SubresourceRange.NumMipLevels,bar.SubresourceRange.FirstArraySlice,bar.SubresourceRange.NumArraySlices,bar.SubresourceRange.FirstPlane,bar.SubresourceRange.NumPlanes },
						(D3D12_TEXTURE_BARRIER_FLAGS)bar.Flags));
				}
				break;
			case BUFFER_BARRIER:
				for (uint32_t j = 0; j < group.m_barrierCount; j++)
				{
					auto bar = std::get<BufferBarrier*>(group.m_Barriers)[j];
					Buffer.push_back(D3D12_BUFFER_BARRIER((D3D12_BARRIER_SYNC)bar.SyncBefore, (D3D12_BARRIER_SYNC)bar.SyncAfter, (D3D12_BARRIER_ACCESS)bar.AccessBefore, (D3D12_BARRIER_ACCESS)bar.AccessAfter, static_cast<D3DBuffer*>(bar.Resource)->m_BufferResource->GetResource(), 0, UINT64_MAX));
				}
				break;
			}
		}

		std::vector<D3D12_BARRIER_GROUP> groups;
		if (!Global.empty()) groups.push_back(D3D12_BARRIER_GROUP(D3D12_BARRIER_TYPE_GLOBAL, static_cast<uint32_t>(Global.size()), {.pGlobalBarriers = Global.data()}));
		if (!Buffer.empty()) groups.push_back(D3D12_BARRIER_GROUP(D3D12_BARRIER_TYPE_BUFFER, static_cast<uint32_t>(Buffer.size()), {.pBufferBarriers = Buffer.data()}));
		if (!Texture.empty()) groups.push_back(D3D12_BARRIER_GROUP(D3D12_BARRIER_TYPE_TEXTURE, static_cast<uint32_t>(Texture.size()), {.pTextureBarriers = Texture.data()}));

		handle->Barrier(group_count, groups.data());
	}

	void D3DRenderCommand::SetInline32bitDataCompute_(uint32_t InputSlot, void* Data, uint32_t size, CommandBuffer* cb)
	{
		dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList()->SetComputeRoot32BitConstants(InputSlot, size, Data, 0);
	}

	void D3DRenderCommand::SetInline32bitDataGraphics_(uint32_t InputSlot, void* Data, uint32_t size, GraphicsCommandBuffer* cb)
	{
		static_cast<D3DGraphicsCommandBuffer*>(cb)->m_CommandList->SetGraphicsRoot32BitConstants(InputSlot, size, Data, 0);
	}

	void D3DRenderCommand::SetInlineResourceViewGraphics_(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ Type, GraphicsCommandBuffer* cb)
	{
		auto handle = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();
		auto addr = static_cast<D3DBuffer*>(Buffer)->m_BufferResource->GetResource()->GetGPUVirtualAddress();

		SKTBD_LOG_ASSERT(desc.Type != BufferType_FormattedBuffer , "DX12 Doesnt Support Formatted Inline Buffers, Use Descriptor Heap Based Binding (Bindless or Descriptor Tables)")
		SKTBD_LOG_ASSERT(!(desc.Type == BufferType_StructureBuffer && desc.Flags & BufferViewFlags_AppendConsumeBuffer), "DX12 Doesnt Support AppendConsume Inline Buffers, Use Descriptor Heap Based Binding (Bindless or Descriptor Tables)")

		switch (Type)
		{
			case ViewAccessType_GpuRead			:
				handle->SetGraphicsRootShaderResourceView(InputSlot, addr + desc.Offset);
				break;
			case ViewAccessType_GpuReadWrite	:
				handle->SetGraphicsRootUnorderedAccessView(InputSlot, addr + desc.Offset);
				break;
			case ViewAccessType_ConstantBuffer	:
				handle->SetGraphicsRootConstantBufferView(InputSlot, addr + desc.Offset);
				break;
		}
	}

	void D3DRenderCommand::SetInlineResourceViewCompute_(uint32_t InputSlot, Buffer* Buffer, BufferViewDesc desc, ViewAccessType_ Type, CommandBuffer* cb)
	{
		auto handle = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();
		auto addr = static_cast<D3DBuffer*>(Buffer)->m_BufferResource->GetResource()->GetGPUVirtualAddress();

		SKTBD_LOG_ASSERT(desc.Type != BufferType_FormattedBuffer, "DX12 Doesnt Support Formatted Inline Buffers, Use Descriptor Heap Based Binding (Bindless or Descriptor Tables)")
		SKTBD_LOG_ASSERT(!(desc.Type == BufferType_StructureBuffer && desc.Flags & BufferViewFlags_AppendConsumeBuffer), "DX12 Doesnt Support AppendConsume Inline Buffers, Use Descriptor Heap Based Binding (Bindless or Descriptor Tables)")

		switch (Type)
		{
		case ViewAccessType_GpuRead:
			handle->SetComputeRootShaderResourceView(InputSlot, addr + desc.Offset);
			break;
		case ViewAccessType_GpuReadWrite:
			handle->SetComputeRootUnorderedAccessView(InputSlot, addr + desc.Offset);
			break;
		case ViewAccessType_ConstantBuffer:
			handle->SetComputeRootConstantBufferView(InputSlot, addr + desc.Offset);
			break;
		}
	}

	void D3DRenderCommand::SetDescriptorTableGraphics_(uint32_t InputSlot, const DescriptorTable& table, GraphicsCommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "DESCRIPTOR TABLES UNSUPPORTED")
			auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);
		//	handle->m_Commandlist->SetGraphicsRootDescriptorTable(InputSlot);
	}
	
	void D3DRenderCommand::SetDescriptorTableCompute_(uint32_t InputSlot, const DescriptorTable& table, CommandBuffer* cb)
	{
		SKTBD_LOG_ASSERT(false, "DESCRIPTOR TABLES UNSUPPORTED")
			auto handle = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();
		//	handle->m_Commandlist->SetComputeRootDescriptorTable(InputSlot, static_cast<D3DDescriptorHandle*>(view.GetResource())->GetGPUPointer());
	}

	void D3DRenderCommand::SetInputLayoutCompute_(ShaderInputLayout* inputLayout, CommandBuffer* cb)
	{
		auto handle = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();
		handle->SetComputeRootSignature((inputLayout) ? static_cast<D3DShaderInputLayout*>(inputLayout)->m_RootSig.Get() : nullptr);
	}

	void D3DRenderCommand::SetInputLayoutGraphics_(ShaderInputLayout* inputLayout, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb)->GetCommandList();
		handle->SetGraphicsRootSignature((inputLayout) ? static_cast<D3DShaderInputLayout*>(inputLayout)->m_RootSig.Get() : nullptr);
	}

	void D3DRenderCommand::SetPipelineState_(ComputePipeline* pipeline, CommandBuffer* cb)
	{
		auto handle = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();
		const auto& state = static_cast<D3DComputePipeline*>(pipeline)->m_State.Get();
		handle->SetPipelineState1(state);
	}

	void D3DRenderCommand::SetPipelineState_(Pipeline* pipeline, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb)->GetCommandList();
		const auto state = dynamic_cast<ID3DPipelineInterface*>(pipeline);
		handle->SetProgram(state->GetProgram());

	}

	void D3DRenderCommand::SetVertexBuffer_(const VertexBufferView* vbview, uint8_t numViews, uint8_t startView, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);
		std::vector<D3D12_VERTEX_BUFFER_VIEW> Views(numViews);

		std::transform(vbview, &vbview[numViews], Views.begin(), [](const VertexBufferView& vb) -> D3D12_VERTEX_BUFFER_VIEW
		{
				D3D12_VERTEX_BUFFER_VIEW vview = {};
				vview.BufferLocation = static_cast<D3DBuffer*>(vb.m_ParentResource.get())->m_BufferResource->GetResource()->GetGPUVirtualAddress() + vb.m_Offset;
				vview.SizeInBytes = vb.m_VertexStride * vb.m_VertexCount;
				vview.StrideInBytes = vb.m_VertexStride;
				return vview;
		});

		handle->m_CommandList->IASetVertexBuffers(startView,numViews,Views.data());
	}

	void D3DRenderCommand::SetPrimitiveTopology_(SKTBD_PRIMITIVE_TOPOLOGY topology, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);

		handle->m_CommandList->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)topology);
	}

	DXGI_FORMAT SelectIndexFormat(const IndexFormat Format)
	{
		switch(Format)
		{
		default:
		case bit32: return DXGI_FORMAT_R32_UINT;
		case bit16:	return DXGI_FORMAT_R16_UINT;
		case bit8:	return DXGI_FORMAT_R8_UINT ;
		}
	}

	void D3DRenderCommand::SetIndexBuffer_(const IndexBufferView* ibview, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);

		if (ibview)
		{
			D3D12_INDEX_BUFFER_VIEW view = {};
			view.BufferLocation = static_cast<D3DBuffer*>(ibview->m_ParentResource.get())->m_BufferResource->GetResource()->GetGPUVirtualAddress() + ibview->m_Offset;
			view.Format = SelectIndexFormat(ibview->m_Format);
			view.SizeInBytes = ((ibview->m_Format == bit32) ? 4 : 2) * ibview->m_IndexCount;

			handle->m_CommandList->IASetIndexBuffer(&view);
		}
		else
			handle->m_CommandList->IASetIndexBuffer(nullptr);

	}

	void D3DRenderCommand::SetRenderTargets_(RenderTargetView* views, uint32_t numViews, DepthStencilView* DepthRenderTarget, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb);
		D3D12_CPU_DESCRIPTOR_HANDLE* RTS = nullptr;
		if (views)
		{
			std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargets;
			std::transform(&views[0], &views[numViews], RenderTargets.begin(), [](RenderTargetView& view) ->D3D12_CPU_DESCRIPTOR_HANDLE { return static_cast<D3DRenderTargetView*>(&view)->m_Descriptor.GetCPUHandle(); });
			RTS = RenderTargets.data();
		}

		handle->m_CommandList->OMSetRenderTargets(numViews,RTS,false,(DepthRenderTarget) ? &static_cast<D3DDepthStencilView*>(DepthRenderTarget)->m_Descriptor.GetCPUHandle() : nullptr);
	}

	void D3DRenderCommand::ClearRenderTargets_(RenderTargetView* views, uint32_t numViews,const float4& colour, Rect* rects, uint32_t numRects, CommandBuffer* cb)
	{
		auto handle = dynamic_cast<ID3DCommandBufferInterface*>(cb)->GetCommandList();

		if(rects)
		{
			std::vector<D3D12_RECT> r(numRects);
			std::transform(&rects[0], &rects[numRects], r.begin(), [](Rect rec) ->D3D12_RECT { return { rec.left, rec.top, rec.right, rec.bottom }; });

			for(uint32_t i = 0; i < numViews ; i++)
			{
				handle->ClearRenderTargetView(static_cast<D3DRenderTargetView*>(&views[i])->m_Descriptor.GetCPUHandle(), (FLOAT*)&colour, numRects, r.data());
			}
		}
		else
		{
			for (uint32_t i = 0; i < numViews; i++)
			{
				handle->ClearRenderTargetView(static_cast<D3DRenderTargetView*>(&views[i])->m_Descriptor.GetCPUHandle(), (FLOAT*)&colour, 0, nullptr);
				
			}
		}
	}

	void D3DRenderCommand::ClearDepthStencil_(DepthStencilView* view, ClearValue clearV, SKTBD_DSVClearMode mode,Rect* rects, uint32_t numRects, GraphicsCommandBuffer* cb)
	{
		auto handle = static_cast<D3DGraphicsCommandBuffer*>(cb)->m_CommandList;

		if (rects)
		{
			std::vector<D3D12_RECT> r(numRects);
			std::transform(&rects[0], &rects[numRects], r.begin(), [](Rect rec) ->D3D12_RECT { return { rec.left, rec.top, rec.right, rec.bottom }; });
			handle->ClearDepthStencilView(static_cast<D3DDepthStencilView*>(view)->m_Descriptor.GetCPUHandle(),(D3D12_CLEAR_FLAGS)(mode),clearV.Depth,clearV.Stencil, numRects, r.data());
		}
		else
		{
			handle->ClearDepthStencilView(static_cast<D3DDepthStencilView*>(view)->m_Descriptor.GetCPUHandle(), (D3D12_CLEAR_FLAGS)(mode), clearV.Depth, clearV.Stencil,0, nullptr);
		}
	}

	/*void D3DRenderCommand::WaitForCommandBufer(CommandBuffer* cb)
	{
		auto handle = static_cast<D3DCommandBuffer*>(cb);
	}*/

	BottomLevelAccelerationStructure D3DRenderCommand::BuildBottomLevelAS_(
		const BottomLevelAccelerationStructureDesc& Desc, const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags,
		const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const  uint64_t& Scratch_Offset, const CommandBuffer* cb)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS preBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometries;

		ID3D12GraphicsCommandList10* pCommandList = dynamic_cast<const ID3DCommandBufferInterface*>(cb)->GetCommandList();

		SKTBD_LOG_ASSERT(StorageBuffer->GetAccessFlags() & ResourceAccessFlag_GpuWrite && ScratchBuffer->GetAccessFlags() & ResourceAccessFlag_GpuWrite, "Output and Scratch Buffers Must be GPU Write Accessible")

		geometries = D3DGraphicsContext::ConvertSkateboardBlasDescToD3DGeometries(Desc);

		preBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		preBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		preBuildDesc.pGeometryDescs = geometries.data();
		preBuildDesc.NumDescs = geometries.size();
		preBuildDesc.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS(Flags);

		asDesc.Inputs = preBuildDesc;
		asDesc.DestAccelerationStructureData = static_cast<D3DBuffer*>(StorageBuffer.get())->GetResourceGPUAddress() + Desc.BufferOffset;
		asDesc.ScratchAccelerationStructureData = static_cast<const D3DBuffer*>(ScratchBuffer)->GetResourceGPUAddress() + Scratch_Offset;

		// Build the AS
		pCommandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

		AccelerationStructureData data{ StorageBuffer,Desc.BufferOffset, asDesc.DestAccelerationStructureData };

		return BottomLevelAccelerationStructure{data};

	}

	TopLevelAccelerationStructure D3DRenderCommand::BuildTopLevelAS_(const TopLevelAccelerationStructureDesc& Desc,
		const SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS& Flags, const BufferRef& StorageBuffer, const Buffer* ScratchBuffer, const  uint64_t& Scratch_Offset, const CommandBuffer* cb)
	{
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometries;

		ID3D12GraphicsCommandList10* pCommandList = dynamic_cast<const ID3DCommandBufferInterface*>(cb)->GetCommandList();

		SKTBD_LOG_ASSERT(StorageBuffer->GetAccessFlags() & ResourceAccessFlag_GpuWrite && ScratchBuffer->GetAccessFlags() & ResourceAccessFlag_GpuWrite, "Output and Scratch Buffers Must be GPU Write Accessible")

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
		buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		buildDesc.Inputs.InstanceDescs = static_cast<D3DBuffer*>(Desc.InstanceDataBuffer)->GetResourceGPUAddress() + Desc.FirstInstanceOffset;
		buildDesc.Inputs.NumDescs = Desc.NumInstances;
		buildDesc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS(Flags);
		buildDesc.DestAccelerationStructureData = static_cast<const D3DBuffer*>(StorageBuffer.get())->GetResourceGPUAddress() + Desc.BufferOffset;
		buildDesc.ScratchAccelerationStructureData = static_cast<const D3DBuffer*>(ScratchBuffer)->GetResourceGPUAddress() + Scratch_Offset;

		// Build the top-level AS
		pCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

		AccelerationStructureData data{ StorageBuffer,Desc.BufferOffset, buildDesc.DestAccelerationStructureData };

		return TopLevelAccelerationStructure{ data };
	}
}
