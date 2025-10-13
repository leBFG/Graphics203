#include "sktbdpch.h"
#include "Buffer.h"

#define SKTBD_LOG_COMPONENT "BUFFER"
#include "Skateboard/Log.h"

namespace Skateboard
{
	uint32_t BufferElement::GetComponentCount() const
	{
		switch (DataType)
		{
		case ShaderDataType_::None:
			return 0u;
		case ShaderDataType_::Bool:
		case ShaderDataType_::Int:
		case ShaderDataType_::Uint:
		case ShaderDataType_::Float:
			return 1;
		case ShaderDataType_::Int2:
		case ShaderDataType_::Uint2:
		case ShaderDataType_::Float2:
			return 2;
		case ShaderDataType_::Int3:
		case ShaderDataType_::Uint3:
		case ShaderDataType_::Float3:
			return 3;
		case ShaderDataType_::Int4:
		case ShaderDataType_::Uint4:
		case ShaderDataType_::Float4:
			return 4;
		default:
			SKTBD_LOG_ASSERT(false, "Could not get component count on this buffer element, impossible shader data type!");
			return 0u;
		}
	}

	uint32_t BufferLayout::GetStride(uint8_t slot) const
	{

		if (m_SlotsAndStrides.contains(slot))
		{
			return m_SlotsAndStrides.at(slot);
		}
		SKTBD_MSG_DEBUG("BufferLayout doesn't use the specified slot");
		return 0;
		
	}

	uint32_t BufferLayout::GetAbsoluteStride() const
	{
		uint32_t Stide = 0;
		for(auto [slot, stride] : m_SlotsAndStrides)
		{
			Stide += stride;
		}
		return Stide;
	}

	void BufferLayout::CalculateOffsetsAndStride()
	{
		uint32_t offset = 0u;
		for (BufferElement& element : v_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_SlotsAndStrides[element.InputSlot] += element.Size;

			SKTBD_LOG_ASSERT((element.Semantic == POSITION && element.DataType == ShaderDataType_::Float3) || element.Semantic != POSITION, "The position semantic can only accept a vertex type of 3 regular floats. Please change your layout.");
		}
	}
}