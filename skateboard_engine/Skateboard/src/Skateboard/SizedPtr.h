#pragma once
#include <stdint.h>

namespace Skateboard
{
	struct SizedPtr
	{
		SizedPtr() = default;
		SizedPtr(void* ptr, uint32_t size) : Ptr(ptr), Size(size) {}

		void* Ptr = nullptr;
		uint32_t Size = 0u;
	};
}

