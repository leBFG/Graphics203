#pragma once
#include "sktbdpch.h"

//
// Copyright (c) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Modified 2024 Einar Bruveris - stripped d3d12 to just leave pure allocator structs 
//

namespace Skateboard::MemoryUtils
{
#define MA_CLASS_NO_COPY(className) \
    private: \
        className(const className&) = delete; \
        className(className&&) = delete; \
        className& operator=(const className&) = delete; \
        className& operator=(className&&) = delete;

        // Assert that will be called very often, like inside data structures e.g. operator[].
        // Making it non-empty can make program slow.
#ifndef MA_HEAVY_ASSERT
#ifdef _DEBUG
#define MA_HEAVY_ASSERT(expr)   SKTBD_ASSERT(expr)
#else
#define MA_HEAVY_ASSERT(expr)
#endif
#endif

        class VirtualBlockPimpl;
        /// \endcond

        class Pool;
        class Allocator;
        struct Statistics;
        struct DetailedStatistics;
        struct TotalStatistics;

        /// \brief Unique identifier of single allocation done inside the memory heap.
        typedef uint64_t AllocHandle;

        /// Pointer to custom callback function that allocates CPU memory.
        using ALLOCATE_FUNC_PTR = void* (*)(size_t Size, size_t Alignment, void* pPrivateData);
        /**
        \brief Pointer to custom callback function that deallocates CPU memory.

        `pMemory = null` should be accepted and ignored.
        */
        using FREE_FUNC_PTR = void (*)(void* pMemory, void* pPrivateData);

        struct ALLOCATION_CALLBACKS
        {
            /// %Allocation function.
            ALLOCATE_FUNC_PTR pAllocate;
            /// Dellocation function.
            FREE_FUNC_PTR pFree;
            /// Custom data that will be passed to allocation and deallocation functions as `pUserData` parameter.
            void* pPrivateData;
        };

        struct Statistics
        {
            uint32_t BlockCount;
            uint32_t AllocationCount;
            uint64_t BlockBytes;
            uint64_t AllocationBytes;
        };

        struct DetailedStatistics
        {
            /// Basic statistics.
            Statistics Stats;
            /// Number of free ranges of memory between allocations.
            uint32_t UnusedRangeCount;
            /// Smallest allocation size. `UINT64_MAX` if there are 0 allocations.
            uint64_t AllocationSizeMin;
            /// Largest allocation size. 0 if there are 0 allocations.
            uint64_t AllocationSizeMax;
            /// Smallest empty range size. `UINT64_MAX` if there are 0 empty ranges.
            uint64_t UnusedRangeSizeMin;
            /// Largest empty range size. 0 if there are 0 empty ranges.
            uint64_t UnusedRangeSizeMax;
        };

        struct TotalStatistics
        {
            DetailedStatistics HeapType[5];
            DetailedStatistics MemorySegmentGroup[2];
            DetailedStatistics Total;
        };

        struct Budget
        {
            Statistics Stats;
            uint64_t UsageBytes;
            uint64_t BudgetBytes;
        };

        struct VirtualAllocation
        {
            AllocHandle AllocHandle;
        };

        /// \brief Bit flags to be used with VIRTUAL_BLOCK_DESC::Flags.
        enum VIRTUAL_BLOCK_FLAGS
        {
            VIRTUAL_BLOCK_FLAG_NONE = 0,
            VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR = 0x1,
            VIRTUAL_BLOCK_FLAG_ALGORITHM_MASK = VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR
        };

        /// Parameters of created MA::Allocator object to be passed to CreateVirtualBlock().
        struct VIRTUAL_BLOCK_DESC
        {
            VIRTUAL_BLOCK_FLAGS Flags;
            uint64_t Size;
            const ALLOCATION_CALLBACKS* pAllocationCallbacks;
        };

        /// \brief Bit flags to be used with VIRTUAL_ALLOCATION_DESC::Flags.
        enum VIRTUAL_ALLOCATION_FLAGS
        {
            VIRTUAL_ALLOCATION_FLAG_NONE = 0,

