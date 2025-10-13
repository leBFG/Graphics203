#include "AGCMemoryAllocator.h"

#ifdef SKTBD_DEBUG
#include <sanitizer/asan_interface.h>
#include <mat.h>
#endif

#define SKTBD_LOG_COMPONENT "MEMORY ALLOC IMPL"
#include "Skateboard/Core.h"
#include "Skateboard/Log.h"

#define SCE_MEM_MIN_ALIGNMENT (16*1024-1)

namespace Skateboard
{

#ifdef SKTBD_DEBUG
	//global counter for Memory analyzer groups representing each Allocator instantiation
	size_t s_matGroups = 0;
#endif // SKTBD_DEBUG

	void RequestVirtualMemoryBlock(off_t& out_addr, size_t size, size_t alignment, SceKernelMemoryType mem_type)
	{
		const auto align = (alignment + SCE_MEM_MIN_ALIGNMENT) & ~SCE_MEM_MIN_ALIGNMENT;
		size = (size + (align -1)) & ~(align -1);

		int32_t ret = sceKernelAllocateMainDirectMemory(size, align, mem_type, &out_addr);
		if (ret)
		{
			printf("sceKernelAllocateMainDirectMemory error:0x%x size:0x%zx\n", ret, size);

			SKTBD_LOG_ASSERT(ret == SCE_OK, "Unable to map memory");

			out_addr = 0;
			return;
		}
	}

	void FreeVirtualMemoryBlock(off_t in_adr, size_t size)
	{
		auto ret = sceKernelReleaseDirectMemory(in_adr, size);
		SKTBD_LOG_ASSERT(ret == SCE_OK, "Unable to Release Virtual memory");
	}

	void MapMemoryBlock(void** out_adr, off_t in_unmapped_adr, size_t size, size_t alignment, uint32_t mem_protection_flags, std::string name)
	{
		// Map a c++ raw pointer to the data
		
		*out_adr = nullptr;

		const auto align = (alignment + SCE_MEM_MIN_ALIGNMENT) & ~SCE_MEM_MIN_ALIGNMENT;
		size = (size + (align - 1)) & ~(align - 1);

		auto ret = 0;
		if (name.size())
		{
			ret = sceKernelMapNamedDirectMemory(out_adr, size, mem_protection_flags, 0, in_unmapped_adr, align, name.substr(0,32).c_str());
		}
		else
		{
			ret = sceKernelMapDirectMemory(out_adr, size, mem_protection_flags, 0, in_unmapped_adr, align);
		}

		SKTBD_LOG_ASSERT(ret == SCE_OK, "Unable to map memory");
	}

	void UnMapMemoryBlock(void* in_adr, size_t size)
	{
		auto ret = sceKernelMunmap(in_adr, size);
		SKTBD_LOG_ASSERT(ret == SCE_OK, "Unable to Release VA memory");
	}


	void* RequestSysMemBlock(size_t size, size_t align, uint32_t mem_protection_flags, SceKernelMemoryType mem_type)
	{
		off_t offsetOut;

		RequestVirtualMemoryBlock(offsetOut, size, align, mem_type);
		
		// Map a c++ raw pointer to the data
		void* ptr = nullptr;
		MapMemoryBlock(&ptr, offsetOut, size, align, mem_protection_flags);

		return ptr;
	}



	void FreeSysMemBlock(void* ptr)
	{
		SceKernelVirtualQueryInfo info;

		if (sceKernelVirtualQuery(ptr, 0, &info, sizeof(info)) < 0)
		{
			printf("virtual query failed for mem 0x%p\n", ptr);
			return;
		}

		if (sceKernelReleaseDirectMemory(info.offset, (uintptr_t)info.end - (uintptr_t)info.start) < 0)
		{
			printf("releasing direct memory failed for offset 0x%lx\n", info.offset);
			return;
		}
	}

}