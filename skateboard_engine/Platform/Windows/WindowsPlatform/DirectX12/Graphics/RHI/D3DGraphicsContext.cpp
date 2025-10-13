#include "sktbdpch.h"
#include "D3DGraphicsContext.h"
#include "Skateboard/Platform.h"

#include "Graphics/Resources/D3DBuffer.h"
#include "Graphics/Resources/D3DView.h"
#include "Graphics/Resources/D3DPipeline.h"
#include "Graphics/Resources/D3DCommandBuffer.h"

#include "D3DDebugTools.h"

#define SKTBD_LOG_COMPONENT "D3DGraphicsContext"
#include "Skateboard/Log.h"

#define UPLOAD_MANAGER_SIZE 2*1024*1024

//Agility SDK definitions
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 616; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

namespace Skateboard
{
	//Skateboard Graphic Constants
	namespace GraphicsConstants
	{
		size_t DEFAULT_RESOURCE_ALIGNMENT = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		size_t SMALL_RESOURCE_ALIGNMENT = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
		size_t MSAA_RESOURCE_ALIGNMENT = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
		size_t SMALL_MSAA_RESOURCE_ALIGNMENT = D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
		size_t CONSTANT_BUFFER_ALIGNMENT = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
		size_t BUFFER_ALIGNMENT = 4;
		size_t RAYTRACING_STRUCT_ALIGNMENT = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT;
		size_t RAYTRACING_TLAS_INSTANCE_DESC_ALIGNMENT = D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT;

		size_t RAYTRACING_SHADER_TABLE_ALIGNMENT = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
		size_t RAYTRACING_SHADER_TABLE_SHADER_ID_ALIGNMENT = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
		size_t RAYTRACING_SHADER_TABLE_RECORD_ALIGNMENT = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	}

	D3DGraphicsContext* gD3DContext = nullptr;
	GraphicsContext* GraphicsContext::Context = nullptr;

