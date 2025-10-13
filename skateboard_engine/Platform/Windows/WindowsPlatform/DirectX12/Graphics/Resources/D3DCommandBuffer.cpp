#include "sktbdpch.h"
#include "D3DCommandBuffer.h"

#include "Graphics/RHI/D3DGraphicsContext.h"

namespace Skateboard
{
	D3DFence::D3DFence(uint64_t initialValue) : Fence(initialValue)
	{
		D3D_CHECK_FAILURE(gD3DContext->GetDevice()->CreateFence(initialValue, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_FenceObject.GetAddressOf())))
	};

	void D3DFence::Signal(uint64_t NewValue)
	{
		m_FenceObject->Signal(NewValue);
	};

	uint64_t D3DFence::GetValue() const
	{
		return m_FenceObject->GetCompletedValue();
	};

	D3DGraphicsCommandBuffer::D3DGraphicsCommandBuffer(CommandBufferPriority_ priority) : GraphicsCommandBuffer(priority)
	{
		D3D_CHECK_FAILURE(gD3DContext->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_Allocator.ReleaseAndGetAddressOf())));

		// Create the command list
		D3D_CHECK_FAILURE(gD3DContext->GetDevice()->CreateCommandList1(
			0,											// For single GPU operation, set this to zero. Otherwise use this to identify the adapter
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_LIST_FLAG_NONE,
			IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf())
		));
	};

	D3DComputeCommandBuffer::D3DComputeCommandBuffer(CommandBufferPriority_ priority) : ComputeCommandBuffer(priority)
	{
		D3D_CHECK_FAILURE(gD3DContext->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(m_Allocator.ReleaseAndGetAddressOf())));

		// Create the command list
		D3D_CHECK_FAILURE(gD3DContext->GetDevice()->CreateCommandList1(
			0,											// For single GPU operation, set this to zero. Otherwise use this to identify the adapter
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			D3D12_COMMAND_LIST_FLAG_NONE,
			IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf())
		));
	};
}
