#include <sktbdpch.h>
#include "MemoryManager.h"

#define SKTBD_LOG_COMPONENT "MEMORY_MANAGER_IMPL"
#include "Skateboard/Log.h"

namespace Skateboard
{
	//MemoryManagerData MemoryManager::s_Data;

	//MemoryManagerData::~MemoryManagerData()
	//{
	//	for (SizedPtr& ptr : v_UploadBufferPendingDatas)
	//	{
	//		free(ptr.Ptr);
	//		ptr = { nullptr, 0u };
	//	}
	//}
	//
	//uint32_t MemoryManager::CreateBuffer(const std::wstring& debugName, BufferDesc  desc, bool frameResource, std::function<RawBuffer* (const std::wstring& debugName, const BufferDesc& desc)> BufferConstructor)
	//{
	//	if (s_Data.v_FreedBufferIndices.size())
	//	{
	//		const uint32_t returnVal = s_Data.v_FreedBufferIndices.front();
	//		s_Data.v_FreedBufferIndices.pop();

	//		if (frameResource)
	//		{
	//			desc.SubResourceCount = GRAPHICS_SETTINGS_NUMFRAMERESOURCES;
	//		}

	//		s_Data.v_UploadBuffers[returnVal].FrameResource = frameResource;
	//		s_Data.v_UploadBuffers[returnVal].Buffer.reset(BufferConstructor(debugName,desc));

	//		s_Data.v_UploadBufferPendingDatas[returnVal] = SizedPtr(calloc(desc.ElementCount, desc.ElementSize), desc.ElementSize);

	//		return returnVal;
	//	}
	//	else
	//	{
	//		const uint32_t returnVal = static_cast<uint32_t>(s_Data.v_UploadBufferPendingDatas.size());
	//		SKTBD_LOG_ASSERT(returnVal != UINT32_MAX, "Possible overflow! Did you really mean to create that many upload bufferS?");

	//		s_Data.v_UploadBuffers.emplace_back(MemoryManagedBuffer(frameResource, std::shared_ptr<RawBuffer>(BufferConstructor(debugName, desc ))));

	//		s_Data.v_UploadBufferPendingDatas.emplace_back(SizedPtr(calloc(desc.ElementCount, desc.ElementSize), desc.ElementSize));
	//		return returnVal;
	//	}
	//}

	//bool MemoryManager::ResetBuffer(uint32_t uploadBufferID, BufferDesc desc, bool frameResource, std::function<RawBuffer* (const std::wstring& debugName, const BufferDesc& desc)> BufferConstructor)
	//{
	//	//SKTBD_LOG_ASSERT(uploadBufferID < static_cast<uint32_t>(s_Data.v_UploadBuffers.size()), "Out of bounds access. Verify that you created a buffer with this input uploadBufferID.");

	//	if (uploadBufferID >= static_cast<uint32_t>(s_Data.v_UploadBuffers.size()))
	//	{
	//		SKTBD_MSG_WARN("Upload buffer ID out of Range: {}", uploadBufferID );
	//		return false;
	//	}

	//	const std::wstring& debugName = s_Data.v_UploadBuffers[uploadBufferID].Buffer->GetDebugName();
	//	s_Data.v_UploadBuffers[uploadBufferID].FrameResource = frameResource;
	//	s_Data.v_UploadBuffers[uploadBufferID].Buffer.reset(BufferConstructor(debugName, desc));
	//	return true;
	//}

	//bool MemoryManager::DestroyBuffer(uint32_t uploadBufferID)
	//{
	//	if (auto buffer = s_Data.v_UploadBuffers[uploadBufferID].Buffer ; buffer)
	//	{
	//		buffer->Release();

	//		s_Data.v_BuffersDirty.erase(uploadBufferID);
	//		s_Data.v_UploadBuffers[uploadBufferID].Buffer.reset();
	//		s_Data.v_FreedBufferIndices.push(uploadBufferID);
	//		free(s_Data.v_UploadBufferPendingDatas[uploadBufferID].Ptr);
	//		s_Data.v_UploadBufferPendingDatas[uploadBufferID].Ptr = nullptr;
	//		s_Data.v_UploadBufferPendingDatas[uploadBufferID].Size = 0;
	//		return true;
	//	}

	//	SKTBD_MSG_ERROR("Buffer ID is already unpopulated");
	//	return false;
	//}


