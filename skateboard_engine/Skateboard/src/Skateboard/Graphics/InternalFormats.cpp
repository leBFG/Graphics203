#include <sktbdpch.h>
#include "InternalFormats.h"

#define SKTBD_LOG_COMPONENT "INTERNAL DATA TYPES"
#include "Skateboard/Log.h"

// Note: These switches may seem rather unefficient, but modern compiler will try to optimise them into nice lookup tables for O(1) conversion
// Read more here:
//		https://parses.wordpress.com/2017/02/21/c-switch-as-a-lookup-table/
//		https://stackoverflow.com/questions/931890/what-is-more-efficient-a-switch-case-or-an-stdmap

namespace Skateboard
{
	uint32_t ShaderDataTypeSizeInBytes(ShaderDataType_ type)

	{
		switch (type)
		{
		case ShaderDataType_::None:			return 0u;
		case ShaderDataType_::Bool:			return 1 * sizeof(bool);
		case ShaderDataType_::Int:			return 1 * sizeof(int32_t);
		case ShaderDataType_::Int2:			return 2 * sizeof(int32_t);
		case ShaderDataType_::Int3:			return 3 * sizeof(int32_t);
		case ShaderDataType_::Int4:			return 4 * sizeof(int32_t);
		case ShaderDataType_::Uint:			return 1 * sizeof(uint32_t);
		case ShaderDataType_::Uint2:		return 2 * sizeof(uint32_t);
		case ShaderDataType_::Uint3:		return 3 * sizeof(uint32_t);
		case ShaderDataType_::Uint4:		return 4 * sizeof(uint32_t);
		case ShaderDataType_::Float:		return 1 * sizeof(float);
		case ShaderDataType_::Float2:		return 2 * sizeof(float);
		case ShaderDataType_::Float3:		return 3 * sizeof(float);
		case ShaderDataType_::Float4:		return 4 * sizeof(float);
		default:
			SKTBD_LOG_ASSERT(false, "Impossible shader data type!");
			return 0u;
		}
	}
}