#pragma once

#include "Graphics/D3D.h"
#include "D3D12MemoryAllocator/include/D3D12MemAlloc.h"

#include "Skateboard/Memory/VirtualAllocator.h"

using namespace Skateboard;
using namespace Skateboard::MemoryUtils;

//class UploadManager;

enum UploadMode
{
	UploadMode_Sync,
	UploadMode_ASync,
};

class UploadManager
{
	
	struct UploadData
	{
		VirtualAllocation allocation;
		uint64_t CompletionFenceValue;

		UploadManager* Manager;
	};


	D3D12MA::Allocation* p_UploadBuffer;
	UINT8* p_UploadDataBegin;

	BlockAllocator* p_SubAllocator;
	std::queue<UploadData> v_PendingUploads;
	 
public:
	void Init(D3D12MA::Allocator*, const size_t UploadBufferSize);
	void Update();
	std::future<void> UploadBuffer(ID3D12Resource* Dst, uint32_t dstOffset, void* src, size_t size, UploadMode Mode = UploadMode_Sync);
	std::future<void> UploadBuffer(ID3D12Resource* Dst, uint32_t dstOffset, size_t size, std::function<void(void*)> Writer, UploadMode Mode = UploadMode_Sync);
};