	D3DGraphicsContext::D3DGraphicsContext(HWND window, const PlatformProperties& props) :
		GraphicsContext(props.BackBufferWidth, props.BackBufferHeight),
		m_D3DDriverType(D3D_DRIVER_TYPE_HARDWARE),
		m_BackBufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM),	// Each element has four 8-bit unsigned components mapped to the [0,1] range
		m_DepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT),	// Specifies an unsigned 24-bit depth buffer mapped to the [0,1] range, with an 8-bit unsigned integer reserved for the stencil buffer ([0, 255] range) 
		m_Vsync(false),
		m_MSAAEnable(false),									// Set to false by default, change it to true in the constructor to enable MSAA
		m_MSAAQuality(0),										// Needs to be queried in Check4xMSAAQualitySupport
		m_MainWindow(window),
		m_LatestFenceValue(0u),
		a_FenceValues{0u},
		m_CurrentBackBuffer(0),									// Start by working on the first buffer
		m_RTVDescriptorSize(0u),
		m_DSVDescriptorSize(0u),
		m_CBVSRVUAVDescriptorSize(0u),
		m_HasDXR(false),										// This will be checked on initialisation, if the GPU supports DXR
		m_ClientResized(true)
	{
		// Assign the global context
		SKTBD_LOG_TRACE("D3DGraphicsContext","Initialising the Graphics Context..");
		SKTBD_LOG_ASSERT(!gD3DContext, "Cannot initialise a second D3D graphics context. Illegal context creation.");
		gD3DContext = this;
		Context = this;

		// Initialise D3D components
		CreateDevice();
		CreateFence();
		CreateDescriptorSizes();
		Check4xMSAAQualitySupport();
		CreateCommandQueueAndCommandList();
		CreateSwapChain();
		
		// Allocate basic descriptor heaps
		CreateDescriptorHeaps();

		// Create the output buffers and off load anything that could have potentially been loded from staging to buffers
		Update();

		//create upload manager
		CreateUploadManager();
	}

	D3DGraphicsContext::~D3DGraphicsContext()
	{
		WaitUntilIdle();

		//stop recieving messages from validation layers if there is an infoqueue
		if(m_InfoQueue.Get())
		m_InfoQueue->UnregisterMessageCallback(m_MessageCallbackCookie);

		SKTBD_MSG_TRACE("Destroying GetDevice..");
	}

	void D3DGraphicsContext::SetRenderTargetToBackBuffer_()
	{
		GetDefaultCommandList()->RSSetViewports(1, &m_Viewport);
		GetDefaultCommandList()->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferHandle = CurrentBackBufferView();
		D3D12_CPU_DESCRIPTOR_HANDLE currentDepthStencilHandle = DepthStencilViewHandle();
		GetDefaultCommandList()->OMSetRenderTargets(1, &currentBackBufferHandle, true, &currentDepthStencilHandle);
	}

	void D3DGraphicsContext::BeginFrame_()
	{
		GetMemoryAllocator()->SetCurrentFrameIndex(m_LatestFenceValue);

		// Get required API objects
		ID3D12GraphicsCommandList10* commandList = GetDefaultCommandList();
		const uint64_t currentFence = GetFenceValue();
		ID3D12CommandAllocator* currentAllocator = GetDefaultCommandAllocator();
		const D3D12_VIEWPORT viewport = GetViewport();
		const D3D12_RECT scissorsRect = GetScissorsRect();
		ID3D12Resource* backBuffer = GetCurrentD3DBackBuffer();
		ID3D12Resource* DepthBuffer = static_cast<D3DTextureBuffer*>(m_DepthStencilBuffer.get())->GetResource();

		const D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferHandle = CurrentBackBufferView();
		const D3D12_CPU_DESCRIPTOR_HANDLE currentDepthStencilHandle = DepthStencilViewHandle();
		ID3D12DescriptorHeap* const pSrvHeap = GetSRVHeap();

		ID3D12DescriptorHeap* const pSamplerHeap = GetSamplerHeap();

		SKTBD_LOG_ASSERT(commandList, "Null command list!");

		// Check if the GPU is done with the next frame resource before proceeding
		// If not wait to avoid buffer overwrites
		if (currentFence && GetFence()->GetCompletedValue() < currentFence)
		{
#ifdef SKTBD_LOG_CPU_IDLE
			SKTBD_MSG_WARN("CPU idle, waiting on GPU to finish tasks!");
#endif
			HANDLE eventHandle = CreateEventEx(nullptr, NULL, NULL, EVENT_ALL_ACCESS);
			SKTBD_LOG_ASSERT(eventHandle, "Fence event creation failed!");

			// Fire the previously created event when GPU hits current fence
			D3D_CHECK_FAILURE(GetFence()->SetEventOnCompletion(currentFence, eventHandle));

			// Wait until the GPU hits the current fence and the event is received
			WaitForSingleObject(eventHandle, INFINITE);

			// Close the event so that it does not exist anymore
			CloseHandle(eventHandle);
		}

		// Process deferred resources here?
		ProcessDeferrals();
		// 

		// Reset the command list allocator to reuse its memory
		// Note: this can only be reset once the GPU has finished processing all the commands
		D3D_CHECK_FAILURE(currentAllocator->Reset());

		// Also reset the command list
		// Note: It can be reset anytime after calling ExecuteCommandList on the command queue
		// Note: The second argument will be used when drawing geometry
		D3D_CHECK_FAILURE(commandList->Reset(currentAllocator, nullptr));

		D3D12_TEXTURE_BARRIER TexBarriers[] =
		{
			CD3DX12_TEXTURE_BARRIER(
				D3D12_BARRIER_SYNC_ALL,									// SyncBefore
				D3D12_BARRIER_SYNC_RENDER_TARGET,						// SyncAfter
				D3D12_BARRIER_ACCESS_COMMON,							// AccessBefore
				D3D12_BARRIER_ACCESS_RENDER_TARGET,                     // AccessAfter
				D3D12_BARRIER_LAYOUT_PRESENT,							// LayoutBefore
				D3D12_BARRIER_LAYOUT_RENDER_TARGET,                     // LayoutAfter
				backBuffer,
				CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),			// All subresources
				D3D12_TEXTURE_BARRIER_FLAG_NONE
			)
		};

		D3D12_BARRIER_GROUP TexBarrierGroup[] =
		{
			CD3DX12_BARRIER_GROUP(1,TexBarriers)
		};

		commandList->Barrier(1, TexBarrierGroup);

		// Set the viewport and scissor rects. They need to be reset everytime the command list is reset
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorsRect);

		if (m_bClearBackBuffer)
		{
			// Clear the back buffer and depth buffer
			commandList->ClearRenderTargetView(
				currentBackBufferHandle,								// RTV to the resource we want to clear
				&m_ClearColour.x,											// The colour to clear the render target to
				0,														// The number of items in the pRects array (next parameter)
				nullptr													// An array of D3D12_RECTs that identify rectangle regions on the render target to clear. When nullptr, the entire render target is cleared
			);
		}

		commandList->ClearDepthStencilView(
			currentDepthStencilHandle,								// DSV to the resource we want to clear
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,		// Flags indicating which part of the depth/stencil buffer to clear (here both)
			GRAPHICS_DEPTH_DEFAULT_CLEAR_COLOUR,					// Defines the value to clear the depth buffer
			GRAPHICS_STENCIL_DEFAULT_CLEAR_COLOUR,					// Defines the value to clear the stencil buffer
			0,														// The number of items in the pRects array (next parameter)
			nullptr													// An array of D3D12_RECTs that identify rectangle regions on the render target to clear. When nullptr, the entire render target is cleared
		);

		D3D12_TEXTURE_BARRIER DepthBarrieraAC[] =
		{
			CD3DX12_TEXTURE_BARRIER(
				D3D12_BARRIER_SYNC_DEPTH_STENCIL,									// SyncBefore
				D3D12_BARRIER_SYNC_ALL,						// SyncAfter
				D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE,							// AccessBefore
				D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE,                     // AccessAfter
				D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE,							// LayoutBefore
				D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE,                     // LayoutAfter
				DepthBuffer,
				CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),			// All subresources
				D3D12_TEXTURE_BARRIER_FLAG_NONE
			),

			CD3DX12_TEXTURE_BARRIER(
				D3D12_BARRIER_SYNC_RENDER_TARGET,								// SyncBefore
				D3D12_BARRIER_SYNC_ALL,											// SyncAfter
				D3D12_BARRIER_ACCESS_RENDER_TARGET,						// AccessBefore
				D3D12_BARRIER_ACCESS_RENDER_TARGET,						// AccessAfter
				D3D12_BARRIER_LAYOUT_RENDER_TARGET,							// LayoutBefore
				D3D12_BARRIER_LAYOUT_RENDER_TARGET,                     // LayoutAfter
				backBuffer,
				CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),			// All subresources
				D3D12_TEXTURE_BARRIER_FLAG_NONE
			)
		};

		D3D12_BARRIER_GROUP DepthBarrierGroupAC[] =
		{
			CD3DX12_BARRIER_GROUP(2,DepthBarrieraAC)
		};

		commandList->Barrier(1, DepthBarrierGroupAC);

		// Set the render target and depth/stencil target to the output merger
		commandList->OMSetRenderTargets(
			1,														// Defines the number of RTVs we are going to bind (next param)
			&currentBackBufferHandle,								// Pointer to an array of RTVs we want to bind to the pipeline
			true,													// Specify true if all the RTVs in the previous array are contiguous in the descriptor heap
			&currentDepthStencilHandle								// Pointer to a DSV we want to bind to the pipeline
		);

		// Set the main GPU descriptor heap, that is a region of memory where we have stored the descriptors for all the resources we want to use in this frame

		ID3D12DescriptorHeap* heaps[] = { pSrvHeap, pSamplerHeap };

		commandList->SetDescriptorHeaps(2, heaps);
	}

	void D3DGraphicsContext::EndFrame_()
	{
		// Get required API objects
		ID3D12GraphicsCommandList10* commandList = GetDefaultCommandList();
		ID3D12CommandQueue* commandQueue = CommandQueue();
		ID3D12Resource* backBuffer = GetCurrentD3DBackBuffer();
		ID3D12Fence* fence = GetFence();

		// Transition the back buffer from a PRESENT state to a RT state so that it can be written onto
		//D3D12_RESOURCE_BARRIER barrier = D3D::TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		D3D12_TEXTURE_BARRIER TexBarriers[] =
		{
			CD3DX12_TEXTURE_BARRIER(
				D3D12_BARRIER_SYNC_RENDER_TARGET,					 // SyncBefore
				D3D12_BARRIER_SYNC_ALL,								 // SyncAfter
				D3D12_BARRIER_ACCESS_RENDER_TARGET,					 // AccessBefore
				D3D12_BARRIER_ACCESS_COMMON,                         // AccessAfter
				D3D12_BARRIER_LAYOUT_RENDER_TARGET,                  // LayoutBefore
				D3D12_BARRIER_LAYOUT_PRESENT,                        // LayoutAfter
				backBuffer,
				CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),       // All subresources
				D3D12_TEXTURE_BARRIER_FLAG_NONE
			)
		};

		D3D12_BARRIER_GROUP TexBarrierGroup[] =
		{
			CD3DX12_BARRIER_GROUP(1,TexBarriers)
		};

		commandList->Barrier(1, TexBarrierGroup);

		// Close the command list for recording.
		D3D_CHECK_FAILURE(commandList->Close());

		//Add the command list to the command queue for execution
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		// Rendering processed, mark the frame fence
		commandQueue->Signal(fence, NextFence());

		// Swap the back and front buffers
		// Arg1 - An integer that specifies how to synchronize presentation of a frame with the vertical blank:
		//	0 - Cancel the remaining time on the previously presented frame and discard this frame if a newer frame is queued
		// Arg2 - An integer value that contains swap-chain presentation options. These options are defined by the DXGI_PRESENT constants
		//D3D_CHECK_FAILURE(m_SwapChain->Present(m_Vsync, 0));

		if (FAILED(m_SwapChain->Present(m_Vsync, 0)))
		{
			HRESULT hr = GetDevice()->GetDeviceRemovedReason();
			D3D_CHECK_FAILURE(hr);
		}

		// Update the index to the current back buffer so that we render to the other buffer next frame
		NextBackBuffer();
	}

	void D3DGraphicsContext::CreateDevice()
	{
		SKTBD_MSG_TRACE("Creating d3d12 device..");

		// First, enable the debug layer of D3D12 when running the application in debug mode
		// This will output debug messages when warnings, errors or crashes occur during rutime in the output window
		UINT DXGIFlags = 0;

#ifndef SKTBD_SHIP
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		DXGIFlags |= DXGI_CREATE_FACTORY_DEBUG;
		HRESULT res = D3D12GetDebugInterface(IID_PPV_ARGS(debugController.ReleaseAndGetAddressOf()));

		if (SUCCEEDED(res))
		{
			SKTBD_MSG_INFO("Debug Layers Enabled");
			debugController->EnableDebugLayer();
		}
		else
		{
			SKTBD_MSG_INFO("Debug Layers Disabled");
		}

		// Enable DRED
		ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> dredSettings;
		if (m_Flags & D3D_DEVICE_REMOVED_EXTENDED_DATA_ENABLE_FLAG && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings))))
		{
			// Turn on AutoBreadcrumbs and Page Fault reporting
			dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);

			SKTBD_MSG_INFO("DRED Enabled");
		}
