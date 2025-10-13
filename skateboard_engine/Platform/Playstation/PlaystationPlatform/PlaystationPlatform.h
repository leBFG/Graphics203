#pragma once
#include "AGC/Graphics/AGCF.h"
#include "Skateboard/Platform.h"

#pragma comment(lib, "SceAgcDriver_stub_weak")
#pragma comment(lib, "SceAgc_stub_weak")
#pragma comment(lib, "SceVideoOut_stub_weak")
//#pragma comment(lib, "libScePackParser")
//#pragma comment(lib, "libedgeanimtool")

#ifndef SKTBD_SHIP
#pragma comment(lib, "SceAgc_debug_nosubmission")
#pragma comment(lib, "SceAgcCore_debug_nosubmission")
#pragma comment(lib, "SceAgcGpuAddress_debug_nosubmission")
#pragma comment(lib, "SceMat_nosubmission_stub_weak")
#endif


#ifdef SKTBD_ENTRY_POINT
//TODO Maybe put this into entry point ps5


// Ensure that the heap size will be extended when running out of memory on an allocation
unsigned int sceLibcHeapExtendedAlloc = 1;
// Disable the upper limit of the heap area. Alternatively, this could be set to a value such as 1*1024*1024 for an upper limit of 1MB.
// Read: https://p.siedev.net/resources/documents/SDK/7.000/C_and_Cpp_standard_libraries/stdlib.html#malloc
size_t sceLibcHeapSize = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;
#endif


namespace Skateboard
{
	class PlaystationPlatform final : public Platform
	{
	public:
		DISABLE_COPY_AND_MOVE(PlaystationPlatform);

		PlaystationPlatform();
		virtual ~PlaystationPlatform() final override;

		void Init(const PlatformProperties& props) final override;

		// Bind main event call back function
		//void SetOnEventCallback(std::function<void(Event&)> callback) final override;

		// Update the system app
		virtual bool Update() final override;
		virtual void OnEvent(Event& e) final override;

		virtual void InitImGui() final override;
		virtual void BeginImGuiPass() final override;
		virtual void EndImGuiPass() final override;
		virtual void ShutdownImGui() final override;

	private:

		void SetupImGuiViewport(const sce::Agc::CxRenderTarget* renderTarget);

	private:

		struct PlatformData
		{
			std::string Title;
			uint32_t BackBufferWidth, BackBufferHeight;
			bool VSync;
			bool Fullscreen;
			bool Windowed;
			bool IsResizing;
			bool IsClosing;
		};
		
		PlatformData m_PlatformData;

		float m_ClockFrequency;
		float m_UIScale;
	};

	
}