	//void MemoryManager::Update()
	//{
	//	// Manage upload buffers so that they are updated for all following frame resources after a change
	//	for (auto it = s_Data.v_BuffersDirty.begin(); it != s_Data.v_BuffersDirty.end();)
	//	{
	//		const uint32_t& bufferID = it->first;
	//		uint32_t& numFramesDirty = it->second;

	//		auto& currentBuffer = s_Data.v_UploadBuffers[bufferID].Buffer;
	//		const uint32_t elementCount = currentBuffer->ElementCount;

	//		auto FrameIndex = Platform::GetPlatform().GetGraphicsContext()->GetCurrentFrameResourceIndex();

	//		//currentBuffer->CopyData(0, s_Data.v_UploadBufferPendingDatas[bufferID].Ptr);
	//		for (uint32_t i = 0u; i < elementCount; ++i)
	//		{
	//			uint8_t* ptr = static_cast<uint8_t*>(s_Data.v_UploadBufferPendingDatas[bufferID].Ptr) + s_Data.v_UploadBufferPendingDatas[bufferID].Size * i;
	//			currentBuffer->CopyData(FrameIndex, i, ptr);
	//		}

	//		--numFramesDirty;
	//		if (numFramesDirty == 0u)
	//			it = s_Data.v_BuffersDirty.erase(it);
	//		else ++it;
	//	}
	//}

	//void MemoryManager::Clean()
	//{
	//	for(auto& buffer:s_Data.v_UploadBuffers)
	//	{
	//		buffer.Buffer->Release();
	//	}

	//	s_Data.v_UploadBuffers.clear();
	//	s_Data.v_UploadBufferPendingDatas.clear();
	//	s_Data.v_BuffersDirty.clear();
	//}

	//void MemoryManager::UploadData(uint32_t uploadBufferID, uint32_t elementIndex, void* pData)
	//{
	//	SKTBD_LOG_ASSERT(uploadBufferID < static_cast<uint32_t>(s_Data.v_UploadBufferPendingDatas.size()), "Out of bounds access. Verify that you created a buffer with this input uploadBufferID.");
	//	SKTBD_LOG_ASSERT(uploadBufferID < static_cast<uint32_t>(s_Data.v_UploadBuffers.size()), "Out of bounds access. Verify that you created a buffer with this input uploadBufferID.");
	//	SKTBD_LOG_ASSERT(elementIndex < s_Data.v_UploadBuffers[uploadBufferID].Buffer->ElementCount, "Out of bounds access. Invalid element index.");

	//	if (s_Data.v_UploadBuffers[uploadBufferID].FrameResource)
	//	{
	//		uint8_t* bufferStart = static_cast<uint8_t*>(s_Data.v_UploadBufferPendingDatas[uploadBufferID].Ptr) + elementIndex * s_Data.v_UploadBuffers[uploadBufferID].Buffer->ElementSize;
	//		memcpy(bufferStart, pData, s_Data.v_UploadBufferPendingDatas[uploadBufferID].Size);
	//		if (s_Data.v_BuffersDirty.count(uploadBufferID))
	//			s_Data.v_BuffersDirty[uploadBufferID] = GRAPHICS_SETTINGS_NUMFRAMERESOURCES;
	//		else
	//			s_Data.v_BuffersDirty.insert({ uploadBufferID, GRAPHICS_SETTINGS_NUMFRAMERESOURCES });
	//	}
	//	else //THIS IS an awfull coppy paste beed to find the ASAN error 
	//	{
	//		uint8_t* bufferStart = static_cast<uint8_t*>(s_Data.v_UploadBufferPendingDatas[uploadBufferID].Ptr) + elementIndex * s_Data.v_UploadBuffers[uploadBufferID].Buffer->ElementSize;
	//		memcpy(bufferStart, pData, s_Data.v_UploadBufferPendingDatas[uploadBufferID].Size);
	//		if (s_Data.v_BuffersDirty.count(uploadBufferID))
	//			s_Data.v_BuffersDirty[uploadBufferID] = 1;
	//		else
	//			s_Data.v_BuffersDirty.insert({ uploadBufferID, 1 });
	//	}
	//}

	//RawBuffer* MemoryManager::GetUploadBuffer(uint32_t uploadBufferID)
	//{
	//	SKTBD_LOG_ASSERT(uploadBufferID < static_cast<uint32_t>(s_Data.v_UploadBuffers.size()), "Out of bound access. Verify that you created a buffer with this input uploadBufferID.");
	//	return s_Data.v_UploadBuffers[uploadBufferID].Buffer.get();
	//}

}