#pragma once
#include "Graphics/D3D.h"
#include "Skateboard/Graphics/Resources/CommandBuffer.h"

namespace Skateboard
{
	struct ID3DCommandBufferInterface
	{
		virtual ID3D12GraphicsCommandList10* GetCommandList() const = 0;
		virtual ID3D12CommandAllocator* GetCommandAllocator() const = 0;

	};

	class D3DFence final : public Fence
	{
	public:
		void Signal(uint64_t NewValue) override;
		D3DFence(uint64_t initialValue);
		uint64_t GetValue() const override;
		void SetDebugName(const std::wstring& debug_name) override { m_FenceObject->SetName(debug_name.c_str()); };
		Microsoft::WRL::ComPtr<ID3D12Fence1> m_FenceObject;
	};

	class D3DGraphicsCommandBuffer final : public GraphicsCommandBuffer, public ID3DCommandBufferInterface
	{
	public:
		D3DGraphicsCommandBuffer(CommandBufferPriority_ priority);
		void SetDebugName(const std::wstring& debug_name) override { m_CommandList->SetName(debug_name.c_str()); };
		ID3D12GraphicsCommandList10* GetCommandList() const override { return m_CommandList.Get(); }
		ID3D12CommandAllocator* GetCommandAllocator() const override { return m_Allocator.Get();   }

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_Allocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList10> m_CommandList;
		uint64_t m_CompletionFenceValue;
	};

	class D3DComputeCommandBuffer final : public ComputeCommandBuffer, public ID3DCommandBufferInterface
	{
	public:
		D3DComputeCommandBuffer(CommandBufferPriority_ priority);
		void SetDebugName(const std::wstring& debug_name) override { m_CommandList->SetName(debug_name.c_str()); };
		ID3D12GraphicsCommandList10* GetCommandList() const override { return m_CommandList.Get(); }
		ID3D12CommandAllocator* GetCommandAllocator() const override { return m_Allocator.Get(); }

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_Allocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList10> m_CommandList;
		uint64_t m_CompletionFenceValue;
	};

}
