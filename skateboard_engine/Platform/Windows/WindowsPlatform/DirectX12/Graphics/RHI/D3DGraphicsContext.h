#pragma once

#include "sktbdpch.h"

#include "Skateboard/Graphics/RHI/GraphicsContext.h"

#include "D3DRenderCommand.h"
#include "D3DResourceFactory.h"

#include "Skateboard/Platform.h"
#include "Graphics/D3DTypes.h"
#include "Graphics/API/D3DDescriptorHeap.h"
#include "Graphics/API/UploadManager.h"

#include "Skateboard/Memory/VirtualAllocator.h"

#ifndef SKTBD_DESKTOP_PLATFORM_MIN_BUFFER_ALIGNMENT
#define SKTBD_DESKTOP_PLATFORM_MIN_BUFFER_ALIGNMENT (64*1024)
#endif // !SKTBD_DESKTOP_PLATFORM_MIN_BUFFER_ALIGNMENT

#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#define D3D12MA_OPTIONS16_SUPPORTED 1

#include "D3D12MemoryAllocator/include/D3D12MemAlloc.h"

using namespace Microsoft::WRL;

#define D3D_DEVICE_REMOVED_EXTENDED_DATA_ENABLE_FLAG 0b1

namespace Skateboard
{
	//easy access d3dContext
	extern D3DGraphicsContext* gD3DContext;

	class D3DGraphicsContext final : public GraphicsContext
	{
		friend class UploadManager;
		friend class D3DRenderCommand;
		friend class D3DResourceFactory;

	public:
		//Flags
		uint32_t m_Flags = D3D_DEVICE_REMOVED_EXTENDED_DATA_ENABLE_FLAG;

	protected:
		RenderCommand* GetAPI() final override { return &D3D_API; };
		ResourceFactory* GetResourceFactory() final override { return &D3D_RESOURCE_FACTORY; };

	public:
		// Let's not make things confusing and initialise everything in the constructor
		D3DGraphicsContext(HWND window, const PlatformProperties& props);
		// And release all in destructor
		virtual ~D3DGraphicsContext() final override;

		//---------------------------------------OVERRIDES

		// Public functions to resize the buffers according to the new dimensions stored in lParam based on the size description in wParam
		void Resize_(int clientWidth, int clientHeight) final override;
		//void OnResized_() final override;

		virtual void SetRenderTargetToBackBuffer_() final override;

		virtual void Update_() final override;
		virtual void BeginFrame_() final override;
		virtual void EndFrame_() final override;

		virtual void Reset_() override;
		virtual void Flush_() final override;
		virtual void WaitUntilIdle_() final override;
		//virtual void Present_() final override;

		virtual bool IsRaytracingSupported_() final override { return m_HasDXR; }
		virtual bool AreWorkGraphsSupported_() final override { return false; };
		virtual bool IsUnifiedMemoryArchitecture_() final override { return p_MemoryAllocator->IsUMA(); };

		virtual CopyResult CopyDataToBuffer_(Buffer* dest, off_t offset, size_t size, void* src)final override;
		virtual CopyResult CopyDataToBuffer_(Buffer* dest, off_t offset, size_t size, std::function<void(void*)> WriterFunct) final override;

		void SubmitCompute_(const ComputeSubmitInfo& submit) override;
		void SubmitGraphics_(const GraphicsSubmitInfo& submit) override;

		void GraphicsSignalFence_(Fence* fence, uint64_t value) override;
		void ComputeSignalFence_(Fence* fence, uint64_t value) override;

		void GraphicsWaitFence_(Fence* fence, uint64_t value) override;
		void ComputeWaitFence_(Fence* fence, uint64_t value) override;

		// Inherited via GraphicsContext
		ShaderIdentifier GetShaderIdentifier(const Pipeline* Pipeline, const std::wstring& ShaderName) override;
		virtual CopyResult WriteTopLevelASInstanceDataToBuffer_(Buffer* dest, off_t offset, size_t num, TLASInstanceData* data) override;
		virtual RaytracingASSizeInfo QueryAccelerationStructureSizeReq_(const AccelerationStructureDesc& AS_Desc, SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS Flags) override;
		virtual ShaderTable BuildShaderTable_(const BufferRef& Target, off_t offset, ComputePipeline* pso, const RaytracingPipelineDesc& m_desc, ShaderTableGroup Group) override;

		//---------------------------------------------------END OVERRIDES

