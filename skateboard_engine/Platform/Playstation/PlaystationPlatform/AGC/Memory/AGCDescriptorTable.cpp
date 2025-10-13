
#include "AGC/Graphics/AGCF.h"
#include "AGCDescriptorTable.h"
#include "AGC/Graphics/Resources/AGCBuffer.h"
#include "AGC/Graphics/RHI/AGCGraphicsContext.h"

#define SKTBD_LOG_COMPONENT "DESCRIPTOR_TABLE_IMPL"

#include "Skateboard/Log.h"

namespace Skateboard
{
	AGCDescriptorTable::AGCDescriptorTable(const DescriptorTableDesc& desc) :
		DescriptorTable(desc),
		m_AGCTable{},
		m_DescriptorSize(0u)
	{
	}

	AGCDescriptorTable::~AGCDescriptorTable()
	{
		if (m_AGCTable.Ptr)
			free(m_AGCTable.Ptr);
	}

	void AGCDescriptorTable::GenerateTable()
	{
		SKTBD_LOG_ASSERT(!m_TableGenerated, "Table already generated.");

		// Generating a descriptor table in AGC means having a copy of the resources one after the other
		//TODO: m_DescriptorSize = m_Desc.Type == ShaderDescriptorTableType_Sampler ? sizeof(sce::Agc::Core::Sampler) : sizeof(sce::Agc::Core::Texture);
		m_DescriptorSize = sizeof(sce::Agc::Core::Texture);
		m_AGCTable.Size = m_DescriptorSize * static_cast<uint32_t>(m_Table.size());
		m_AGCTable.Ptr = malloc(m_AGCTable.Size * GRAPHICS_SETTINGS_NUMFRAMERESOURCES);
		SKTBD_LOG_ASSERT(m_AGCTable.Ptr != nullptr, "Failed to allocate memory for AGC descriptor table.");

		// Terminate generation with base class
		DescriptorTable::GenerateTable();
	}

	SizedPtr AGCDescriptorTable::GetTable()
	{
		SKTBD_LOG_ASSERT(m_AGCTable.Ptr != nullptr, "Invalid call on empty table. Did you call GenerateTable()?");

		// If the frame index changed, then we need to update the table with the current frame resources.
		// Loop through all the descriptors and copy their resource to the table. This is the LAZY way of handling this
		uint64_t frameIndex = gAGCContext->GetCurrentFrameResourceIndex();
		if (m_FrameResourceTracker != frameIndex)
		{
			m_FrameResourceTracker = frameIndex;
			ProcessDefferredUpdates();
		}

		// Return the section of the table for this frame resource
		SizedPtr ret = m_AGCTable;
		ret.Ptr = static_cast<uint8_t*>(ret.Ptr) + ret.Size * frameIndex;
		return ret;
	}

	void AGCDescriptorTable::UpdateTableEntry(uint64_t hash)
	{
		SKTBD_LOG_ASSERT(m_AGCTable.Ptr != nullptr, "Invalid call on empty table. Did you call GenerateTable() before updating an existing entry?");

		// Retrieve the descriptor and its memory location in the table
		const Descriptor& descriptor = m_Table.at(hash);
		const uint64_t tableIndex = m_TableIndices.at(hash);
		uint8_t* pCurrentDescriptor = static_cast<uint8_t*>(m_AGCTable.Ptr) +
			tableIndex * m_DescriptorSize + gAGCContext->GetCurrentFrameResourceIndex() * m_AGCTable.Size;

		// Perform update
		//TODO: if (m_Desc.Type == ShaderDescriptorTableType_Sampler) { ... } else..

		GPUResource* pRes = descriptor.pResource;
	//	switch (pRes->GetType())
	//	{ 
	////	case GPUResourceType_DefaultBuffer:
	////	case GPUResourceType_UploadBuffer:
	////	case GPUResourceType_Buffer:
	//		memcpy(pCurrentDescriptor, pRes->GetDefaultDescriptor(), sizeof(sce::Agc::Core::Buffer));
	//		break;
	//	case GPUResourceType_Texture2D:
	//	case GPUResourceType_Texture3D:
	//	case GPUResourceType_Texture1D:
	//		memcpy(pCurrentDescriptor, pRes->GetDefaultDescriptor(), sizeof(sce::Agc::Core::Texture));
	//		break;
	//	case GPUResourceType_FrameBuffer:
	//		memcpy(m_AGCTable.Ptr, (sce::Agc::Core::Texture*)pRes->GetDefaultDescriptor()+gAGCContext->GetCurrentFrameResourceIndex() * sizeof(sce::Agc::Core::Texture), sizeof(sce::Agc::Core::Texture));
	//		break;
	//	case GPUResourceType_UnorderedAccessBuffer:
	//case GPUResourceType_Buffer:
	//memcpy(pCurrentDescriptor, pRes->GetDefaultDescriptor(), sizeof(sce::Agc::Core::Texture));
	//		break;
	//	default:
	//		SKTBD_LOG_ASSERT(false, "Invalid resource type.");
	//	}
	}
}