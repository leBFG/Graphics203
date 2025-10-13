#pragma once
#include "Skateboard/Graphics/InternalFormats.h"
#include "Skateboard/Graphics/Resources/GPUResource.h"
#include <unordered_map>

namespace Skateboard
{
	struct DescriptorTableDesc
	{
		ShaderDescriptorTableType_ Type;

		static DescriptorTableDesc Init(ShaderDescriptorTableType_ type);
	};

	struct Descriptor
	{
		uint32_t ShaderRegister;
		uint32_t ShaderRegisterSpace;
		ShaderElementType_ ShaderElementType;
		GPUResource* pResource;

		static Descriptor ConstantBufferView(GPUResource* pResource, uint32_t shaderSlot, uint32_t shaderSpace);
		static Descriptor ShaderResourceView(GPUResource* pResource, uint32_t shaderSlot, uint32_t shaderSpace);
		static Descriptor UnorderedAccessView(GPUResource* pResource, uint32_t shaderSlot, uint32_t shaderSpace);
		static Descriptor Init(GPUResource* pResource, ShaderElementType_ type, uint32_t shaderSlot, uint32_t shaderSpace);
	};

	class DescriptorTable
	{
	protected:
		DescriptorTable(const DescriptorTableDesc& desc);
	public:
		virtual ~DescriptorTable() {}
		static DescriptorTable* Create(const DescriptorTableDesc& desc);

		void AddDescriptor(const Descriptor& descriptor);
		void UpdateDescriptor(const Descriptor& descriptor);
		GPUResource* GetResource(ShaderElementType_ type, uint32_t shaderSlot, uint32_t shaderSpace) const;
		virtual void GenerateTable();
		bool IsEmpty() const { return m_Table.empty(); }

		ShaderDescriptorTableType_ GetTableType() const { return m_Desc.Type; }
		uint32_t GetDescriptorCount() const { return static_cast<uint32_t>(m_Table.size()); };

		// Iterators for nice C++ functionalities
		std::unordered_map<uint64_t, Descriptor>::iterator begin() { return m_Table.begin(); }
		std::unordered_map<uint64_t, Descriptor>::iterator end() { return m_Table.end(); }
		std::unordered_map<uint64_t, Descriptor>::const_iterator begin() const { return m_Table.begin(); }
		std::unordered_map<uint64_t, Descriptor>::const_iterator end() const { return m_Table.end(); }

	protected:
		virtual void UpdateTableEntry(uint64_t hash) = 0;
		void ProcessDefferredUpdates();
		uint64_t ComputeHash(ShaderElementType_ type, uint32_t shaderSlot, uint32_t shaderSpace) const;
		bool VerifiyResourceCompatibility(ShaderElementType_ type) const;

	protected:
		DescriptorTableDesc m_Desc;
		std::unordered_map<uint64_t, Descriptor> m_Table;
		std::unordered_map<uint64_t, uint64_t> m_TableIndices;
		std::unordered_map<uint64_t, uint64_t> m_DefferredUpdates;
		uint64_t m_FrameResourceTracker;
		bool m_TableGenerated;
	};
}