		void NextBackBuffer()
		{
			m_CurrentBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();
			//m_CurrentBackBuffer = (m_CurrentBackBuffer + 1) % g_SwapChainBufferCount;
		}
		
		IDXGISwapChain* SwapChain() const { return m_SwapChain.Get(); }

		DXGI_FORMAT GetBackBufferFormat() const { return m_BackBufferFormat; }
		DXGI_FORMAT GetDepthStencilFormat() const { return m_DepthStencilFormat; }

		ID3D12DescriptorHeap* const GetSRVHeap() const { return m_SRVDescriptorHeap.GetHeap(); }
		ID3D12DescriptorHeap* const GetSamplerHeap() const { return m_SamplerHeap.GetHeap(); }

		D3DDescriptorHandle GetImGuiDescriptorHandle() const { return m_ImGuiHandle; }

		ID3D12GraphicsCommandList10* GetDefaultCommandList() const;
		ID3D12CommandAllocator* GetDefaultCommandAllocator() const;

		ID3D12Device14* GetDevice() const { return m_Device.Get(); }
		auto GetDxcUtils() const { return m_Utils.Get(); }

		//Raytracing helpers
		static std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> ConvertSkateboardBlasDescToD3DGeometries(const BottomLevelAccelerationStructureDesc& Descriptions);

		ID3D12CommandQueue* CommandQueue() const { return m_GraphicsCommandQueue.Get(); }
		ID3D12CommandQueue* CopyQueue() const { return m_CopyCommandQueue.Get(); }

		D3DDescriptorHeap& GetCPUSRVDescriptorHeap() { return m_RWSRVDescriptorHeap; }
		D3DDescriptorHeap& GetGPUSRVDescriptorHeap() { return m_SRVDescriptorHeap; }
		D3DDescriptorHeap& GetRTVDescriptorHeap() { return m_RTVDescriptorHeap; }
		D3DDescriptorHeap& GetDSVDescriptorHeap() { return m_DSVDescriptorHeap; }
		D3DDescriptorHeap& GetSamplerDescriptorHeap() { return m_SamplerHeap; }

		_NODISCARD D3D12_RECT GetScissorsRect() { return m_ScissorRect; }
		_NODISCARD D3D12_RECT GetScissorsRect() const { return m_ScissorRect; }

		_NODISCARD D3D12_VIEWPORT GetViewport() { return m_Viewport; }
		_NODISCARD D3D12_VIEWPORT GetViewport() const { return m_Viewport; }

		_NODISCARD ID3D12Resource* GetCurrentD3DBackBuffer() { return m_SwapChainBuffers[m_CurrentBackBuffer].Get(); }
		_NODISCARD const ID3D12Resource* GetCurrentD3DBackBuffer() const { return m_SwapChainBuffers[m_CurrentBackBuffer].Get(); }

		uint64_t NextFence() { ++m_LatestFenceValue; GetFenceValue() = m_LatestFenceValue; return m_LatestFenceValue; }
		_NODISCARD ID3D12Fence* GetFence() { return m_Fence.Get(); }
		_NODISCARD const ID3D12Fence* GetFence() const { return m_Fence.Get(); }
		_NODISCARD uint64_t& GetFenceValue() { return a_FenceValues[m_CurrentFrameResourceIndex]; }
		
		// We need to be able to access the descriptors stored in the respective heaps of our buffer
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilViewHandle() const;

		void SetDeferredReleasesFlag();

		void DeferredRelease(IUnknown* resource);

		void ProcessDeferrals();

		bool CheckDeviceRemovedStatus() const;

		D3D12MA::Allocator* GetMemoryAllocator() { return p_MemoryAllocator; }

	private:
		void CreateDevice();
		void CreateFence();
		void CreateDescriptorSizes();
		void Check4xMSAAQualitySupport();
		void CreateCommandQueueAndCommandList();
		void CreateSwapChain();

		void CreateRenderTargetViews();
		void CreateDepthStencilBuffer();
		void SetViewPort();
		void SetScissorRectangles();

		void CreateDescriptorHeaps();

		void CreateUploadManager();

	private:
		// Derived class should set these in derived constructor to customize starting values.
		D3D_DRIVER_TYPE	m_D3DDriverType;					// Driver type options
		DXGI_FORMAT		m_BackBufferFormat;					// Format of the back buffer
		DXGI_FORMAT		m_DepthStencilFormat;				// Format of the depth/stencil buffer

