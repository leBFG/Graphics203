#include "sktbdpch.h"
#include "D3DDescriptorHeap.h"

#include "Graphics/RHI/D3DGraphicsContext.h"

namespace Skateboard
{
	D3DDescriptorHandle D3DDescriptorHandle::operator+(uint64_t offset) const
	{
		D3DDescriptorHandle ret = *this;
		ret += offset;
		return ret;
	}

	void D3DDescriptorHandle::operator+=(uint64_t offset)
	{
		//SKTBD_LOG_ASSERT(m_CPUHandle.ptr != (size_t)-1, "Invalid descriptor handle!");
		//SKTBD_LOG_ASSERT(m_GPUHandle.ptr != (uint64_t)-1, "Invalid descriptor handle!");

		m_CPUHandle.ptr += offset;
		m_GPUHandle.ptr += offset;
	}

	void D3DDescriptorHandle::SetValid(bool isValid)
	{
		m_IsValid = isValid;
	}

	void D3DDescriptorHandle::SetIndex(uint32_t index)
	{
		m_Index = index;
	}

	void D3DDescriptorHeap::Create(const std::wstring& debugName, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shaderVisible)
	{
		const auto pDevice = gD3DContext->GetDevice();

		SKTBD_LOG_ASSERT((capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2), "Invalid capacity!");
//		SKTBD_LOG_ASSERT(((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) && capacity < D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE), "Invalid heap type!");

		if (m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			m_IsShaderVisible = false;
		}

		m_IsShaderVisible = shaderVisible;

		// Create a descriptor heap that can allocate up to maxCount descriptors
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = capacity;
		desc.Flags = (shaderVisible) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;
		pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_Heap.ReleaseAndGetAddressOf()));
#ifndef SKTBD_SHIP
		m_Heap->SetName(debugName.c_str());
#endif

		// Setup memory tracking
		m_DescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(desc.Type);
		m_FreeDescriptorCount = desc.NumDescriptors;

		m_FirstHandle = (m_IsShaderVisible) ? D3DDescriptorHandle(m_Heap->GetCPUDescriptorHandleForHeapStart(), m_Heap->GetGPUDescriptorHandleForHeapStart()) : D3DDescriptorHandle(m_Heap->GetCPUDescriptorHandleForHeapStart(), D3D12_GPU_DESCRIPTOR_HANDLE());

		m_NextFreeHandle = m_FirstHandle;

		// Initialise available slots, capacity and current descriptor count.
		m_AvailableHandles = std::make_unique<uint32_t[]>(capacity);
		m_HeapCapacity = capacity;
		m_HeapCount = 0u;
		m_HeapSize = gD3DContext->GetDevice()->GetDescriptorHandleIncrementSize(type);

		// Set the available indices.
		for(uint32_t i = 0; i < capacity; ++i)
		{
			m_AvailableHandles[i] = i;
		}

		for(uint32_t i = 0; i < GRAPHICS_SETTINGS_NUMFRAMERESOURCES; ++i)
		{

		}

	//	VIRTUAL_BLOCK_DESC aDesc = {};
	//	aDesc.Size = desc.NumDescriptors * m_DescriptorIncrementSize;
	//	
	//	
	//	Skateboard::MemoryUtils::CreateBlock(&aDesc, &m_suballocator);

	//	VirtualAllocation allocation;

	//	Skateboard::MemoryUtils::VIRTUAL_ALLOCATION_DESC allocDesc = {};
	////	allocDesc.Flags = VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME;
	//	allocDesc.Size = 1;

	//	uint64_t offset;

	//	m_suballocator->Allocate(&allocDesc, &allocation, &offset);
		
	}

	D3DDescriptorHandle D3DDescriptorHeap::Allocate(uint32_t count)
	{
		// Protects the heap from race conditions
		std::lock_guard lock(m_HeapMutex);

		if (count == 0u)
		{
			SKTBD_MSG_WARN("Tried to allocate 0 descriptors in a descriptor heap. Returning an invalid handle");
			return { {SIZE_T_MAX}, {UINT64_MAX} };
		}

		// Check if there is enough space, otherwise throw as this would be an invalid descriptor allocation (your GPU would crash either way after using it)
		SKTBD_LOG_ASSERT(count <= m_FreeDescriptorCount, "Not enough space to allocate new descriptor(s) on this heap, consider increasing heap size!");
		SKTBD_LOG_ASSERT(!(m_HeapCount > m_HeapCapacity), "Out of Memory!");

		// Allocation simply means changing the memory pointer to a further region of the allocated heap
		D3DDescriptorHandle ret = m_NextFreeHandle;
		m_NextFreeHandle += count * m_DescriptorIncrementSize;
		m_FreeDescriptorCount -= count;

		ret.SetIndex(m_AvailableHandles[m_HeapCount]);

		m_HeapCount += count;

		return ret;
	}

	void D3DDescriptorHeap::Free(D3DDescriptorHandle& handle)
	{
		if(!handle.IsValid())
		{
			return;
		}
		// Protects the heap from race conditions
		std::lock_guard lock(m_HeapMutex);

		// We need to ensure the provided handle is valid.
		// Check it is not pointing to an address beyond the heap, the type of handle matches this heap type
		// and the handle index is valid.
		SKTBD_LOG_ASSERT(handle.GetCPUHandlePtr()->ptr >= m_FirstHandle.GetCPUHandlePtr()->ptr, "Invalid handle!");
		SKTBD_LOG_ASSERT((handle.GetCPUHandlePtr()->ptr - m_FirstHandle.GetCPUHandlePtr()->ptr) % m_HeapSize == 0, "Incorrect type of handle.");
		const auto index = static_cast<uint32_t>((handle.GetCPUHandlePtr()->ptr - m_FirstHandle.GetCPUHandlePtr()->ptr) / m_DescriptorIncrementSize);
		SKTBD_LOG_ASSERT(index < m_HeapCapacity, "Index has exceeded maximum heap capacity!");

		// Defer freeing any resources until the next frame has started. 
		m_DeferredAvailableIndices[gD3DContext->GetCurrentFrameResourceIndex()].push_back(index);

		//Inform the internal D3D engine, descriptors need to be free.
		gD3DContext->SetDeferredReleasesFlag();
		handle= {};
	}

	void D3DDescriptorHeap::ProcessDeferredFree(uint64_t frameResourceIndex)
	{
		// Protects the heap from race conditions
		std::lock_guard lock(m_HeapMutex);

		// Ensure we haven't supplied an invalid frame index.
		SKTBD_LOG_ASSERT(frameResourceIndex < GRAPHICS_SETTINGS_NUMFRAMERESOURCES, "Invalid frame index!");

		std::vector<UINT32>& indices{ m_DeferredAvailableIndices[frameResourceIndex] };

		// Clear unused indices and return them to the pool of available slots.
		if (!indices.empty())
		{
			for (const auto index : indices)
			{
				--m_HeapCount;
				m_AvailableHandles[m_HeapCount] = index;
			}
			indices.clear();
		}

	}

	void D3DDescriptorHeap::Release()
	{
		// At this point, calling release, all descriptors and their resources should be clean!
		// We cannot remove the heap until this has been achieved.
		SKTBD_LOG_ASSERT(!m_HeapCount, "Heap is not empty!");

		// Push the heap back into the deferred resources vector.
		// Again, it is imperative at this point in the shutdown process, all resources have been cleaned.
		gD3DContext->DeferredRelease(m_Heap.Get());

	}
}
