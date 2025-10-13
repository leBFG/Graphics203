#include <sktbdpch.h>
#include "DescriptorTable.h"
#include "Skateboard/Graphics/RHI/GraphicsContext.h"

#define SKTBD_LOG_COMPONENT "DESCRIPTOR TABLE IMPL"
#include "Skateboard/Log.h"

namespace Skateboard
{
	DescriptorTableDesc DescriptorTableDesc::Init(ShaderDescriptorTableType_ type)
	{
		return {
			type
		};
	}

	Descriptor Descriptor::ConstantBufferView(GPUResource* pResource, uint32_t shaderSlot, uint32_t shaderSpace)
	{
		return Init(pResource, ShaderElementType_ConstantBufferView, shaderSlot, shaderSpace);
	}

	Descriptor Descriptor::ShaderResourceView(GPUResource* pResource, uint32_t shaderSlot, uint32_t shaderSpace)
	{
		return Init(pResource, ShaderElementType_ShaderResourceView, shaderSlot, shaderSpace);
	}

	Descriptor Descriptor::UnorderedAccessView(GPUResource* pResource, uint32_t shaderSlot, uint32_t shaderSpace)
	{
		return Init(pResource, ShaderElementType_UnorderedAccessView, shaderSlot, shaderSpace);
	}

	Descriptor Descriptor::Init(GPUResource* pResource, ShaderElementType_ type, uint32_t shaderSlot, uint32_t shaderSpace)
	{
		return {
			shaderSlot,
			shaderSpace,
			type,
			pResource
		};
	}


	DescriptorTable::DescriptorTable(const DescriptorTableDesc& desc) :
		m_Desc(desc),
		m_FrameResourceTracker(0u),
		m_TableGenerated(false)
	{
	}

	void DescriptorTable::AddDescriptor(const Descriptor& descriptor)
	{
		SKTBD_LOG_ASSERT(VerifiyResourceCompatibility(descriptor.ShaderElementType), "Resource type is not compatible with descriptor table type");
		SKTBD_LOG_ASSERT(!m_TableGenerated, "Cannot add a descriptor when the table was already generated!");

		// Create a hash for the descriptor for easy indexing in the map
		const uint64_t hash = ComputeHash(descriptor.ShaderElementType, descriptor.ShaderRegister, descriptor.ShaderRegisterSpace);
		SKTBD_LOG_ASSERT(m_Table.count(hash) == 0u, "Cannot add this descriptor. The shader element already exists in the table. Check\
			that you are not overloading the same shader register (ex: register(b0, space0) where b = ShaderElementType_, 0 = slot, space0 = space)");

		// Place the descriptor in the table and register its position for update
		m_TableIndices.insert(std::pair<uint64_t, uint64_t>(hash, m_Table.size()));
		m_Table.insert(std::pair<uint64_t, Descriptor>(hash, descriptor));

		// This descriptor will now also need to be updated for further N-1 frame resources
		constexpr uint64_t numUpdates = GRAPHICS_SETTINGS_NUMFRAMERESOURCES - 1u;
		if (numUpdates)
			m_DefferredUpdates.insert(std::pair<uint64_t, uint64_t>(hash, numUpdates));
	}

	void DescriptorTable::UpdateDescriptor(const Descriptor& descriptor)
	{
		SKTBD_LOG_ASSERT(VerifiyResourceCompatibility(descriptor.ShaderElementType), "Resource type is not compatible with descriptor table type");
		SKTBD_LOG_ASSERT(m_TableGenerated, "Cannot update a descriptor when the table was not generated!");
		const uint64_t hash = ComputeHash(descriptor.ShaderElementType, descriptor.ShaderRegister, descriptor.ShaderRegisterSpace);
		SKTBD_LOG_ASSERT(m_Table.count(hash) != 0u, "Cannot update a descriptor that doesn't exists! Ensure that this descriptor was added\
			before the call to GenerateTable().");

		// Perform update
		m_Table.at(hash) = descriptor;
		UpdateTableEntry(hash);

		// This descriptor will now also need to be updated for further N-1 frame resources
		constexpr uint64_t numUpdates = GRAPHICS_SETTINGS_NUMFRAMERESOURCES - 1u;
		if(numUpdates)
			m_DefferredUpdates.insert_or_assign(hash, numUpdates);
	}

	GPUResource* DescriptorTable::GetResource(ShaderElementType_ type, uint32_t shaderSlot, uint32_t shaderSpace) const
	{
		const uint64_t hash = ComputeHash(type, shaderSlot, shaderSpace);
		return m_Table.at(hash).pResource;
	}

	void DescriptorTable::GenerateTable()
	{
		// Loop through all the descriptors and copy their resource to the table
		for (const std::pair<uint64_t, Descriptor>& item : m_Table)
			UpdateTableEntry(item.first);

		// Flag that the initial generation was complete
		m_TableGenerated = true;
	}

	void DescriptorTable::ProcessDefferredUpdates()
	{
		// This function will replace the pending descriptors for this frame resource.
		// However, it because redundant when replacing the descriptors each frame anyway and
		// therefore the copy happens twice. Not the most efficient way of doing this, but it works for now.
		// Loop through all the descriptors and copy their resource to the table. This is the LAZY way of handling this
		auto it = m_DefferredUpdates.begin();
		while (it != m_DefferredUpdates.end())
		{
			const uint64_t& hash = it->first;
			uint64_t& numFramesDirty = it->second;

			UpdateTableEntry(hash);

			--numFramesDirty;
			if (numFramesDirty == 0u)
				it = m_DefferredUpdates.erase(it);
			else ++it;
		}
	}

	uint64_t DescriptorTable::ComputeHash(ShaderElementType_ type, uint32_t shaderSlot, uint32_t shaderSpace) const
	{
		return static_cast<uint64_t>(type) << 32 | static_cast<uint64_t>(shaderSlot) << 16 | shaderSpace;
	}

	bool DescriptorTable::VerifiyResourceCompatibility(ShaderElementType_ type) const
	{
		switch (type)
		{
		case ShaderElementType_ConstantBufferView:
		case ShaderElementType_ShaderResourceView:
		case ShaderElementType_UnorderedAccessView:
			return m_Desc.Type == ShaderDescriptorTableType_CBV_SRV_UAV;
	//	case ShaderElementType_Sampler:
			//return m_Desc.Type == ShaderDescriptorTableType_Sampler;
		default:
			return false;
		}
	}
}