		// Window settings, mostly in regards with resizing
		bool	m_Vsync;								// Note: Fullscreen in this framework is handled by the platform API

		// MSAA support (not used in this project, but could be enabled)
		bool m_MSAAEnable;
		UINT m_MSAAQuality;

		// Working environment
		HWND m_MainWindow;

		//DXC GetDxcUtils for shader reflection logic
		Microsoft::WRL::ComPtr<IDxcUtils> m_Utils;

		//upload manager for uploading buffers
		UploadManager m_UploadManager;

		// COM Objects
		// GetDevice and DXGI Factory
		Microsoft::WRL::ComPtr<ID3D12Device14>				m_Device;						// The device is the display adapter, like the graphics card

		DWORD												m_MessageCallbackCookie;
		ComPtr<ID3D12InfoQueue1>							m_InfoQueue;

		Microsoft::WRL::ComPtr<IDXGIFactory7>				m_DXGIInterface;				// The interface to generate any DXGI objects (version 4 provides more functionalities, such as EnumWarpAdapters)

		//Memory Allocator // INITIALISED IN THE CREATE DEVICE
		D3D12MA::Allocator*									p_MemoryAllocator;

		// Fence
		Microsoft::WRL::ComPtr<ID3D12Fence>					m_Fence;						// The fence object to synchronise the CPU/GPU
		uint64_t											m_LatestFenceValue;
		std::array<uint64_t, GRAPHICS_SETTINGS_NUMFRAMERESOURCES> a_FenceValues;			// Identify a fence point in time. Everytime we mark a new fence point, increment this integer

		// Command Objects
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>			m_GraphicsCommandQueue;					// The command queue we will use for this application to submit commands to the GPU
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>			m_ComputeCommandQueue;					// The command queue we will use for this application to submit commands to the GPU
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>			m_CopyCommandQueue;					// The command queue we will use for this application to submit commands to the GPU

		//Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		m_DirectCommandAllocator;
//		std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, GRAPHICS_SETTINGS_NUMFRAMERESOURCES> a_CommandAllocators;
//		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList10>	m_CommandList;

		// array of Command Allocators for device queues
//		std::array<std::stack<ComPtr<ID3D12CommandAllocator>>, 7 > m_AvailableCommandAllocators;
//		std::array<std::stack<ComPtr<ID3D12CommandAllocator>>, 7 > m_InUseCommandAllocators;

		// SwapChain
		Microsoft::WRL::ComPtr<IDXGISwapChain4>				m_SwapChain;

		// Buffers
		static const int									g_SwapChainBufferCount = GRAPHICS_SETTINGS_NUMFRAMERESOURCES;
		int													m_CurrentBackBuffer;

		Microsoft::WRL::ComPtr<ID3D12Resource>				m_SwapChainBuffers[g_SwapChainBufferCount];	// The back buffers to use in the swapchain

		// Descriptor sizes
		UINT m_RTVDescriptorSize;
		UINT m_DSVDescriptorSize;
		UINT m_CBVSRVUAVDescriptorSize;

		// Viewports and scissor rectangles
		D3D12_VIEWPORT	m_Viewport;							// The viewport to which the 3D world will be rendered onto
		D3D12_RECT		m_ScissorRect;						// Pixels outside of this rectangle are culled (not rasterized onto the back buffer)

		// Bools
		bool	m_HasDXR;			// A bool that will be checked on init to initialise raytracing components
		bool	m_HasWorkGraphs;
		bool	m_ClientResized;

		std::vector<IUnknown*> m_DeferredReleases[GRAPHICS_SETTINGS_NUMFRAMERESOURCES]{};
		UINT32 m_DeferredFlags[GRAPHICS_SETTINGS_NUMFRAMERESOURCES];

		std::mutex m_DeferredMutex;

		D3DDescriptorHeap m_RWSRVDescriptorHeap;		// Read-Write
		D3DDescriptorHeap m_SRVDescriptorHeap;			// Write only as shader visible

		D3DDescriptorHeap m_RTVDescriptorHeap;
		D3DDescriptorHeap m_DSVDescriptorHeap;

		D3DDescriptorHeap m_SamplerHeap;

		//Imgui
		D3DDescriptorHandle m_ImGuiHandle;

		//RenderAPI
		D3DRenderCommand D3D_API;
		D3DResourceFactory D3D_RESOURCE_FACTORY;

		
};
}
