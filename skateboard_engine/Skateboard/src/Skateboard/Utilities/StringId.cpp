#include "sktbdpch.h"
#include "StringId.h"
#include "Skateboard/Utilities/CRC.h"

namespace Skateboard
{

    StringIdTable::StringIdTable()
    {}

    StringIdTable::~StringIdTable()
    {}

    uint32_t StringIdTable::Add(const std::string& str)
    {
        uint32_t stringId = GetId(str);

        auto iter = m_Table.find(stringId);

        if (iter == m_Table.end())
            m_Table[stringId] = str;

        return stringId;
    }

    uint32_t StringIdTable::GetId(const std::string& str)
    {
        return CRC::GetICRC(str.c_str());
    }

    bool StringIdTable::Contains(const uint32_t id, std::string& result)
    {
        auto iter = m_Table.find(id);
        if (iter != m_Table.end())
        {
            result = iter->second;
            return true;
        }
        else
            return false;
    }

}


