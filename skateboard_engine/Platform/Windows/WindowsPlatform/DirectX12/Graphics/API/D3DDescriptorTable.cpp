#include "sktbdpch.h"
#include "D3DDescriptorTable.h"
#include "Graphics/RHI/D3DGraphicsContext.h"

namespace Skateboard
{
	D3DDescriptorTable::D3DDescriptorTable(const DescriptorTableDesc& desc) :
		DescriptorTable(desc),
		p_CurrentTable(nullptr),
		m_CurrentTableIndex(0u),
		m_DescriptorSize(gD3DContext->GetDevice()->GetDescriptorHandleIncrementSize(ShaderDescriptorTableTypeToD3D(desc.Type))),
		m_TableDirty(false)
	{
	}

	D3DDescriptorTable::~D3DDescriptorTable()
	{
	}

	void D3DDescriptorTable::GenerateTable()
	{
		// We'll need at least one initial table. If no update is being made, this will remain the only table.
		// In the contrary events, new tables will be added to the vector until a new frame is made
		p_CurrentTable = &va_DescriptorTablesStart.emplace_back(std::array<D3DDescriptorHandle, GRAPHICS_SETTINGS_NUMFRAMERESOURCES>());

		// Initialise the table for all frame resources
		InitTable(*p_CurrentTable);

		// Terminate generation with base class
		DescriptorTable::GenerateTable();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3DDescriptorTable::GetGPUHandle()
	{
		CheckFrameIndex();

		// Flag the table as dirty since we won't be able to modify it further until the next frame
		// That is, if we wish to update a descriptor after getting its handle, we consider that a new
		// table must be constructed with the update descriptor such that the change doesn't affect
		// prior use of this table
		m_TableDirty = true;

		return (*p_CurrentTable)[gD3DContext->GetCurrentFrameResourceIndex()].GetGPUHandle();
	}

	void D3DDescriptorTable::UpdateTableEntry(uint64_t hash)
	{
		CheckFrameIndex();

		// If the table was flagged as dirty and a descriptor is being updated, we must use the next
		// available table and use this one until it is also flagged dirty. The GetGPUHanlde() function
		// will then return the handle of this current table
		if (m_TableDirty)
		{
			m_TableDirty = false;
			++m_CurrentTableIndex;

			if (m_CurrentTableIndex >= static_cast<uint32_t>(va_DescriptorTablesStart.size()))
			{
				// Here we're allocating more descriptor heap so that the table can be modified without corrupting any prior use to this update
				// While this might be scary (we don't want to allocate memory every frame, this is expensive), we ensure that allocation only
				// occurs based on the update rate defined by the client code. In other words, during the first frame, if the client code defines
				// 3 updates it is likely that there wont be more or less updates in the following frame. Therefore, we can just perform the allocations
				// the first time they occur (i.e. the first frame) and reuse these tables on the following frames. While this stays rather funky, it
				// will offer the desired flexibility when it comes to updating resources on the same pipeline. A proper game engine would not update
				// descriptors in flight at all like we do it, but rather construct all the descriptor tables during the scene loading and use these until exit
				auto& table = va_DescriptorTablesStart.emplace_back(std::array<D3DDescriptorHandle, GRAPHICS_SETTINGS_NUMFRAMERESOURCES>());
				InitTable(table);
			}

			p_CurrentTable = &va_DescriptorTablesStart[m_CurrentTableIndex];
		}

		// Retrieve the descriptor and its memory location in the table
		const Descriptor& descriptor = m_Table.at(hash);
		const uint64_t tableIndex = m_TableIndices.at(hash);
		D3DDescriptorHandle currentItemInTable = (*p_CurrentTable)[gD3DContext->GetCurrentFrameResourceIndex()] + tableIndex * m_DescriptorSize;
		ID3D12Device* pDevice = gD3DContext->GetDevice();

		// Perform update
		//TODO: if (m_Desc.Type == ShaderDescriptorTableType_Sampler) { ... } else..

		//GPUResource* pRes = descriptor.pResource;
		//switch (pRes->GetType())
		//{
		//case GPUResourceType_DefaultBuffer:
		//{
		//	D3DDefaultBuffer* pBuffer = static_cast<D3DDefaultBuffer*>(pRes);
		//	pDevice->CopyDescriptorsSimple(1, currentItemInTable.GetCPUHandle(), pBuffer->GetSRVHandle().GetCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//	break;
		//}
		//case GPUResourceType_UploadBuffer:
		//{
		//	D3DUploadBuffer* pBuffer = static_cast<D3DUploadBuffer*>(pRes);
		//	pDevice->CopyDescriptorsSimple(1, currentItemInTable.GetCPUHandle(), pBuffer->GetSRVHandle().GetCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//	break;
		//}
		//case GPUResourceType_Texture2D:
		//{
		//	D3DTexture* pBuffer = static_cast<D3DTexture*>(pRes);
		//	pDevice->CopyDescriptorsSimple(1, currentItemInTable.GetCPUHandle(), pBuffer->GetSRVHandle().GetCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//	break;
		//}
		//case GPUResourceType_FrameBuffer:
		//{
		//	// TODO: GetSRVHandle(0) -> Do we need to index anything else really?
		//	D3DFrameBuffer* pBuffer = static_cast<D3DFrameBuffer*>(pRes);
		//	pDevice->CopyDescriptorsSimple(1, currentItemInTable.GetCPUHandle(), pBuffer->GetSRVHandle(0).GetCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//	break;
		//}
		//case GPUResourceType_UnorderedAccessBuffer:
		//{
		//	D3DUnorderedAccessBuffer* pBuffer = static_cast<D3DUnorderedAccessBuffer*>(pRes);
		//	pDevice->CopyDescriptorsSimple(1, currentItemInTable.GetCPUHandle(), pBuffer->GetSRVHandle().GetCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//	break;
		//}
		//default:
		//	SKTBD_LOG_ASSERT(false, "unsupported");
		//	break;
		//}
	}

	void D3DDescriptorTable::InitTable(std::array<D3DDescriptorHandle, GRAPHICS_SETTINGS_NUMFRAMERESOURCES>& table)
	{
		// Allocate descriptor heap based on entry count for all frame resources
		const uint32_t entryCount = static_cast<uint32_t>(m_Table.size());
		for (uint32_t i = 0u; i < GRAPHICS_SETTINGS_NUMFRAMERESOURCES; ++i)
			table[i] = gD3DContext->GetGPUSRVDescriptorHeap().Allocate(entryCount);
	}

	void D3DDescriptorTable::CheckFrameIndex()
	{
		// If the frame index changed, then we need to reset the current table index to the first table as it can now
		// be safely reused (note: this is the case because we are using the next frame resource. The table is an array
		// of N handles, i.e. while table[0] is technically still used by the GPU, we can use table[1] safely).
		uint64_t frameIndex = gD3DContext->GetCurrentFrameResourceIndex();
		if (m_FrameResourceTracker != frameIndex)
		{
			m_FrameResourceTracker = frameIndex;
			m_CurrentTableIndex = 0u;
			m_TableDirty = false;
			p_CurrentTable = &va_DescriptorTablesStart.front();

			// Note: The last update will be transferred to the first table
			// This is intended as if the user doesn't call for further updates, this was indeed the last effective update
			// and therefore must be considered. Since the first table will be used by default (until a descriptor is changed),
			// the deferral has to occur on table[0]
			ProcessDefferredUpdates();
		}
	}
}