#endif

		// Create the DXGI interface 1.1
		D3D_CHECK_FAILURE(CreateDXGIFactory2(DXGIFlags,IID_PPV_ARGS(m_DXGIInterface.ReleaseAndGetAddressOf())));

		// Create DXC GetDxcUtils 

		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(m_Utils.GetAddressOf()));

		// Create a list of every GFX adapter available
		IDXGIAdapter4* pAdapter = nullptr;
		std::vector<IDXGIAdapter4*> vAdapters;
		UINT adapterIndex = 0;
		for (UINT i = 0u; m_DXGIInterface->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)) != DXGI_ERROR_NOT_FOUND; ++i)
			vAdapters.push_back(pAdapter);

		// Find the best GFX adapter on this machine
		SIZE_T maximumVideoMemory = 0;
		DXGI_ADAPTER_DESC adapterDescription;
		for (auto& currentAdapter : vAdapters)
		{
			// Get the description of this adapter
			// This will help identify the adapter's properties
			currentAdapter->GetDesc(&adapterDescription);

			// Select the best adapter based on the maximum video memory
			if (adapterDescription.DedicatedVideoMemory > maximumVideoMemory)
			{
				maximumVideoMemory = adapterDescription.DedicatedVideoMemory;
				pAdapter = currentAdapter;
			}
		}


		// Try to create a device on the selected adapter
		// We want to limit this application to adapter capable of handling DX12
		if (pAdapter)
		{
			if (FAILED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()))))
			{
				SKTBD_MSG_INFO("D3D12 Feature Level 12_2 not supported dropping to 12_1");
				SKTBD_MSG_INFO("D3D12 Feature Level 12_2 not supported! Excercise caution when using Skateboard interfaces to create D3D12 Objects, Refer to Microsoft for Gpu Feature Support");


				if (FAILED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()))))
				{
					SKTBD_MSG_INFO("D3D12 Feature Level 12_1 not supported dropping to 12_0");


					if (FAILED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()))))
					{
						SKTBD_MSG_INFO("D3D12 Feature Level 12_0 not supported dropping to 11_0");

						if (FAILED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()))))
						{
							SKTBD_MSG_ERROR("Adapter doesnt support DX12, What are you using this on ????");
						}
					}
				}
			}
		}
		
		//check support for enhanced barriers, otherwise will have to make do with a Warp device
		D3D12_FEATURE_DATA_D3D12_OPTIONS12 options{};
		if(m_Device)
		m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options, sizeof(options));

		if(!pAdapter || !options.EnhancedBarriersSupported)
		{
			// Fallback to a WARP device
			SKTBD_MSG_WARN(L"No GPU detected or Gpu lacks support for enhaned barriers, fallback to a warp device... rendering will be fully cpu, but most d3d features will be supported\n");

			if (pAdapter) pAdapter->Release();
			// Get a suitable WARP adapter and create a device with it
			D3D_CHECK_FAILURE(m_DXGIInterface->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter)));

			// Create a device on the warp adapter. If failed, throw an exception as no adapter was selected
			D3D_CHECK_FAILURE(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf())));

			//// Get description and release memory
			//pAdapter->GetDesc(&adapterDescription);
			//pAdapter->Release(), pAdapter = nullptr;
		}

#ifndef SKTBD_SHIP
		// Output the selected device on the console
		std::wstring selectedAdapterInfo;
		if (pAdapter) pAdapter->GetDesc(&adapterDescription);
		selectedAdapterInfo += L"Selected device:\n\t";
		selectedAdapterInfo += adapterDescription.Description;
		selectedAdapterInfo += L"\n\tAvailable Dedicated Video Memory: ";
		selectedAdapterInfo += std::to_wstring(adapterDescription.DedicatedVideoMemory / 1000000000.f);
		selectedAdapterInfo += L" GB";
		SKTBD_MSG_INFO(selectedAdapterInfo.c_str());

		SKTBD_MSG_INFO("Registering GetDevice for Debug Layer Messages");

		// Set up the info queue for the device
		// Debug layer must be enabled for this: so only perform this in debug
		// Note that means that m_InfoQueue should always be checked for existence before use
		if (!FAILED(m_Device->QueryInterface(IID_PPV_ARGS(&m_InfoQueue))))
		{
			// Set up message callback
			D3D_CHECK_FAILURE(m_InfoQueue->RegisterMessageCallback(D3DDebugTools::D3DMessageHandler, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_MessageCallbackCookie));
			D3D_CHECK_FAILURE(m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true));
			SKTBD_MSG_INFO("D3D Info Queue message callback created.");
		}
		else
		{
			SKTBD_MSG_WARN("D3D Info Queue interface not available! D3D device messages will not be received.");
		}

