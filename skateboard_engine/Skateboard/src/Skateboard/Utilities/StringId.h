#pragma once
#include "Skateboard/Core.h"
#include "sktbdpch.h"
#include <map>

namespace Skateboard
{

	class StringIdTable
	{
	public:
		StringIdTable();
		~StringIdTable();

		uint32_t Add(const std::string& str);
		uint32_t GetId(const std::string& str);

		bool Contains(const uint32_t id, std::string& result);

		const std::map<uint32_t, std::string> GetTable() const { return m_Table; }

	private:
		std::map<uint32_t, std::string> m_Table;
	};

}
