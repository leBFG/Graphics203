#pragma once
#include "Skateboard/SizedPtr.h"
#include "Skateboard/Graphics/Resources/CommonResources.h"
#include "Skateboard/Graphics/RHI/ResourceFactory.h"

namespace Skateboard
{
	//struct MemoryManagedBuffer
	//{
	//	MemoryManagedBuffer() = default;
	//	MemoryManagedBuffer(bool fr, std::shared_ptr<GPUResource> b) : FrameResource(fr), Buffer(b) {};

	//	bool FrameResource;
	//	std::shared_ptr<GPUResource> Buffer;
	//};

	//struct MemoryManagerData
	//{
	//	~MemoryManagerData();

	//	std::vector<MultiResource<Buffer_Base>>					v_UploadBuffers;			// A container of all the upload buffers

	//	std::vector<SizedPtr>									v_UploadBufferPendingDatas;	// A container of cpu buffers with the data to be transferred to the GPU
	//	std::unordered_map<uint32_t, uint32_t>					v_BuffersDirty;				// pair< bufferID , numFramesDirty > (Keeps track of the number of frames a buffer is dirty)
	//	std::queue<uint32_t>									v_FreedBufferIndices;
	//};

	//class MemoryManager
	//{
	//	
	//public:
	//	/// <summary>
	//	/// Will create a buffer under the management of Memory Manager
	//	/// </summary>
	//	/// <param name="debugName"> debug name of the buffer which will register with debugging tools</param>
	//	/// <param name="desc"> BufferDesc containing the num and size of the elements as well as common acces patter for the buffer</param>
	//	/// <param name="frameResource"> Will tell the manager that this buffer is a frame resource and thus its size will be * GRAPHICS_SETTINGS_NUMFRAMERESOURCES, for user when Get buffer is called it will still appear as single direct buffe </param>
	//	/// <param name="function that return a RawBuffer* Implementation for a given platform"></param>
	//	/// <returns>32bit ID of the buffer</returns>
	//	[[nodiscard]] static uint32_t CreateBuffer(const std::wstring& debugName, BufferDesc desc, bool frameResource = true, std::function<MultiResource<Buffer_Base>* (const std::wstring& debugName, const BufferDesc& desc)> BufferConstructor = ResourceFactory::CreateBufferResource<MultiResource<Buffer_Base>>);

	//	

	//	/// <summary>
	//	/// Same As Create Function except it allows to reuse already existing Buffer ID under the management of the memory manager
	//	/// </summary>
	//	/// <returns> returns weather the reset have been performed succesfully</returns>
	//	static bool ResetBuffer(uint32_t uploadBufferID, BufferDesc desc, bool frameResource = true, std::function<GPUResource* (const std::wstring& debugName, const BufferDesc& desc)> BufferFactory = BufferFactory::CreateBuffer);

	//	/// <summary>
	//	/// Queues the buffer for deletion and removes it fro the management of the memory manager
	//	/// </summary>
	//	/// <param name="uploadBufferID"></param>
	//	/// <returns></returns>
	//	static bool DestroyBuffer(uint32_t uploadBufferID);

	//	/// <summary>
	//	/// 
	//	/// </summary>
	//	static void Update();

	//	/// <summary>
	//	/// Destroys all Resources currently under management of resource manager
	//	/// </summary>
	//	static void Clean();

	//	/// <summary>
	//	/// Function that updates the buffer data under given ID weather it is a frame rsource or not
	//	/// </summary>
	//	/// <param name="uploadBufferID"></param>
	//	/// <param name="elementIndex"></param>
	//	/// <param name="pData"></param>
	//	static void UploadData(uint32_t uploadBufferID, uint32_t elementIndex, void* pData);

	//	/// <summary>
	//	/// returns 
	//	/// </summary>
	//	/// <param name="uploadBufferID"></param>
	//	/// <returns></returns>
	//	static View* GetUploadBuffer(uint32_t uploadBufferID);

	//private:
	//	static MemoryManagerData s_Data;
	//};
}