#pragma once
#include "Skateboard/Memory/DescriptorTable.h"
#include "Skateboard/SizedPtr.h"

namespace Skateboard
{
	class AGCDescriptorTable final : public DescriptorTable
	{
	public:
		AGCDescriptorTable(const DescriptorTableDesc& desc);
		virtual ~AGCDescriptorTable() final override;

		virtual void GenerateTable() final override;

		SizedPtr GetTable();

	private:
		virtual void UpdateTableEntry(uint64_t hash) final override;

	private:
		SizedPtr m_AGCTable;
		uint32_t m_DescriptorSize;
	};
}