#pragma once
#include "Skateboard/Graphics/InternalFormats.h"
#include "Skateboard/Graphics/RHI/GraphicsSettingsDefines.h"
#include "Skateboard/Core.h"


namespace Skateboard
{
	// A 'GpuResource' is a resource that will be transferred to the platform GPU in some way

	class GPUResource
	{
		//DISABLE_COPY_AND_MOVE(GPUResource);
	public:
		GPUResource() = default;

		constexpr virtual GPUResourceType_ GetResourceType() = 0;
		virtual ~GPUResource() = default;

	protected:
#ifndef SKTBD_SHIP
		std::wstring DebugName;
	public:
		virtual void SetDebugName(const std::wstring& debug_name) { DebugName = debug_name; }
#else
		virtual void SetDebugName(const std::wstring& debug_name) { }
#endif
	};

	
	template<class Resource, uint32_t ResourceCount = GRAPHICS_SETTINGS_NUMFRAMERESOURCES>
	class MultiResource
	{
		static_assert(ResourceCount > 1);

	public:
		template<typename... Args>
		MultiResource() : m_internalCounter(0) { };

		static constexpr uint32_t GetHandleCount() { return  ResourceCount; }

		Resource Get() const { return m_MemoryHandles[m_internalCounter]; }

		Resource GetPrevious() const { return m_MemoryHandles[(m_internalCounter - 1) % ResourceCount]; }
		Resource GetNext() const { return m_MemoryHandles[(m_internalCounter + 1) % ResourceCount]; }

		std::array<Resource, ResourceCount>& GetResourceArray()
		{
			return m_MemoryHandles;
		}

		void SetCounter(uint32_t new_idx)
		{
			ASSERT_SIMPLE(new_idx < ResourceCount, "Frame Index Out of Range for This Resource");
			m_internalCounter = new_idx;
		}

		void IncrementCounter()
		{
			m_internalCounter = (m_internalCounter + 1) % ResourceCount;
		}

		void ForEach(std::function<void(Resource&)>func)
		{
			for (auto& i : m_MemoryHandles) func(i);
		}

		Resource& operator[](size_t idx) { return m_MemoryHandles[idx]; }
		const Resource& operator[](size_t idx) const { return m_MemoryHandles[idx]; }

		void operator++()
		{
			IncrementCounter();
		};

	protected:
		std::array<Resource, ResourceCount> m_MemoryHandles;
		uint32_t m_internalCounter;
	};
};