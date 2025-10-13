#pragma once

#include "AGC/Graphics/AGCF.h"
#include "AGC/Memory/AGCMemoryAllocator.h"
#include "Skateboard/Memory/VirtualAllocator.h"

#include "Skateboard/Log.h"

namespace Skateboard
{
	/// <summary>
	/// Playstation 5 doesnt have a descriptor heap per se but it allows us to create descriptors in gpu visible memory, any gpu visible memory,
	/// meaning for bindless approach of Dx12 where descriptors all live in heap
	/// we can simply create a sce::agc::Buffer filled with sce::Agc::ResourceDescriptor and expose it to shaders as RegularBuffer<ResourceDescriptor>
	/// </summary>

	template<typename DescriptorType>
	struct DescriptorHandle
	{
		DescriptorType* m_DescriptorPtr;
		uint m_Index;
		MemoryUtils::VirtualAllocation m_Handle;
	};

	template<typename DescriptorType>
	class DescriptorHeap
	{
		MemoryHandle<DescriptorType, sce::Agc::Alignment::kBuffer> m_DescriptorBufferMemory;
		sce::Agc::Core::Buffer m_DescriptorBuffer = sce::Agc::Core::Buffer();
		MemoryUtils::BlockAllocator* m_Allocator = nullptr;

	public:
		sce::Agc::Core::Buffer GetHeapBaseDescriptor() const { return m_DescriptorBuffer; }


		void InitDescriptorHeap(TemplatedGPUMemoryPoolAllocator& Allocator, std::string Debug_name, size_t Descriptor_count = 64 * 1024 / sizeof(DescriptorType))
		{
			m_DescriptorBufferMemory = Allocator.TypedAlignedAllocate<DescriptorType, sce::Agc::Alignment::kBuffer>(Descriptor_count);
			auto err = sce::Agc::Core::initializeRegularBuffer(  &m_DescriptorBuffer, m_DescriptorBufferMemory.data, sizeof(DescriptorType), Descriptor_count);
			SCE_AGC_ASSERT(err == SCE_OK);

			MemoryUtils::VIRTUAL_BLOCK_DESC allocatorDesc = { MemoryUtils::VIRTUAL_BLOCK_FLAGS::VIRTUAL_BLOCK_FLAG_NONE, Descriptor_count };
			CreateBlock(&allocatorDesc, &m_Allocator); 

			SKTBD_LOG_INFO("AGCDescriptorHeap", "Initialised Descriptor Heap: {0} with {1} descriptors", typeid(DescriptorType).name(), Descriptor_count);

			sce::Agc::Core::registerResource(&m_DescriptorBuffer, Debug_name.c_str());
		}
		

		DescriptorHandle<DescriptorType> Allocate(size_t count = 1)
		{
			MemoryUtils::VIRTUAL_ALLOCATION_DESC Desc = { MemoryUtils::VIRTUAL_ALLOCATION_FLAGS::VIRTUAL_ALLOCATION_FLAG_NONE, count, 0, nullptr };

			DescriptorHandle<DescriptorType> ret;

			uint64_t Offt;
			m_Allocator->Allocate(&Desc, &ret.m_Handle, &Offt);

			ret.m_Index = Offt;
			ret.m_DescriptorPtr = &m_DescriptorBufferMemory.data[Offt];

			return ret;
		};

		void Free(DescriptorHandle<DescriptorType>& handle)
		{
			m_Allocator->FreeAllocation(handle.m_Handle);
		};
	};

}
