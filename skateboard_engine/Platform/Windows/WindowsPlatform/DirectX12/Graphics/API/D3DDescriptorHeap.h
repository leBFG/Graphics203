#pragma once
#include <memory>
#include <vector>

#include "Graphics/D3D.h"

#include "Skateboard/SizedPtr.h"
#include "Skateboard/Graphics/RHI/GraphicsContext.h"
#include "Skateboard/Memory/VirtualAllocator.h"

namespace Skateboard
{
	//struct DescriptorHandle
	//{
	//	uint64_t m_IndexIntoHeap;
	//	
	//	DescriptorHandle operator+(uint64_t offsetInDescriptors) const
	//	{
	//		DescriptorHandle ret = *this;
	//		ret += offsetInDescriptors;
	//		return ret;
	//	};
	//	void operator+=(uint64_t offsetInDescriptors)
	//	{
	//		m_IndexIntoHeap += offsetInDescriptors;
	//	};
	//};

	//class DescriptorHeap
	//{
	////	DISABLE_C_M_D(DescriptorHeap)

	//	std::unordered_map<uint64_t, VirtualAllocation> allocations;
	//	bool m_IsShaderVisible{ true };
	//	const D3D12_DESCRIPTOR_HEAP_TYPE m_Type{};

	//	BlockAllocator m_Suballocator;
	//};


	class D3DDescriptorHandle
	{
	public:
		D3DDescriptorHandle() : m_CPUHandle({ UINT64_MAX }), m_GPUHandle({ UINT64_MAX }), m_IsValid(false), m_Index(UINT32_MAX) {}
		D3DDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cH, D3D12_GPU_DESCRIPTOR_HANDLE gH) : m_CPUHandle(cH), m_GPUHandle(gH), m_IsValid(true), m_Index(UINT32_MAX) {}
		~D3DDescriptorHandle() = default;

		D3DDescriptorHandle operator+(uint64_t offset) const;
		void operator+=(uint64_t offset);

		uint64_t GetCPUPointer() const { return m_CPUHandle.ptr; }
		uint64_t GetGPUPointer() const { return m_GPUHandle.ptr; }

		const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return m_CPUHandle; }
		const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const { return m_GPUHandle; }
		D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUHandlePtr() { return &m_CPUHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE* GetGPUHandlePtr() { return &m_GPUHandle; }

		void SetValid(bool isValid);
		[[nodiscard("")]] bool IsValid() const { return m_IsValid; }

		void SetIndex(uint32_t index);
		[[nodiscard("")]] uint32_t GetIndex() const { return m_Index; }

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;
		bool m_IsValid;
		uint32_t m_Index;
	};

	class D3DDescriptorHeap
	{
	public:
		D3DDescriptorHeap() : m_DescriptorIncrementSize(0u), m_FreeDescriptorCount(0u) {}
		D3DDescriptorHeap(const D3DDescriptorHeap&) = delete;
		auto operator=(const D3DDescriptorHeap&)->D3DDescriptorHeap & = delete;
		D3DDescriptorHeap(D3DDescriptorHeap&&) noexcept = delete;
		auto operator=(D3DDescriptorHeap&&) noexcept -> D3DDescriptorHeap & = delete;

		~D3DDescriptorHeap() = default;

		void Create(const std::wstring& debugName, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shaderVisible);
		D3DDescriptorHandle Allocate(uint32_t count = 1u);

		void Free(D3DDescriptorHandle& handle);
		void ProcessDeferredFree(uint64_t frameResourceIndex);

		void Release();

		[[nodiscard("")]] ID3D12DescriptorHeap* const GetHeap() const { return m_Heap.Get(); }
		[[nodiscard("")]] D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return m_Type; }
		[[nodiscard("")]] uint32_t GetHeapSize() const { return m_HeapSize; }
		[[nodiscard("")]] uint32_t GetDescriptorIncrementSize() const { return m_DescriptorIncrementSize; }
	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
		uint32_t m_DescriptorIncrementSize;	// Avoid continuous query
		uint32_t m_FreeDescriptorCount;

		uint32_t m_HeapSize{ 0u };
		uint32_t m_HeapCapacity{ 0u };
		uint32_t m_HeapCount{ 0u };

		D3DDescriptorHandle m_FirstHandle;
		D3DDescriptorHandle m_NextFreeHandle;

		std::unique_ptr<uint32_t[]>		m_AvailableHandles{ nullptr };
		std::vector<uint32_t>			m_DeferredAvailableIndices[GRAPHICS_SETTINGS_NUMFRAMERESOURCES]{};
		std::mutex m_HeapMutex;

		bool m_IsShaderVisible{ true };
		const D3D12_DESCRIPTOR_HEAP_TYPE m_Type{};

		MemoryUtils::BlockAllocator* m_suballocator;
	};
}