 #pragma once
#include <memory>
#include <agc.h>
#include <entt.hpp>

#include "Skateboard/Core.h"
#include "sktbdpch.h"

#ifdef SKTBD_DEBUG
#include <mat.h>
#include <sanitizer/asan_interface.h>

//#define ALLOC_PRINT

#endif // SKTBD_DEBUG


namespace Skateboard
{
	/// <summary>
	/// allocate a virtual memory range on ps5
	/// </summary>
	/// <param name="out_addr"> this will be the addrress of memory when its mapped, technically it could be a void* intead of off_t but if we were to try access void* w would get a [age fault hence the offt</param>
	/// <param name="size"></param>
	/// <param name="alignment"></param>
	/// <param name="mem_type"> Use TYPE_C_SHARED For CPU-GPU accessed memory, Use TYPE_C for GPU only accesed memory (PS most of the memory used will be c_shared)</param>

	void RequestVirtualMemoryBlock(off_t& out_addr, size_t size, size_t alignment, SceKernelMemoryType mem_type);

	void FreeVirtualMemoryBlock(off_t& in_adr, size_t size);

	/// <summary>
	/// Map virtual range 
	/// </summary>
	/// <param name="out_adr"></param>
	/// <param name="in_unmapped_adr"></param>
	/// <param name="size"></param>
	/// <param name="align"></param>
	/// <param name="mem_protection_flags"></param>
	/// <param name="name">optional name string, max size 32 chars</param>

	void MapMemoryBlock(void** out_adr, off_t in_unmapped_adr, size_t size, size_t align, uint32_t mem_protection_flags, std::string name = "");

	void UnMapMemoryBlock(void* in_adr, size_t size);

	/// <summary>
	/// Convinience functions to get physical memory directly
	/// </summary>
	/// <param name="size"></param>
	/// <param name="align"></param>
	/// <param name="mem_protection_flags"></param>
	/// <param name="mem_type"></param>
	/// <returns></returns>

	void* RequestSysMemBlock(size_t size, size_t align, uint32_t mem_protection_flags, SceKernelMemoryType mem_type = SCE_KERNEL_MTYPE_C_SHARED);

	void  FreeSysMemBlock(void* ptr);

	//general purpouse templated suballocator 

	template<typename T, size_t align, uint32_t flags>
	struct Block
	{
		void* data;
		Block* next = nullptr;
		std::map<size_t, size_t> emptySpace;
		size_t blockSize;

#ifdef SKTBD_DEBUG
		MatPool m_pool;
#endif

		Block(size_t size)
		{
			data = (void*)RequestSysMemBlock(size, 0xffff, flags);

			uint64_t alignsize = (size + 0xffffu) & ~0xffffu; //alignment of system allocations
			constexpr uint64_t type_alignsize = (sizeof(T) + (align - 1)) & ~(align - 1);//(sizeof(T) + align) & ~align; //alignment of elements 

			blockSize = alignsize;

			emptySpace[(size_t)data] = alignsize / type_alignsize;
		
#ifdef ALLOC_PRINT
			std::cout << typeid(T).name() << " allocator (align: " << align << ") created new block: " << blockSize << " with elements " << alignsize / type_alignsize << " at address: " << (size_t)data << std::endl;
#endif // ALLOC_PRINT
		};
	};

	extern size_t s_matGroups;

	//expand with memory and gpu access flags?
	template<typename T, size_t align, uint32_t flags>
	class Allocator
	{
	
#ifdef SKTBD_DEBUG
	public:
		inline static MatGroup MatGroup = 0;
	private:
#endif
		inline static Block<T, align, flags>* firstBlock = nullptr;

		static Block<T, align, flags>* NewBlock(size_t size)
		{
			auto block = new Block<T, align, flags>(size);

#ifdef SKTBD_DEBUG
			block->m_pool = sceMatAllocPoolMemory(block->data, block->blockSize, MatGroup);
			sceMatTagPool(block->m_pool, typeid(Block<T, align, flags>).name());

			ASAN_POISON_MEMORY_REGION(block->data, block->blockSize);
#endif
			return block;
		};

	public:
		inline constexpr static uint64_t type_alignsize = (sizeof(T) + (align - 1)) & ~(align - 1);

		static const Block<T, align, flags>* GetFirstBlock() { return firstBlock; };

		static T* Allocate(size_t count)
		{
			if (count == 0) return nullptr;

			size_t size = count * sizeof(T);
			size_t aligned_size = (size + (type_alignsize - 1)) & ~(type_alignsize - 1);

			size_t packed_count = aligned_size / type_alignsize;

			if (!firstBlock)
			{
#ifdef SKTBD_DEBUG
				//newMatGroup = s_matGroups;

				std::string name = std::string(typeid(Allocator < T, align,flags>).name());
				// bind it to this type and alignment
				MatGroup = s_matGroups++;
				sceMatRegisterGroup(MatGroup, name.c_str(), SCEMAT_GROUP_AUTO);
#endif // SKTBD_DEBUG

				firstBlock = NewBlock(aligned_size);
			}

			Block<T, align, flags>* current = firstBlock;

			while (true)
			{
				if (current->emptySpace.size())
				{
					//go through empty spaces and check if we can squeeze the requested count // might not be the best way to find a "right" fit; ie blocks down the chain might contain exact fits
					for (auto [ptr, c] : current->emptySpace)
					{
						if (c == packed_count)
						{
							current->emptySpace.erase(ptr);

#ifdef ALLOC_PRINT
							std::cout << typeid(T).name() << " allocator (align: " << align << ") allocated: " << packed_count << " elements " << "at address: " << ptr << std::endl;
#endif

							#ifdef SKTBD_DEBUG
							
							sceMatAlloc((void*)ptr, packed_count, sizeof(T) - type_alignsize , MatGroup);
							ASAN_UNPOISON_MEMORY_REGION((void*)ptr, aligned_size);

							#endif // SKTBD_DEBUG


							return (T*)ptr;
						}
						if (c > packed_count)
						{
							current->emptySpace.erase(ptr);
							current->emptySpace[(size_t)ptr + aligned_size] = c - packed_count;

							#ifdef SKTBD_DEBUG
							sceMatAlloc((void*)ptr, packed_count, sizeof(T) - type_alignsize, MatGroup);
							ASAN_UNPOISON_MEMORY_REGION((void*)ptr, aligned_size);
							
#ifdef ALLOC_PRINT
							std::cout << typeid(T).name() << " allocator (align: " << align << ") allocated: " << packed_count << " elements " << "at address: " << ptr << std::endl;
#endif // ALLOC_PRINT
							#endif // SKTBD_DEBUG
							
							

							return (T*)ptr;
						}
					}
				}

				if (current->next)
				{
					current = current->next;
				}
				else
				{
					current->next = NewBlock(aligned_size);
					current = current->next;
				}
			}
		};