            /** %Allocation will be created from upper stack in a double stack pool.
            This flag is only allowed for custom pools created with #POOL_FLAG_ALGORITHM_LINEAR flag.
            */
            VIRTUAL_ALLOCATION_FLAG_UPPER_ADDRESS = 0x8,

            /** %Allocation strategy that chooses smallest possible free range for the allocation
            to minimize memory usage and fragmentation, possibly at the expense of allocation time.
            */
            VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY = 0x00010000,

            /** %Allocation strategy that chooses first suitable free range for the allocation -
            not necessarily in terms of the smallest offset but the one that is easiest and fastest to find
            to minimize allocation time, possibly at the expense of allocation quality.
            */
            VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME = 0x00020000,

            /** %Allocation strategy that chooses always the lowest offset in available space.
            This is not the most efficient strategy but achieves highly packed data.
            Used internally by defragmentation, not recomended in typical usage.
            */
            VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET = 0x0004000,

            //MASK
            VIRTUAL_ALLOCATION_FLAG_STRATEGY_MASK = VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY | VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME | VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET,
        };

        /// Parameters of created virtual allocation to be passed to Allocator::Allocate().
        struct VIRTUAL_ALLOCATION_DESC
        {
            VIRTUAL_ALLOCATION_FLAGS Flags;
            uint64_t Size;
            uint64_t Alignment;
            void* pPrivateData;
        };

        /// Parameters of an existing virtual allocation, returned by Allocator::GetAllocationInfo().
        struct VIRTUAL_ALLOCATION_INFO
        {
            uint64_t Offset;
            uint64_t Size;
            void* pPrivateData;
        };

        /** \brief Represents pure allocation algorithm and a data structure with allocations in some memory block, without actually allocating any GPU memory.

        This class allows to use the core algorithm of the library custom allocations e.g. CPU memory or
        sub-allocation regions inside a single GPU buffer.

        To create this object, fill in MA::VIRTUAL_BLOCK_DESC and call CreateVirtualBlock().
        To destroy it, call its method `Allocator::Release()`.
        You need to free all the allocations within this block or call Clear() before destroying it.

        This object is not thread-safe - should not be used from multiple threads simultaneously, must be synchronized externally.
        */
        class BlockAllocator
        {
        public:
            bool IsEmpty() const;
            void GetAllocationInfo(VirtualAllocation allocation, VIRTUAL_ALLOCATION_INFO* pInfo) const;
            SKTBDR Allocate(const VIRTUAL_ALLOCATION_DESC* pDesc, VirtualAllocation* pAllocation, uint64_t* pOffset);
            void FreeAllocation(VirtualAllocation allocation);
            void Clear();
            void SetAllocationPrivateData(VirtualAllocation allocation, void* pPrivateData);
            void GetStatistics(Statistics* pStats) const;
            void CalculateStatistics(DetailedStatistics* pStats) const;

            void BuildStatsString(wchar_t** ppStatsString) const;
            void FreeStatsString(wchar_t* pStatsString) const;

            //part of the IUnknown that needs to be separately implemented dealing with COM Reference system
            void Release() { ReleaseThis(); }

        protected:
            void ReleaseThis();

        private:
            friend void CreateBlock(const VIRTUAL_BLOCK_DESC*, BlockAllocator**);
            template<typename T> friend void MA_DELETE(const ALLOCATION_CALLBACKS&, T*);

            VirtualBlockPimpl* m_Pimpl;

            BlockAllocator(const ALLOCATION_CALLBACKS& allocationCallbacks, const VIRTUAL_BLOCK_DESC& desc);
            ~BlockAllocator();

            MA_CLASS_NO_COPY(BlockAllocator)
        };

        void CreateBlock(const VIRTUAL_BLOCK_DESC* pDesc, BlockAllocator** ppVirtualBlock);
};