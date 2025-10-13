#include "sktbdpch.h"
#include "UploadManager.h"
#include "Graphics/RHI/D3DGraphicsContext.h"

void UploadManager::Init(D3D12MA::Allocator* alloc, const size_t UploadBufferSize)
{
	D3D12_RESOURCE_DESC1 Desc{};
	Desc.Alignment = 0;
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	Desc.DepthOrArraySize = 1;
	Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	Desc.Format = DXGI_FORMAT_UNKNOWN;
	Desc.Height = 1;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Desc.MipLevels = 1;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Width = ROUND_UP(UploadBufferSize, 64 * 1024 * 1024);

	D3D12MA::ALLOCATION_DESC AllocDesc{};
	AllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

	D3D_CHECK_FAILURE(alloc->CreateResource3(
		&AllocDesc,
		&Desc,
		D3D12_BARRIER_LAYOUT_UNDEFINED,
		nullptr,
		0, nullptr,
		&p_UploadBuffer,
		IID_NULL,
		nullptr
	));

	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU. // we also never need to unmap this as we are are perpertually gonna be be wrting to it
	D3D_CHECK_FAILURE(p_UploadBuffer->GetResource()->Map(0, &readRange, reinterpret_cast<void**>(&p_UploadDataBegin)));

	VIRTUAL_BLOCK_DESC VB{};
	VB.Size = ROUND_UP(UploadBufferSize, 64 * 1024 * 1024);

	CreateBlock(&VB, &p_SubAllocator);
}

void UploadManager::Update()
{
	if (!v_PendingUploads.empty())
	{
		while (v_PendingUploads.front().CompletionFenceValue == gD3DContext->m_LatestFenceValue)
		{
			p_SubAllocator->FreeAllocation(v_PendingUploads.front().allocation);
			v_PendingUploads.pop();

			if (v_PendingUploads.empty()) break;
		}
	}
}

std::future<void> UploadManager::UploadBuffer(ID3D12Resource* Dst, uint32_t dstOffset, void* src, size_t size, UploadMode Mode)
{
	if (Mode == UploadMode_Sync)
	{
		UploadData data;

		VIRTUAL_ALLOCATION_DESC desc{};
		desc.Size = size;
		uint64_t Offset;
		p_SubAllocator->Allocate(&desc, &data.allocation, &Offset);
		memcpy(p_UploadDataBegin + Offset, src, size);

		auto cb = gD3DContext->GetDefaultCommandList();

		CD3DX12_BUFFER_BARRIER startbarriers[] =
		{
			CD3DX12_BUFFER_BARRIER(D3D12_BARRIER_SYNC_ALL, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COMMON, D3D12_BARRIER_ACCESS_COPY_DEST, Dst)
		};

		CD3DX12_BARRIER_GROUP group(1, startbarriers);

		cb->Barrier(1, &group);

		cb->CopyBufferRegion(Dst, dstOffset, p_UploadBuffer->GetResource(), Offset, size);

		CD3DX12_BUFFER_BARRIER endBarriers[] =
		{
			CD3DX12_BUFFER_BARRIER(D3D12_BARRIER_SYNC_ALL, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_DEST,D3D12_BARRIER_ACCESS_COMMON , Dst)
		};

		group = CD3DX12_BARRIER_GROUP(1, endBarriers);

		cb->Barrier(1, &group);

		data.CompletionFenceValue = gD3DContext->m_LatestFenceValue + 1;
		v_PendingUploads.push(data);
	}
	else
	{ //UploadMode_ASync:
	//	CommandBufferMemHandle CommandBuffer;
	};

	return std::future<void>();
}

std::future<void> UploadManager::UploadBuffer(ID3D12Resource* Dst, uint32_t dstOffset, size_t size, std::function<void(void*)> Writer, UploadMode Mode)
{
	if (Mode == UploadMode_Sync)
	{
		UploadData data;

		VIRTUAL_ALLOCATION_DESC desc{};
		desc.Size = size;
		uint64_t Offset;
		p_SubAllocator->Allocate(&desc, &data.allocation, &Offset);

		Writer(p_UploadDataBegin + Offset + dstOffset);

		auto cb = gD3DContext->GetDefaultCommandList();

		CD3DX12_BUFFER_BARRIER startbarriers[] =
		{
			CD3DX12_BUFFER_BARRIER(D3D12_BARRIER_SYNC_ALL, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COMMON, D3D12_BARRIER_ACCESS_COPY_DEST, Dst)
		};

		CD3DX12_BARRIER_GROUP group(1, startbarriers);

		cb->Barrier(1, &group);

		cb->CopyBufferRegion(Dst, dstOffset, p_UploadBuffer->GetResource(), Offset, size);

		CD3DX12_BUFFER_BARRIER endBarriers[] =
		{
			CD3DX12_BUFFER_BARRIER(D3D12_BARRIER_SYNC_ALL, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_DEST,D3D12_BARRIER_ACCESS_COMMON , Dst)
		};

		group = CD3DX12_BARRIER_GROUP(1, endBarriers);

		cb->Barrier(1, &group);

		data.CompletionFenceValue = gD3DContext->m_LatestFenceValue + 1;
		v_PendingUploads.push(data);
	}
	else
	{ //UploadMode_ASync:
	//	CommandBufferMemHandle CommandBuffer;
	};

	return std::future<void>();
}