		static void Free(T* ptr, size_t count)
		{
			Block<T, align, flags>* current = firstBlock;

			size_t size = count * sizeof(T);
			size_t aligned_size = (size + (type_alignsize - 1)) & ~(type_alignsize - 1);

			size_t packed_count = aligned_size / type_alignsize;

			while (true)
			{
				if (current == nullptr)
				{
					//std::cout << "provided an invalid adress to free" << std::endl;
					return;
				}

				if ((size_t)current->data <= (size_t)ptr && (size_t)ptr < (size_t)current->data + current->blockSize)
				{
					current->emptySpace[(size_t)ptr] = packed_count;
					

					#ifdef SKTBD_DEBUG

#ifdef ALLOC_PRINT
					std::cout << "freed an address: " << (size_t)ptr << " size: " << count << std::endl;
#endif // ALLOC_PRINT

					ASAN_POISON_MEMORY_REGION((void*)ptr, aligned_size);
					sceMatFree((void*)ptr);

					#endif

					//release blocks if they are empty
					return;
				}
				else
				{
					current = current->next;
				}


			}
		};

		static void PrintoutAllocatorState(char* dbg)
		{
			Block<T, align, flags>* current = firstBlock;

			int count = 0;

			while (true)
			{
				if (!current) return;

				std::cout << dbg << std::endl;
				std::cout << typeid(T).name() << " " << count << " Block size: " << current->blockSize << " free space: " << std::endl;
				for (auto i : current->emptySpace) std::cout << "     start addr: " << i.first << " Empty Size: " << i.second << std::endl;
				count++;
				current = current->next;
			}
		}
	};

	template<typename T, size_t alignment>
	struct MemoryHandle
	{
		MemoryHandle() : data(nullptr), count(0) {};
		MemoryHandle(T* ptr,size_t cnt ) : data(ptr), count(cnt) {};

		T* data;
		size_t count;
	};

	struct RawMemoryHandle
	{
		void* m_Data;
		bool m_isPhysical;
		sce::Agc::SizeAlign m_SizeAlign;
		SceKernelMemoryType m_Memtype;
	};

	class TemplatedGPUMemoryPoolAllocator
	{
		//local data about the allocator
	public:
		template<typename T, size_t align>
		T* TypedAlignedAllocateSimple(size_t count)
		{
			Allocator<T, align, SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_CPU_RW> alloc;
			return alloc.Allocate(count);
		};

		template<typename T, size_t align>
		void TypedAlignedFreeSimple(T* ptr, size_t count)
		{
			Allocator<T, align, SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_CPU_RW> alloc;
			alloc.Free(ptr,count);
		}

		template<typename T, size_t align>
		MemoryHandle<T, align> TypedAlignedAllocate(size_t count)
		{
			return { TypedAlignedAllocateSimple<T, align>(count), count};
		}

		template<typename T, size_t align>
		void TypedAlignedFree(MemoryHandle<T, align> &hmem)
		{
			Allocator<T, align, SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_CPU_RW> alloc;
			alloc.Free(hmem.data, hmem.count);
			hmem.count = 0;
			hmem.data = nullptr;
		}

		template<size_t align = 1>
		void* AlignedByteAllocate(size_t bytecount)
		{
			return TypedAlignedAllocateSimple<uint8_t, align>(bytecount);
		}

		template<size_t align = 1>
		void* AlignedByteFree(size_t bytecount)
		{
			return TypedAlignedFreeSimple<uint8_t, align>(bytecount);
		}

		static RawMemoryHandle AllocateRaw(sce::Agc::SizeAlign sizealign, uint ProtectionFlags, SceKernelMemoryType Type = SCE_KERNEL_MTYPE_C_SHARED)
		{
			RawMemoryHandle ret =  { RequestSysMemBlock(sizealign.m_size,sizealign.m_align, ProtectionFlags,Type),true, sizealign, Type };

			ASAN_UNPOISON_MEMORY_REGION(ret.m_Data, sizealign.m_size);

			return ret;
		}

		static void FreeRaw(RawMemoryHandle& handle)
		{
			ASAN_POISON_MEMORY_REGION(handle.m_Data, handle.m_SizeAlign.m_size);

			FreeSysMemBlock(handle.m_Data);
			handle.m_Data = nullptr;
			handle.m_isPhysical = false;
		}
	};

}