#endif

		//Initialise Memory allocator
		{
			D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
			allocatorDesc.pDevice = m_Device.Get();
			allocatorDesc.pAdapter = pAdapter;
			// These flags are optional but recommended.
			allocatorDesc.Flags = (D3D12MA::ALLOCATOR_FLAGS) (D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED | D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED);

			HRESULT hr = D3D12MA::CreateAllocator(&allocatorDesc, &p_MemoryAllocator);
			D3D_CHECK_FAILURE(hr)
		}


		// Release unused adapters to clean memory
		for (auto& currentAdapter : vAdapters) currentAdapter->Release(), currentAdapter = nullptr;

		// Verify compatibility with raytracing
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};
		if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData))) || featureSupportData.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		{
			SKTBD_MSG_WARN(L"DirectX Raytracing unsupported on this hardware. Raytracing functionalities disabled.");
			return;
		}

		// If compatible, enable raytracing support on this application
		m_HasDXR = true;
	}

	void D3DGraphicsContext::CreateFence()
	{
		SKTBD_MSG_TRACE("Creating fence..");

		// Create a fence object for CPU/GPU synchornisation
		D3D_CHECK_FAILURE(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf())));
	}

	void D3DGraphicsContext::CreateDescriptorSizes()
	{
		// When we will work with descriptors, we are going to need their sizes
		// Descriptor sizes can vary across GPUs, so we need to query this information
		// We cache the descriptors sizes on this class so that it is available when we need them for various descriptor types
		// It could be queried every time we need them instead

		m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DSVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CBVSRVUAVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	void D3DGraphicsContext::Check4xMSAAQualitySupport()
	{
		// Check for 4x MultiSample Anti-Aliasing support
		// 4x is chosen because it guaranteed to be supported by DX12 hardware
		// and because it provides good improvement without being too expensive
		// However, we do have to check for the supported quality level

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleQualityLevels = {};
		multisampleQualityLevels.Format = m_BackBufferFormat;							//
		multisampleQualityLevels.SampleCount = 4;										// 4 samples for a 4x MSAA
		multisampleQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;	//
		multisampleQualityLevels.NumQualityLevels = 0;									// Initially 0, will be determined by the next call

		D3D_CHECK_FAILURE(m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &multisampleQualityLevels, sizeof(multisampleQualityLevels)));

		// Since 4x should natively be supported, m_MSAAQuality should always be greater than 0
		m_MSAAQuality = multisampleQualityLevels.NumQualityLevels;
		SKTBD_LOG_ASSERT(m_MSAAQuality > 0, "Unexpected MSAA quality level.");
	}

	void D3DGraphicsContext::CreateCommandQueueAndCommandList()
	{
		SKTBD_MSG_TRACE("Creating command queue and command list..");

		// Start with describing the command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	// Specifies a command buffer that the GPU can execute. A direct command list doesn't inherit any GPU state.
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	// No particular flags

		// Create the command queue
		D3D_CHECK_FAILURE(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_GraphicsCommandQueue.ReleaseAndGetAddressOf())));

		//Create Compute Queue
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		D3D_CHECK_FAILURE(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_ComputeCommandQueue.ReleaseAndGetAddressOf())));

		//Create Copy Queue
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		D3D_CHECK_FAILURE(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CopyCommandQueue.ReleaseAndGetAddressOf())));

		m_DefaultGraphicsCB.ForEach([&](GraphicsCommandBufferRef& ref){ ref = ResourceFactory::CreateGraphicsCommandBuffer(CommandBufferPriority_Primary); });
	}

	void D3DGraphicsContext::CreateSwapChain()
	{
		SKTBD_MSG_TRACE("Creating swap chain..");

		// First reset any previous swap chain that was created, in the event that a new swap chain is being created during runtime (i.e. if the user changes some settings!)
		m_SwapChain.Reset();

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = GRAPHICS_SETTINGS_NUMFRAMERESOURCES;
		swapChainDesc.Width = m_ClientWidth;
		swapChainDesc.Height = m_ClientHeight;
		swapChainDesc.Format = m_BackBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ComPtr<IDXGISwapChain1> swapChain;
		D3D_CHECK_FAILURE(m_DXGIInterface->CreateSwapChainForHwnd(
			m_GraphicsCommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
			m_MainWindow,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		D3D_CHECK_FAILURE(swapChain.As(&m_SwapChain));
	}

	void D3DGraphicsContext::Resize_(int clientWidth, int clientHeight)
	{
		// Assign new dimensions if existing (we don't want a 0 dimension, would crash)
		SKTBD_LOG_ASSERT(clientWidth || clientHeight, "Impossible client surface dimensions");

		m_ClientWidth = clientWidth;
		m_ClientHeight = clientHeight;
		m_ClientResized = true;
	}

	void D3DGraphicsContext::Update_()
	{
		//Resizing Work
		{
			// Do not perform unnecessary resizing
			if (m_ClientResized)
			{
				SKTBD_MSG_INFO("Resizing back buffers, two GPU flushes expected..");

				// This function will be called everytime the application is being resized
				// It needs to re-create RTVs, a DSV, a viewport and scissor rectangles
				// First verify that we have all the required objects
				SKTBD_LOG_ASSERT(m_Device, "GetDevice does not exists when trying to resize buffers!");
				SKTBD_LOG_ASSERT(m_SwapChain, "Swapchain does not exists when trying to resize buffers!");
				SKTBD_LOG_ASSERT(GetDefaultCommandAllocator(), "Command allocator does not exists when trying to resize buffers!");

				// Flush before changing any resource
				WaitUntilIdle();

				// Reset the command list to a NULL state
				D3D_CHECK_FAILURE(GetDefaultCommandList()->Reset(GetDefaultCommandAllocator(), nullptr));

				// Release the previous resources we will be recreating.
				for (int i = 0; i < g_SwapChainBufferCount; ++i) m_SwapChainBuffers[i].Reset();

				if (m_DepthStencilBuffer.get())
				{
					m_DepthStencilBuffer.reset();
					m_DefaultDSV.reset();
				}

				// Resize the swap chain.
				D3D_CHECK_FAILURE(m_SwapChain->ResizeBuffers(g_SwapChainBufferCount, m_ClientWidth, m_ClientHeight, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

				// Reset the current back buffer to the first so we can properly re-create the render targets
				m_CurrentBackBuffer = 0;

				// Create new render target views
				CreateRenderTargetViews();

				// Create the detph/stencil buffer view
				CreateDepthStencilBuffer();

				D3D12_TEXTURE_BARRIER text_b = {};

				text_b.pResource = static_cast<D3DTextureBuffer*>(m_DepthStencilBuffer.get())->m_TextureResource->GetResource();
				text_b.LayoutBefore = D3D12_BARRIER_LAYOUT_UNDEFINED;
				text_b.LayoutAfter = D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;

				text_b.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS;
				text_b.AccessAfter = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;

				text_b.SyncBefore = D3D12_BARRIER_SYNC_NONE;
				text_b.SyncAfter = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
				text_b.Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff);
				text_b.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;

				//CD3DX12_TEXTURE_BARRIER()

				D3D12_BARRIER_GROUP b_group[1];
				b_group[0].NumBarriers = 1;
				b_group[0].Type = D3D12_BARRIER_TYPE_TEXTURE;
				b_group[0].pTextureBarriers = &text_b;

				// Execute the resize commands
				Flush();

				// Update the viewport transform to cover the client area
				SetViewPort();

				// Update the scissor rectangles
				SetScissorRectangles();

				// Reset resized tracker
				m_ClientResized = false;
			}
		}

		// upload manager;
		{
			m_UploadManager.Update();
		}

		// 
		{
			ProcessDeferrals();
		}
	}

	void D3DGraphicsContext::CreateDescriptorHeaps()
	{
		m_SRVDescriptorHeap.Create(L"Main SRV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096, true);
		m_SamplerHeap.Create(L"Main Sampler Heap", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024, true);

		m_RTVDescriptorHeap.Create(L"Main RTV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 4096, false);
		m_DSVDescriptorHeap.Create(L"Main DSV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 4096, false);


		// Allocate the backbuffers and depth/stencil buffer
		m_ImGuiHandle = m_SRVDescriptorHeap.Allocate(1);
	}

	void D3DGraphicsContext::CreateUploadManager()
	{
		m_UploadManager.Init(p_MemoryAllocator, UPLOAD_MANAGER_SIZE);
	}

	ShaderIdentifier D3DGraphicsContext::GetShaderIdentifier(const Pipeline* pipeline, const std::wstring& ShaderName)
	{
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> rtpsoProperties;
		
		D3D_CHECK_FAILURE(reinterpret_cast<const ID3DPipelineInterface*>(pipeline)->GetState()->QueryInterface(IID_PPV_ARGS(rtpsoProperties.ReleaseAndGetAddressOf())));

		return ShaderIdentifier(rtpsoProperties->GetShaderIdentifier(ShaderName.c_str()));
	}

	void D3DGraphicsContext::CreateRenderTargetViews()
	{
		// Get the descriptor handle of the first buffer in the heap
		//D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
		//D3DDescriptorHandle rtvHeapHandle = m_RTVDescriptorHandle;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = SkateboardBufferFormatToD3D(DataFormat_DEFAULT_BACKBUFFER);
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		for (int i = 0; i < GRAPHICS_SETTINGS_NUMFRAMERESOURCES; i++) {

			D3D_CHECK_FAILURE(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_SwapChainBuffers[i].ReleaseAndGetAddressOf())));

			auto handle = m_RTVDescriptorHeap.Allocate();
			m_Device->CreateRenderTargetView(m_SwapChainBuffers[i].Get(), &rtvDesc, handle.GetCPUHandle());
		
			m_SwapChainRTVs[i] = std::make_shared<D3DRenderTargetView>(handle);
		};
	}

	void D3DGraphicsContext::CreateDepthStencilBuffer()
	{
		// The depth/stencil buffer is a 2D texture, which differs from the render target views
		// Therefore, we must first initialise a D3D12_RESOURCE_DESC with the given charasteristics for this resource

		TextureDesc Desc;
		Desc.Dimension = TextureDimension_Texture2D;				// Specify the dimensions of this resource
		Desc.Width = m_ClientWidth;									// The width of the texture in texels; for buffers, this is the number of bytes in the buffer
		Desc.Height = m_ClientHeight;								// The height of the texture in texels
		Desc.Depth = 1;												// The depth of the texture in texels, or the texture array size (for 1D and 2D textures)
		Desc.Mips = 1;												// 
		Desc.Format = DataFormat_DEFAULT_DEPTHSTENCIL;				// 
		Desc.Type = TextureType_DepthStencil;
		Desc.AccessFlags = ResourceAccessFlag_GpuRead | ResourceAccessFlag_GpuWrite;
		Desc.Clear.Depth = GRAPHICS_DEPTH_DEFAULT_CLEAR_COLOUR;
		Desc.Clear.Stencil = GRAPHICS_STENCIL_DEFAULT_CLEAR_COLOUR;


		m_DepthStencilBuffer = ResourceFactory::CreateTextureBuffer(Desc);

		// Create the Depth/Stencil view
		// If the resource was created with a typed format (i.e. not typeless), then the D3D12_DEPTH_STENCIL_VIEW_DESC parameter can be null
		// It indicates to create a view to the first mipmap level of this resource with the format the resource was created with

		/*DepthStencilDesc ViewDesc{};
		ViewDesc.ArraySize = 1;
		ViewDesc.Dimension = TextureDimension_Texture2D;
		ViewDesc.FirstArraySlice = 0;
		ViewDesc.Format = DataFormat_DEFAULT_DEPTHSTENCIL;
		ViewDesc.MipLevels = 1;*/

		m_DefaultDSV = ResourceFactory::CreateDepthStencilView(nullptr, m_DepthStencilBuffer);
	}

	void D3DGraphicsContext::SetViewPort()
	{
		// Describe the viewport charasteristics to fit the entire window with a normal depth buffer range
		m_Viewport.TopLeftX = 0.f;
		m_Viewport.TopLeftY = 0.f;
		m_Viewport.Width = static_cast<float>(m_ClientWidth);
		m_Viewport.Height = static_cast<float>(m_ClientHeight);
		m_Viewport.MinDepth = 0.f;
		m_Viewport.MaxDepth = 1.f;
	}

	void D3DGraphicsContext::SetScissorRectangles()
	{
		// For now we don't have any HUDs, so assign the area to fit the client surface
		m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };
	}

	void D3DGraphicsContext::Reset_()
	{
		SKTBD_MSG_WARN("Reset called and this might be bad!");
		D3D_CHECK_FAILURE(GetDefaultCommandList()->Reset(GetDefaultCommandAllocator(), nullptr));
	}

	void D3DGraphicsContext::Flush_()
	{
		//only valid between begin frame and end frame.... is that what i want

		// Execute the new commands and wait until completion
		D3D_CHECK_FAILURE(GetDefaultCommandList()->Close());

		CommandBuffer* cb = GetDefaultCommandBuffer();

		SubmitGraphics_({ &cb,1  });
		WaitUntilIdle_();
	}

	void D3DGraphicsContext::WaitUntilIdle_()
	{
		SKTBD_MSG_WARN("Flushing GPU commands -- Woosh!");

		// Advance the fence value to mark commands up to this fence point
		uint64_t currentFence = NextFence();

		// Set a new fence point on the command queue. Because we are on the GPU timeline, the new fence point won't
		// be set until the GPU finishes processing all the commands prior to this Signal()
		D3D_CHECK_FAILURE(m_GraphicsCommandQueue->Signal(m_Fence.Get(), currentFence));

		// Wait until the GPU has completed commands up to the fence point
		if (m_Fence->GetCompletedValue() < currentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, NULL, NULL, EVENT_ALL_ACCESS);
			SKTBD_LOG_ASSERT(eventHandle != NULL, "Fence event created an invalid handle!");

			// Fire the previously created event when GPU hits current fence
			D3D_CHECK_FAILURE(m_Fence->SetEventOnCompletion(currentFence, eventHandle));

			// Wait until the GPU hits the current fence and the event is received
			WaitForSingleObject(eventHandle, INFINITE);

			// Close the event so that it does not exist anymore
			CloseHandle(eventHandle);
		}
	}

	//TODO
	CopyResult D3DGraphicsContext::CopyDataToBuffer_(Buffer* dest, off_t offset, size_t size, void* src)
	{
		SKTBD_LOG_ASSERT(dest, "Can not copy to null or from null");
		auto DEST = static_cast<D3DBuffer*>(dest)->m_BufferResource->GetResource();

		if (dest->GetAccessFlags() & ResourceAccessFlag_CpuWrite)
		{
			// Copy the triangle data to the vertex buffer.
			UINT8* pDataBegin;
			CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			D3D_CHECK_FAILURE(DEST->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
			memcpy(pDataBegin + offset, src, size);
			DEST->Unmap(0, nullptr);
		}
		else
		{
			m_UploadManager.UploadBuffer(DEST, offset, src, size);
		}

		return CopyResult();
	}

	CopyResult D3DGraphicsContext::CopyDataToBuffer_(Buffer* dest, off_t offset, size_t size, std::function<void(void*)> WriterFunct)
	{
		SKTBD_LOG_ASSERT(dest, "Can not copy to null");

		auto DEST = static_cast<D3DBuffer*>(dest)->m_BufferResource->GetResource();

		if (dest->GetAccessFlags() & ResourceAccessFlag_CpuWrite)
		{
			// Copy the triangle data to the vertex buffer.
			UINT8* pDataBegin;
			CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			D3D_CHECK_FAILURE(DEST->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
			WriterFunct(pDataBegin + offset);
			DEST->Unmap(0, nullptr);
		}
		else
		{
			m_UploadManager.UploadBuffer(DEST, offset, size, WriterFunct);
		}

		return CopyResult();
	}

	void D3DGraphicsContext::SubmitCompute_(const ComputeSubmitInfo& submit)
	{
		std::vector<ID3D12CommandList*> buffers(submit.BufferCount);

		std::transform(&submit.CommandBuffers[0], &submit.CommandBuffers[submit.BufferCount], buffers.begin(), [submit](CommandBuffer* in) {return reinterpret_cast<ID3DCommandBufferInterface*>(in)->GetCommandList(); });

		m_ComputeCommandQueue->ExecuteCommandLists(submit.BufferCount, buffers.data());
	}

	void D3DGraphicsContext::SubmitGraphics_(const GraphicsSubmitInfo& submit)
	{
		std::vector<ID3D12CommandList*> buffers(submit.BufferCount);
		std::transform(&submit.CommandBuffers[0], &submit.CommandBuffers[submit.BufferCount], buffers.begin(), [submit](CommandBuffer* in) {return static_cast<D3DGraphicsCommandBuffer*>(in)->m_CommandList.Get(); });

		m_GraphicsCommandQueue->ExecuteCommandLists(submit.BufferCount, buffers.data());
	}

	void D3DGraphicsContext::GraphicsSignalFence_(Fence* fence, uint64_t value)
	{
		auto F = static_cast<D3DFence*>(fence);
		m_GraphicsCommandQueue->Signal(F->m_FenceObject.Get(), value);
	}

	void D3DGraphicsContext::ComputeSignalFence_(Fence* fence, uint64_t value)
	{
		auto F = static_cast<D3DFence*>(fence);
		m_ComputeCommandQueue->Signal(F->m_FenceObject.Get(), value);
	}

	void D3DGraphicsContext::GraphicsWaitFence_(Fence* fence, uint64_t value)
	{
		auto F = static_cast<D3DFence*>(fence);
		m_GraphicsCommandQueue->Wait(F->m_FenceObject.Get(), value);
	}

	void D3DGraphicsContext::ComputeWaitFence_(Fence* fence, uint64_t value)
	{
		auto F = static_cast<D3DFence*>(fence);
		m_ComputeCommandQueue->Wait(F->m_FenceObject.Get(), value);
	}

	CopyResult D3DGraphicsContext::WriteTopLevelASInstanceDataToBuffer_(Buffer* dest, off_t offset, size_t num,
	                                                                    TLASInstanceData* data)
	{
		SKTBD_LOG_ASSERT(is_aligned(offset, D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT), "Descriptions are not aligned to GraphicsConstants::RAYTRACING_TLAS_INSTANCE_DESC_ALIGNMENT")
		std::function<void(void*)> WriterFunc = [&](void* Instance) -> void
		{
			auto desc = static_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(Instance);

			std::transform(&data[0], &data[num],&desc[0], [] (const TLASInstanceData& in )->D3D12_RAYTRACING_INSTANCE_DESC
			{
				D3D12_RAYTRACING_INSTANCE_DESC out{};
				auto m = glm::transpose(in.Transform);
				memcpy(out.Transform, &m, sizeof(out.Transform));
				out.Flags = in.TlasFlags & 0xF; //D3D12_RAYTRACING_INSTANCE_FLAGS MAX is 0xF Safety net
				out.AccelerationStructure = in.BottomASHandle;
				out.InstanceID = in.InstanceID;
				out.InstanceContributionToHitGroupIndex = in.InstanceContributionToHitGroupIndex;
				out.InstanceMask = in.InstanceMask;

				return out;
			});
		};

		auto DEST = static_cast<D3DBuffer*>(dest)->GetResource();

		if (dest->GetAccessFlags() & ResourceAccessFlag_CpuWrite)
		{
			// Copy the triangle data to the vertex buffer.
			UINT8* pDataBegin;
			CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			D3D_CHECK_FAILURE(DEST->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
			WriterFunc(pDataBegin + offset);
			DEST->Unmap(0, nullptr);
		}
		else
		{
			m_UploadManager.UploadBuffer(DEST, offset, num*sizeof(D3D12_RAYTRACING_INSTANCE_DESC), WriterFunc);
		}

		return CopyResult();
	}

	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> D3DGraphicsContext::ConvertSkateboardBlasDescToD3DGeometries(const BottomLevelAccelerationStructureDesc& Descriptions)
	{
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometries(Descriptions.NumGeometries);

		std::transform(&Descriptions.Geometries[0], &Descriptions.Geometries[Descriptions.NumGeometries], geometries.begin(), [](const GeometryDesc& Desc) -> D3D12_RAYTRACING_GEOMETRY_DESC
			{
				D3D12_RAYTRACING_GEOMETRY_DESC geometry{};

				geometry.Type = GeometryTypeToD3D(Desc.Type);
				geometry.Flags = GeometryFlagsToD3D(Desc.Flags);
				if (geometry.Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
				{
					// Sanity checks
					SKTBD_LOG_ASSERT(Desc.Triangles.pVertexBuffer != nullptr, "A vertex buffer is required to create this object's acceleration structure.");
					SKTBD_LOG_ASSERT(Desc.Triangles.VertexCount != 0u, "A vertex buffer is required to create this object's acceleration structure.");
					SKTBD_LOG_ASSERT(Desc.Triangles.pIndexBuffer != nullptr, "An index buffer is required to create this object's acceleration structure.");
					SKTBD_LOG_ASSERT(Desc.Triangles.IndexCount != 0u, "An index buffer is required to create this object's acceleration structure.");

					// Get the vertex & index start locations
					const uint32_t vertexStride = Desc.Triangles.pVertexBuffer->m_VertexStride;
					const uint32_t indexStride = (Desc.Triangles.pIndexBuffer->m_Format == IndexFormat::bit16) ? sizeof(int16_t) : sizeof(int32_t);
					const D3D12_GPU_VIRTUAL_ADDRESS vertexStart = static_cast<D3DBuffer*>(Desc.Triangles.pVertexBuffer->m_ParentResource.get())->m_BufferResource->GetResource()->GetGPUVirtualAddress() + Desc.Triangles.pVertexBuffer->m_Offset + Desc.Triangles.StartVertexLocation * vertexStride;
					const D3D12_GPU_VIRTUAL_ADDRESS indexStart = static_cast<D3DBuffer*>(Desc.Triangles.pIndexBuffer->m_ParentResource.get())->m_BufferResource->GetResource()->GetGPUVirtualAddress() + Desc.Triangles.pIndexBuffer->m_Offset + Desc.Triangles.StartIndexLocation * indexStride;

					geometry.Triangles.VertexBuffer.StartAddress = vertexStart;
					geometry.Triangles.VertexBuffer.StrideInBytes = vertexStride;
					geometry.Triangles.VertexCount = Desc.Triangles.VertexCount;
					geometry.Triangles.VertexFormat = SkateboardBufferFormatToD3D(Desc.Triangles.VertexPositionFormat);
					geometry.Triangles.IndexBuffer = indexStart;
					geometry.Triangles.IndexFormat = (Desc.Triangles.pIndexBuffer->m_Format == bit16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
					geometry.Triangles.IndexCount = Desc.Triangles.IndexCount;
					geometry.Triangles.Transform3x4 = NULL;
				}
				else
				{
					// Sanity checks
					SKTBD_LOG_ASSERT(Desc.Procedurals.AABBCount > 0u, "Cannot instanciate a zero procedural primitive BLAS.");
					SKTBD_LOG_ASSERT(Desc.Procedurals.BufferAABB.pProceduralGeometryAABBBuffer != nullptr, "A procedural primitive buffer that contains the AABBs is required to create this object's acceleration structure");

					geometry.AABBs.AABBCount = Desc.Procedurals.AABBCount;
					geometry.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
					geometry.AABBs.AABBs.StartAddress = static_cast<D3DBuffer*>(Desc.Procedurals.BufferAABB.pProceduralGeometryAABBBuffer)->m_BufferResource->GetResource()->GetGPUVirtualAddress() + Desc.Procedurals.BufferAABB.Offset * geometry.AABBs.AABBs.StrideInBytes;
				}

				return geometry;
			});

		return geometries;
	}

	RaytracingASSizeInfo D3DGraphicsContext::QueryAccelerationStructureSizeReq_(const AccelerationStructureDesc& AS_Desc, SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS Flags)
	{
		ID3D12Device5* pDevice = gD3DContext->GetDevice();
		
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS preBuildDesc{};
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometries;

		// Query the the memory needed to build the BLAS and to store the fully built structure
		// In order to perform this query, a description of the above initialised geometry must be defined

		switch (AS_Desc.m_Type)
		{
		case BottomLevel:
			geometries = std::move(ConvertSkateboardBlasDescToD3DGeometries(std::get<BottomLevelAccelerationStructureDesc>(AS_Desc.m_Desc)));

			preBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			preBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			preBuildDesc.pGeometryDescs = geometries.data();
			preBuildDesc.NumDescs = geometries.size();

			break;
		case TopLevel:
			preBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			preBuildDesc.NumDescs = std::get<TopLevelAccelerationStructureDesc>(AS_Desc.m_Desc).NumInstances;

			break;
		}

		preBuildDesc.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS(Flags);

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};
		pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&preBuildDesc, &info);	// "info" now holds the buffers sizes

		RaytracingASSizeInfo ret;

		ret.StorageSize = info.ResultDataMaxSizeInBytes;
		ret.ScratchSizeBuild = info.ScratchDataSizeInBytes;
		ret.ScratchSizeUpdate = info.UpdateScratchDataSizeInBytes;

		return ret;
	}

	ShaderTable D3DGraphicsContext::BuildShaderTable_(const BufferRef& target, off_t offset, ComputePipeline* pso,
		const RaytracingPipelineDesc& desc, ShaderTableGroup group)
	{
		ID3D12Device5* pDevice = gD3DContext->GetDevice();
		auto pState =static_cast<D3DComputePipeline*>(pso)->m_State;

		// Get the identifiers of the shaders
		// A shader identifier is an opaque data blob of 32 bytes that uniquely identifies (within the current device / process)
		// one of the raytracing shaders: ray generation shader, hit group, miss shader, callable shader.
		// The application can request the shader identifier for any of these shaders from the system. It can be thought of as a pointer to a shader (MSDN).
		// The shader table also provides the local root arguments of local root signatures associated with these shaders.
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> rtpsoProperties;
		D3D_CHECK_FAILURE(pState.As(&rtpsoProperties));	// Get the RTPSO properties

		SizedPtr pRayGenShaderIdentifier;

		std::vector<SizedPtr> vGroupShaderIdentifiers;

		switch (group)
		{
		case Hit:
			for (const RaytracingHitGroup& hitGroup : desc.RaytracingShaders.HitGroups)
				vGroupShaderIdentifiers.push_back({ rtpsoProperties->GetShaderIdentifier(hitGroup.HitGroupName), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES });
			break;
		case Miss:
			for (const wchar_t* missShaderEntryPoint : desc.RaytracingShaders.MissShaderEntryPoints)
				vGroupShaderIdentifiers.push_back({ rtpsoProperties->GetShaderIdentifier(missShaderEntryPoint), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES });
			break;
		case Callable:
			for (const wchar_t* callableShader : desc.RaytracingShaders.CallableShaders)
				vGroupShaderIdentifiers.push_back({ rtpsoProperties->GetShaderIdentifier(callableShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES });
			break;
		case Raygen:
			pRayGenShaderIdentifier = { rtpsoProperties->GetShaderIdentifier(desc.RaytracingShaders.RayGenShaderEntryPoint), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES };
			break;
		}

		//	std::vector<UINT64> vRootArguments;
		//	constexpr const size_t maxRootArguments = 255u;
		//	vRootArguments.reserve(maxRootArguments);
		//	SizedPtr pRootArguments = { nullptr, 0 };

		// Retrieve the shader identifiers

		auto d3dBuffer = static_cast<D3DBuffer*>(target.get());

		ShaderTable ret;

		ret.Offset = offset;
		ret.Size = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		ret.Stride = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
		ret.m_StorageBuffer = target;
		ret.m_Address = d3dBuffer->GetResourceGPUAddress() + offset;

		if (group == Raygen)
			CopyDataToBuffer_(target.get(), offset, pRayGenShaderIdentifier.Size, pRayGenShaderIdentifier.Ptr);
		else
			CopyDataToBuffer_(target.get(), offset, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT * vGroupShaderIdentifiers.size(), [&](void* dest)->void
			{
				//pointermathmybeloved
				off_t offt = 0;
				for (auto const& i : vGroupShaderIdentifiers)
				{
					memcpy(((uint8_t*)dest+offt), i.Ptr, i.Size);
					offt += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
				}
			});

		return ret;
		/*BufferDesc bufferDesc;
		bufferDesc.Init(32 * (1 + vMissShaderIdentifiers.size() + vHitGroupShaderIdentifiers.size() + vCallableShaderIdentifiers.size()), ResourceAccessFlag_CpuWrite);*/

		/*auto STBuffer = std::make_shared<D3DBuffer>(bufferDesc);
		ret.m_StorageBuffer = std::static_pointer_cast<Buffer>(STBuffer);*/


		//// RayGenerationShader record
		//if (m_LocalRaygenRootSignature.Get())
		//{
		//	// Constant buffers are directly bound to the root signature
		//	for (D3DUploadBuffer*& buffer : v_LocalRayGenConstantBuffers)
		//		vRootArguments.push_back(buffer->GetGPUVirtualAddress());

		//	// Input (Skateboard) Descriptor tables
		//	for (D3DDescriptorTable*& table : v_LocalRayGenDescriptorTables)
		//		vRootArguments.push_back(table->GetGPUHandle().ptr);

		//	// SRVs and UAVs are part of the Descriptor table
		//	if (m_LocalRaygenDescriptorTableHandle.IsValid())
		//		vRootArguments.push_back(m_LocalRaygenDescriptorTableHandle.GetGPUHandle().ptr);

		//	pRootArguments = { vRootArguments.data(), static_cast<uint32_t>(vRootArguments.size() * sizeof(UINT64)) };
		//	m_ShaderTable.SetRayGenShader(ShaderRecord(pRayGenShaderIdentifier, pRootArguments));
		//}
		//else
		/* m_ShaderTable.SetRayGenShader(ShaderRecord(pRayGenShaderIdentifier));*/

		// HitGroup record
		//if (m_LocalHitGroupRootSignature.Get())
		//{
		//	const size_t vectorStart = vRootArguments.size();

		//	// Constant buffers are directly bound to the root signature
		//	for (D3DUploadBuffer*& buffer : v_LocalHitGroupConstantBuffers)
		//		vRootArguments.push_back(buffer->GetGPUVirtualAddress());

		//	// Input (Skateboard) Descriptor tables
		//	for (D3DDescriptorTable*& table : v_LocalHitGroupDescriptorTables)
		//		vRootArguments.push_back(table->GetGPUHandle().ptr);

		//	// SRVs and UAVs are part of the Descriptor table
		//	if (m_LocalHitGroupDescriptorTableHandle.IsValid())
		//		vRootArguments.emplace_back(m_LocalHitGroupDescriptorTableHandle.GetGPUHandle().ptr);

		//	pRootArguments = { vRootArguments.data() + vectorStart, static_cast<uint32_t>(vRootArguments.size() - vectorStart * sizeof(UINT64)) };
		//	for (const SizedPtr& ptr : vHitGroupShaderIdentifiers)
		//		m_ShaderTable.AddHitGroupShaders(ShaderRecord(ptr, pRootArguments));
		//}
		/*for (const SizedPtr& ptr : vHitGroupShaderIdentifiers)
			m_ShaderTable.AddHitGroupShaders(ShaderRecord(ptr));*/

			// MissShader records
			//if (m_LocalMissRootSignature.Get())
			//{
			//	const size_t vectorStart = vRootArguments.size();

			//	// Constant buffers are directly bound to the root signature
			//	for (D3DUploadBuffer*& buffer : v_LocalMissConstantBuffers)
			//		vRootArguments.push_back(buffer->GetGPUVirtualAddress());

			//	// Input (Skateboard) Descriptor tables
			//	for (D3DDescriptorTable*& table : v_LocalMissDescriptorTables)
			//		vRootArguments.push_back(table->GetGPUHandle().ptr);

			//	// SRVs and UAVs are part of the Descriptor table
			//	if (m_LocalMissDescriptorTableHandle.IsValid())
			//		vRootArguments.push_back(m_LocalMissDescriptorTableHandle.GetGPUHandle().ptr);

			//	pRootArguments = { vRootArguments.data() + vectorStart, static_cast<uint32_t>(vRootArguments.size() - vectorStart * sizeof(UINT64)) };
			//	for (const SizedPtr& ptr : vMissShaderIdentifiers)
			//		m_ShaderTable.AddMissShader(ShaderRecord(ptr, pRootArguments));
			//}
			/*for (const SizedPtr& ptr : vMissShaderIdentifiers)
				m_ShaderTable.AddMissShader(ShaderRecord(ptr));*/

				//if (m_LocalCallableRootSignature.Get())
				//{
				//	const size_t vectorStart = vRootArguments.size();

				//	// Constant buffers are directly bound to the root signature
				//	for (D3DUploadBuffer*& buffer : v_LocalCallableConstantBuffers)
				//		vRootArguments.push_back(buffer->GetGPUVirtualAddress());

				//	// Input (Skateboard) Descriptor tables
				//	for (D3DDescriptorTable*& table : v_LocalCallableDescriptorTables)
				//		vRootArguments.push_back(table->GetGPUHandle().ptr);

				//	// SRVs and UAVs are part of the Descriptor table
				//	if (m_LocalCallableDescriptorTableHandle.IsValid())
				//		vRootArguments.push_back(m_LocalCallableDescriptorTableHandle.GetGPUHandle().ptr);

				//	pRootArguments = { vRootArguments.data() + vectorStart, static_cast<uint32_t>(vRootArguments.size() - vectorStart * sizeof(UINT64)) };
				//	for (const SizedPtr& ptr : vCallableShaderIdentifiers)
				//		m_ShaderTable.AddCallableShader(ShaderRecord(ptr, pRootArguments));
				//}
				/*for (const SizedPtr& ptr : vCallableShaderIdentifiers)
					m_ShaderTable.AddCallableShader(ShaderRecord(ptr));*/

					// Finally, store everything in the buffer
					//SKTBD_LOG_ASSERT(vRootArguments.size() < maxRootArguments, "Corrupeted vector. TODO: Refactor this in a nicer way.");
					/*m_ShaderTable.GenerateShaderTableBuffer(pDevice);*/
	}



	ID3D12GraphicsCommandList10* D3DGraphicsContext::GetDefaultCommandList() const
	{
		return std::static_pointer_cast<D3DGraphicsCommandBuffer>(m_DefaultGraphicsCB.Get())->m_CommandList.Get();
	}

	ID3D12CommandAllocator* D3DGraphicsContext::GetDefaultCommandAllocator() const
	{
		return std::static_pointer_cast<D3DGraphicsCommandBuffer>(m_DefaultGraphicsCB.Get())->m_Allocator.Get();
	}
	

	



	void D3DGraphicsContext::SetDeferredReleasesFlag()
	{
		m_DeferredFlags[GetCurrentFrameResourceIndex()] = 1;
	}

	void D3DGraphicsContext::DeferredRelease(IUnknown* resource)
	{
		const uint64_t frame = GetCurrentFrameResourceIndex();

		// Lock this function, prevents threads from over writing deferrals.
		std::lock_guard lock(m_DeferredMutex);

		m_DeferredReleases[frame].push_back(resource);
		SetDeferredReleasesFlag();
	}

	void __declspec(noinline) D3DGraphicsContext::ProcessDeferrals()
	{
		std::lock_guard lock(m_DeferredMutex);

		m_DeferredFlags[m_CurrentFrameResourceIndex] = 0;

		// Process deferred descriptors.
		m_RWSRVDescriptorHeap.ProcessDeferredFree(m_CurrentFrameResourceIndex);
		m_SRVDescriptorHeap.ProcessDeferredFree(m_CurrentFrameResourceIndex);
		m_RTVDescriptorHeap.ProcessDeferredFree(m_CurrentFrameResourceIndex);
		m_DSVDescriptorHeap.ProcessDeferredFree(m_CurrentFrameResourceIndex);

		//Clear the deferred resources.
		std::vector<IUnknown*>& resources{ m_DeferredReleases[m_CurrentFrameResourceIndex] };
		if(!resources.empty())
		{
			for(const auto& resource : resources)
			{
				resource->Release();
			}
			resources.clear();
		}

	}

	bool D3DGraphicsContext::CheckDeviceRemovedStatus() const
	{
		const HRESULT result = m_Device->GetDeviceRemovedReason();
		if (result != S_OK)
		{
			SKTBD_MSG_ERROR(L"GetDevice removed reason: {}", DXException(result).ToString().c_str());
			return true;
		}
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3DGraphicsContext::CurrentBackBufferView() const
	{
		// Offset to the RTV of the current back buffer and return the location of the according descriptor
		//D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
		//handle.ptr = m_RTVHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_CurrentBackBuffer * m_RTVDescriptorSize;		// Note: Overflow higly improbable as m_CurrentBackBuffer usually ranges in [0,3]
		//return handle;
		return static_cast<D3DRenderTargetView*>(m_SwapChainRTVs[m_CurrentBackBuffer].get())->m_Descriptor.GetCPUHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3DGraphicsContext::DepthStencilViewHandle() const
	{
		// There is only one descriptor for this heap, just return where it starts
		//return m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
		return static_cast<D3DDepthStencilView*>(m_DefaultDSV.get())->m_Descriptor.GetCPUHandle();
	}

}