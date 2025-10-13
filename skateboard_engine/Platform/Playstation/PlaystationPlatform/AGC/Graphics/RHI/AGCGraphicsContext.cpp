#include <sktbdpch.h>
#include "AGCGraphicsContext.h"
#include "Skateboard/Graphics/InternalFormats.h"

#define SKTBD_LOG_COMPONENT "AGCGraphicsContext"
#include "AGC/Graphics/Resources/AGCBuffer.h"
#include "Skateboard/Log.h"

#include "AGC/Graphics/Resources/AGCCommandBuffer.h"

namespace Skateboard
{
	AGCGraphicsContext* gAGCContext = nullptr;
	GraphicsContext* GraphicsContext::Context = nullptr;

	namespace GraphicsConstants
	{
		size_t DEFAULT_RESOURCE_ALIGNMENT = sce::Agc::Alignment::kMaxTiledAlignment;;		// NOTSURE
		size_t SMALL_RESOURCE_ALIGNMENT = sce::Agc::Alignment::kMaxTiledAlignment;			// NOTSURE
		size_t MSAA_RESOURCE_ALIGNEMNT = sce::Agc::Alignment::kMaxTiledAlignment;			// NOTSURE
		size_t SMALL_MSAA_RESOURCE_ALIGNMENT = sce::Agc::Alignment::kMaxTiledAlignment;		// NOTSURE
		size_t CONSTANT_BUFFER_ALIGNMENT = sce::Agc::Alignment::kBuffer;
		size_t BUFFER_ALIGNMENT = sce::Agc::Alignment::kBuffer;
		size_t RAYTRACING_STRUCT_ALIGNMENT = sce::Psr::kRequiredAlignment;					// NOTSURE
		size_t RAYTRACING_TLAS_INSTANCE_DESC_ALIGNEMNT = sce::Psr::kRequiredAlignment;		// NOTSURE

		size_t RAYTRACING_SHADER_TABLE_ALIGNMENT = sce::Psr::kRequiredAlignment;			// NOTSURE
		size_t RAYTRACING_SHADER_TABLE_SHADER_ID_ALIGNMENT = sce::Psr::kRequiredAlignment;	// NOTSURE
		size_t RAYTRACING_SHADER_TABLE_RECORD_ALIGNMENT = sce::Psr::kRequiredAlignment;		// NOTSURE

	}

	constexpr AGCGraphicsCommandBuffer* GetDCB(CommandBuffer* cb)
	{
		return static_cast<AGCGraphicsCommandBuffer*>(cb);
	}

	AGCGraphicsContext::AGCGraphicsContext(const PlatformProperties& props) :
		m_VideoHandle(0),
		m_VideoSetIndex(0),
		m_LatencyControl{}
	{
		// Assign the singleton
		SKTBD_ASSERT(!Context, "GraphicsContext already exists! Only one context can be created.");
		Context = this;
		gAGCContext = this;

		InitialiseAGC();
		InitVideoOutAndPollResolution();
		CreateDrawCommandAndStateBuffers();
		CreateFilpLabels();
		CreateRenderTargets();
		CreateDepthTarget();
		BindRenderTargetsToVideoOut();
		CreateViewPort();

		m_DescriptorHeap.InitDescriptorHeap(m_MemoryAllocator,"ResourceDescriptorHeap");
		m_SamplerHeap.InitDescriptorHeap(m_MemoryAllocator,"SamplerHeap");
	}

	AGCGraphicsContext::~AGCGraphicsContext()
	{
		// Release the video
		sceVideoOutUnregisterBuffers(m_VideoHandle, m_VideoSetIndex);

		// Release memory
		m_MemoryAllocator.TypedAlignedFree(m_ResourceRegistrationMemory);
		m_MemoryAllocator.TypedAlignedFree(m_DepthStencilTargetMemory);
		m_MemoryAllocator.TypedAlignedFree(m_DepthStencilHTileMemory);
		m_MemoryAllocator.TypedAlignedFree(m_FlipLabelsMemory);
		for (uint32_t i = 0u; i < GRAPHICS_SETTINGS_NUMFRAMERESOURCES; ++i)
		{
			m_MemoryAllocator.TypedAlignedFree(m_DrawCommandBuffersMemory[i]);
			m_MemoryAllocator.TypedAlignedFree(m_RenderTargetsMemory[i]);
		}
	}

	sce::Agc::DrawCommandBuffer& AGCGraphicsContext::GetDrawCommandBuffer()
	{
		return GetDCB(m_DefaultGraphicsCB.Get().get())->m_DCB; 
	}

	sce::Agc::Core::StateBuffer& AGCGraphicsContext::GetStateBuffer()
	{
		return GetDCB(m_DefaultGraphicsCB.Get().get())->m_STB;
	}

	void AGCGraphicsContext::InitialiseAGC()
	{
		SceError error = sce::Agc::init();
		SCE_AGC_ASSERT(error == SCE_OK);

		size_t resourceRegistrationBufferSize;
		error = sce::Agc::ResourceRegistration::queryMemoryRequirements(&resourceRegistrationBufferSize, 1024, 64);
		if (error != SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG)
		{
			SCE_AGC_ASSERT(error == SCE_OK);
			m_ResourceRegistrationMemory = m_MemoryAllocator.TypedAlignedAllocate<uint8_t,sce::Agc::Alignment::kResourceRegistration>(resourceRegistrationBufferSize);
			error = sce::Agc::ResourceRegistration::init(m_ResourceRegistrationMemory.data, resourceRegistrationBufferSize, 64);
			SCE_AGC_ASSERT(error == SCE_OK);
			error = sce::Agc::ResourceRegistration::registerDefaultOwner(nullptr);
			SCE_AGC_ASSERT(error == SCE_OK);
		}

		error = sce::Agc::Toolkit::init();
		SCE_AGC_ASSERT(error == SCE_OK);
	}

	void AGCGraphicsContext::InitVideoOutAndPollResolution()
	{
		// First we need to select what we want to display on, which in this case is the TV, also known as SCE_VIDEO_OUT_BUS_TYPE_MAIN.
		m_VideoHandle = sceVideoOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);
		SCE_AGC_ASSERT_MSG(m_VideoHandle >= 0, "sceVideoOutOpen() returns handle=%d\n", m_VideoHandle);

		SceVideoOutOutputStatus outputStatus = {};
		sceVideoOutGetOutputStatus(m_VideoHandle, &outputStatus);
		uint32_t screenWidth = 0, screenHeight = 0;
		switch (outputStatus.resolution)
		{
		case SCE_VIDEO_OUT_OUTPUT_RESOLUTION_HD:
			m_ClientWidth = 1920;
			m_ClientHeight = 1080;
			break;
		case SCE_VIDEO_OUT_OUTPUT_RESOLUTION_4K:
			m_ClientWidth = 3840;
			m_ClientHeight = 2160;
			break;
		default:
			SCE_AGC_ASSERT(false);
			break;
		}
	}

	bool DCBOutOfMemoryCallback(sce::Agc::TwoSidedAllocator* alloc, uint32_t sizeInDwords, void* userData)
	{
		// This is the callback which is invoked when the current operation leaves less than m_reservedSpaceInDwords 
		// of free memory.

		// This function will return true if the requested space is available on alloc() otherwise false.


		return false;
	}

	void DCBOutOfMemoryUserData()
	{



	}

	//Data Shuffling
	CopyResult AGCGraphicsContext::CopyDataToBuffer_(Buffer* dest_buffer, off_t offset, size_t size, void* src)
	{
		auto dst = static_cast<uint8_t*>(static_cast<AGCBuffer*>(dest_buffer)->BufferMemory.m_Data);
		std::memcpy(dst + offset, src, size);

		return nullptr;
	};

	CopyResult AGCGraphicsContext::CopyDataToBuffer_(Buffer* dest_buffer, off_t offset, size_t size, std::function<void(void*)> WriterFunct)
	{
		auto dst = static_cast<uint8_t*>(static_cast<AGCBuffer*>(dest_buffer)->BufferMemory.m_Data);
		WriterFunct(dst + offset);
		return nullptr;
	};

	//Raytracing Data
	CopyResult AGCGraphicsContext::WriteTopLevelASInstanceDataToBuffer_(Buffer* dest, off_t offset, size_t num, TLASInstanceData* data)
	{
		return nullptr;
	};

	void AGCGraphicsContext::CreateDrawCommandAndStateBuffers()
	{
		// Create a draw command buffer and state buffer for each frame resource. Note that we will not consider
		// resizing the dcb when running out of memory.
		//const uint32_t dcb_size = 2 * 1024 * 1024;	// ~8 MB per command list should be enough, (allocating uint 32s -> size 4)

		m_DefaultGraphicsCB.ForEach([](GraphicsCommandBufferRef& ref) {ref = ResourceFactory::CreateGraphicsCommandBuffer(CommandBufferPriority_Primary); });

		for (uint32_t i = 0; i < GRAPHICS_SETTINGS_NUMFRAMERESOURCES; ++i)
		{
			auto buffer = static_cast<AGCGraphicsCommandBuffer*>(m_DefaultGraphicsCB[i].get());

#ifndef SKTBD_SHIP
			sce::Agc::Core::registerResource(&buffer->m_DCB, "DCB %d", i);
			sce::Agc::Core::registerResource(&m_FlipLabels[i], "Flip label %d", i);
#endif // !SKTBD_SHIP
		}
	}

	void AGCGraphicsContext::CreateFilpLabels()
	{
		// The flip labels are used to track if a frame resource is currently being used by the GPU.
		// In other words, if the flabel.m_value is 0 then the GPU is currently using the frame resource and we should wait until it is done.
		m_FlipLabelsMemory = m_MemoryAllocator.TypedAlignedAllocate<sce::Agc::Label,sce::Agc::Alignment::kLabel>(GRAPHICS_SETTINGS_NUMFRAMERESOURCES);
		m_FlipLabels = m_FlipLabelsMemory.data;
		for (uint32_t i = 0; i < GRAPHICS_SETTINGS_NUMFRAMERESOURCES; ++i)
			m_FlipLabels[i].m_value = 1; // 1 means "not used by GPU"
	}

	void AGCGraphicsContext::CreateRenderTargets()
	{
		// Set up the RenderTarget spec
		sce::Agc::Core::RenderTargetSpec rtSpec;
		rtSpec.init();
		rtSpec.m_width = static_cast<uint32_t>(m_ClientWidth);
		rtSpec.m_height = static_cast<uint32_t>(m_ClientHeight);
		rtSpec.m_format = { sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 }; // matching Dx12 R8G8B8A8 UNORM
		//rtSpec.m_format = BufferFormatToAGC(); // TODO: Get it from props
		rtSpec.m_tileMode = sce::Agc::CxRenderTarget::TileMode::kRenderTarget;

		// Get the aligned size of the render target and allocate the require memory backing
		sce::Agc::SizeAlign rtSize = sce::Agc::Core::getSize(&rtSpec);
		m_RenderTargetsMemory[0] = m_MemoryAllocator.TypedAlignedAllocate<uint8_t,sce::Agc::Alignment::kMaxTiledAlignment>(rtSize.m_size);
		rtSpec.m_dataAddress = m_RenderTargetsMemory[0].data;

		// We can now initialize the render target. This will check that the dataAddress is properly aligned
		SceError error = sce::Agc::Core::initialize(&m_RenderTargets[0], &rtSpec);
		SCE_AGC_ASSERT_MSG(error == SCE_OK, "Failed to initialize RenderTarget.");

#ifndef SKTBD_SHIP
		sce::Agc::Core::registerResource(&m_RenderTargets[0], "Color %d", 0);
#endif // !SKTBD_SHIP

		// Now that we have the first RT set up, we will use it to initialise all the others.
		// The only difference is that each render target needs a unique dataAddress!
		for (uint32_t i = 1u; i < GRAPHICS_SETTINGS_NUMFRAMERESOURCES; ++i)
		{
			m_RenderTargets[i] = m_RenderTargets[0];
			m_RenderTargetsMemory[i] = m_MemoryAllocator.TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment>(rtSize.m_size);
			m_RenderTargets[i].setDataAddress(m_RenderTargetsMemory[i].data);
#ifndef SKTBD_SHIP
			sce::Agc::Core::registerResource(&m_RenderTargets[i], "Color %d", i);
#endif // !SKTBD_SHIP
		}

		// Enable writing to all the channels of the render target
		m_RenderTargetMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xf);
	}

	void AGCGraphicsContext::CreateDepthTarget()
	{
		//Set up the DepthRenderTarget spec
		sce::Agc::Core::DepthRenderTargetSpec drtSpec = {};
		drtSpec.init();																	// Always call init()
		drtSpec.m_width = static_cast<uint32_t>(m_ClientWidth);							// The depth buffer will match our application client size
		drtSpec.m_height = static_cast<uint32_t>(m_ClientHeight);
		drtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k32Float;	// We want a 32-bit floating point depth buffer
		drtSpec.m_compression = sce::Agc::Core::MetadataCompression::kHtileDepth;		// Compression for depth and stencil
		//TODO: drtSpec.m_stencilFormat = sce::Agc::CxDepthRenderTarget::StencilFormat::k8UInt;	// We want a 8-bit unsigned integer stencil buffer
		//TODO: drtSpec.m_compression = sce::Agc::Core::MetadataCompression::kHtileStencil;		// Compression for depth AND stencil

		// Get the aligned size of the depth target and allocate the required memory backing (for the locations on where to read and write to it)
		sce::Agc::SizeAlign dtSize = sce::Agc::Core::getSize(&drtSpec, sce::Agc::Core::DepthRenderTargetComponent::kDepth);
		m_DepthStencilTargetMemory = m_MemoryAllocator.TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment>(dtSize.m_size);
		drtSpec.m_depthReadAddress = drtSpec.m_depthWriteAddress = m_DepthStencilTargetMemory.data;

		//// Get the aligned size of the stencil target and allocate the required memory backing (for the locations on where to read and write to it)
		//TODO: sce::Agc::SizeAlign stSize = sce::Agc::Core::getSize(&drtSpec, sce::Agc::Core::DepthRenderTargetComponent::kStencil);
		//TODO: drtSpec.m_stencilReadAddress = drtSpec.m_stencilWriteAddress = allocDmem(stSize);

		// Retrieve the size of htile buffer for this depth render target and allocate the required memory backing.
		// The HTILE buffer is optional. It contains a DWORD that serves as a summary of the depth buffer,
		// which can be used to speed up depth clears or even sometimes avoid depth reads entirely!
		// Read: https://p.siedev.net/resources/documents/SDK/7.000/Agc-Reference/0795.html
		sce::Agc::SizeAlign htileSize = sce::Agc::Core::getSize(&drtSpec, sce::Agc::Core::DepthRenderTargetComponent::kHtile);
		m_DepthStencilHTileMemory = m_MemoryAllocator.TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment>(htileSize.m_size);
		drtSpec.m_htileAddress = m_DepthStencilHTileMemory.data;

		// We can now initialize the depth render target. This will check that the addresses are properly aligned
		SceError error = sce::Agc::Core::initialize(&m_DepthStencilTarget, &drtSpec);
		SCE_AGC_ASSERT_MSG(error == SCE_OK, "Failed to initialize DepthRenderTarget.");

#ifndef SKTBD_SHIP
		sce::Agc::Core::registerResource(&m_DepthStencilTarget, "Depth");
#endif // !SKTBD_SHIP

		// The depth/stencil render targets also own the clear value
		m_DepthStencilTarget.setDepthClearValue(1.0f);
		//TODO: m_DepthStencilTarget.setStencilClearValue(0u);

		// Enable depth testing and writing, as well as stencil testing
		m_DepthStencilControl.init();
		m_DepthStencilControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kEnable);
		m_DepthStencilControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable);
		m_DepthStencilControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLess);
		//TODO: m_DepthStencilControl.setStencil(sce::Agc::CxDepthStencilControl::Stencil::kEnable);
		//TODO: m_DepthStencilControl.setStencilFunction(sce::Agc::CxDepthStencilControl::StencilFunction::kAlways);

		m_BlendAlphaControl.init();
		m_BlendAlphaControl.setBlend(sce::Agc::CxBlendControl::Blend::kEnable);
		m_BlendAlphaControl.setAlphaBlendFunc(sce::Agc::CxBlendControl::AlphaBlendFunc::kAdd);
		m_BlendAlphaControl.setAlphaDestMultiplier(sce::Agc::CxBlendControl::AlphaDestMultiplier::kOneMinusSrcAlpha);
		m_BlendAlphaControl.setAlphaSourceMultiplier(sce::Agc::CxBlendControl::AlphaSourceMultiplier::kOne);
		m_BlendAlphaControl.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd);
		m_BlendAlphaControl.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha);
		m_BlendAlphaControl.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha);
	}

	void AGCGraphicsContext::BindRenderTargetsToVideoOut()
	{
		// Next we need to inform scan-out about the format of our buffers. This can be done by directly talking to VideoOut or
		// by letting Agc::Core do the translation. To do so, we first need to get a RenderTargetSpec, which we can extract from
		// the list of CxRenderTargets passed into the function.
		sce::Agc::Core::RenderTargetSpec spec = {};
		SceError error = sce::Agc::Core::translate(&spec, &m_RenderTargets[0]);
		SCE_AGC_ASSERT(error == SCE_OK);

		// Next, we use this RenderTargetSpec to create a SceVideoOutBufferAttribute2 which tells VideoOut how it should interpret
		// our buffers. VideoOut needs to know how the color data in the target should be interpreted, and since our pixel shader has
		// been writing linear values into an sRGB RenderTarget, the data VideoOut will find in memory are sRGB encoded.
		SceVideoOutBufferAttribute2 attribute = {};
		error = sce::Agc::Core::translate(&attribute, &spec, sce::Agc::Core::Colorimetry::kSrgb, sce::Agc::Core::Colorimetry::kBt709);
		SCE_AGC_ASSERT(error == SCE_OK);

		// Ideally, all buffers should be registered with VideoOut in a single call to sceVideoOutRegisterBuffers2.
		// The reason for this is that the buffers provided in each call get associated with one attribute slot in the API.
		// Even if consecutive calls pass the same SceVideoOutBufferAttribute2 into the function, they still get assigned
		// new attribute slots. When processing a flip, there is significant extra cost associated with switching attribute
		// slots, which should be avoided.
		SceVideoOutBuffers* addresses = (SceVideoOutBuffers*)calloc(GRAPHICS_SETTINGS_NUMFRAMERESOURCES, sizeof(SceVideoOutBuffers));
		for (uint32_t i = 0; i < GRAPHICS_SETTINGS_NUMFRAMERESOURCES; ++i)
		{
			// We could manually call into VideoOut to set up the scan-out buffers, but Agc::Core provides a helper for this.
			addresses[i].data = m_RenderTargets[i].getDataAddress();
		}

		// VideoOut internally groups scan-out buffers in sets. Every buffer in a set has the same attributes and switching (flipping) between
		// buffers of the same set is a light-weight operation. Switching to a buffer from a different set is significantly more expensive
		// and should be avoided. If an application wants to change the attributes of a scan-out buffer or wants to unregister buffers,
		// these operations are done on whole sets and affect every buffer in the set. Here we only registers one set of buffers and never
		// modify the set.
		error = sceVideoOutRegisterBuffers2(
			m_VideoHandle,
			m_VideoSetIndex,
			0,
			addresses,
			GRAPHICS_SETTINGS_NUMFRAMERESOURCES,
			&attribute,
			SCE_VIDEO_OUT_BUFFER_ATTRIBUTE_CATEGORY_UNCOMPRESSED,
			nullptr
		);
		SCE_AGC_ASSERT(error == SCE_OK);
		free(addresses);

		// Define out to control the video synchronisation. This will be used in the StartDraw() in sceVideoOutLatencyControlWaitBeforeInput().
		// Essentially, we will wait until it is safe to process the frame based on these settings.
		m_LatencyControl.control = SCE_VIDEO_OUT_LATENCY_CONTROL_WAIT_BY_FLIP_QUEUE_NUM;	// This allows us to simply pass the frame number as the flipArg.
		m_LatencyControl.targetNum = 1;														// Allow for the CPU to get one frame ahead. This means the duration of CPU + GPU processing should be less than one frame to hold framerate. In your game application, you probably want this to be 2 or 3.
		m_LatencyControl.extraUsec = 0;
	}

	void AGCGraphicsContext::CreateViewPort()
	{
		// Set up a viewport using a helper function from Core.
		sce::Agc::Core::setViewport(&m_Viewport, static_cast<uint32_t>(m_ClientWidth), static_cast<uint32_t>(m_ClientHeight), 0u, 0u, -1.0f, 1.0f);
	}

	ShaderTable AGCGraphicsContext::BuildShaderTable_(const BufferRef& Target, off_t offset, ComputePipeline* pso,
		const RaytracingPipelineDesc& m_desc, ShaderTableGroup Group)
	{
		return ShaderTable();
	}

	RaytracingASSizeInfo AGCGraphicsContext::QueryAccelerationStructureSizeReq_(
		const AccelerationStructureDesc& AS_Desc, SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS Flags)
	{
		return RaytracingASSizeInfo();
	}

	RenderCommand* AGCGraphicsContext::GetAPI()
	{
		return &AGC_API;
	}

	ResourceFactory* AGCGraphicsContext::GetResourceFactory()
	{
		return &AGC_RESOURCE_FACTORY;
	}

	void AGCGraphicsContext::BeginFrame_()
	{
		// Grab the relevant data for the current frame. 
		const uint32_t frameIndex = gAGCContext->GetCurrentFrameResourceIndex();
		int32_t videoHandle = gAGCContext->GetVideoHandle();
		const SceVideoOutLatencyControl& latencyControl = gAGCContext->GetLatencyControl();
		sce::Agc::DrawCommandBuffer& dcb = gAGCContext->GetDrawCommandBuffer();
		sce::Agc::Core::StateBuffer& sb = gAGCContext->GetStateBuffer();
		sce::Agc::CxRenderTarget& rt = gAGCContext->GetRenderTarget();
		sce::Agc::CxDepthRenderTarget& dst = gAGCContext->GetDepthStencilTarget();
		const sce::Agc::CxRenderTargetMask& rtMask = gAGCContext->GetRenderTargetMask();
		const sce::Agc::CxDepthStencilControl& dsControl = gAGCContext->getDepthStencilControl();
		const auto& baControl = gAGCContext->getBlendControl();
		sce::Agc::Label& flipLabel = gAGCContext->GetFlipLabel();
		const sce::Agc::CxViewport& viewport = gAGCContext->GetViewport();

		// Check if the command buffer has been fully processed, if so it's safe for us to overwrite it on the CPU.
		while (flipLabel.m_value != 1)
			sceKernelUsleep(1000);

		// We can now set the flip label to 0, which the GPU will set back to 1 when it's done.
		flipLabel.m_value = 0;

		// Delay processing on the CPU to control latency. If this causes GPU stalls, you need to increase targetNum or decrease extraUsec in latencyControl.
		SceError error = sceVideoOutLatencyControlWaitBeforeInput(videoHandle, &latencyControl, nullptr);
		SCE_AGC_ASSERT(error == SCE_OK);
		// Notify VideoOut that we are about to begin processing the frame. This the start point of the latency computation.
		static int64_t monotonicallyIncrementedInteger = 0;
		error = sceVideoOutLatencyMeasureSetStartPoint(videoHandle, monotonicallyIncrementedInteger++);
		SCE_AGC_ASSERT(error == SCE_OK);


		// First we reset our components, since we're writing a completely new DCB.
		// This is actually quite wasteful, since we could reuse the previous data, but the
		// point of this code is to demonstrate a Gnm-like approach to writing DCBs.
		dcb.resetBuffer();
		sb.reset();

		// This will stall the Command Processor (CP) until the buffer is no longer being displayed.
		// Note that we're actually pulling the DCB out of the context and accessing it
		// directly here. This is very much how Agc's contexts work. They do not hide away the underlying
		// components but mostly just try to remove redundant work.
		dcb.waitUntilSafeForRendering(videoHandle, frameIndex);

		// Clear our current RenderTarget by using Agc::Toolkit.
		sce::Agc::Toolkit::Result toolkitResult1 = sce::Agc::Toolkit::clearRenderTargetCs(&dcb, &rt, sce::Agc::Core::Encoder::encode({ (double)m_ClearColour.x, (double)m_ClearColour.y, (double)m_ClearColour.z, (double)m_ClearColour.w }));
		SCE_AGC_ASSERT(toolkitResult1.m_errorCode == SCE_OK);

		// Clear our current DepthRenderTarget by using Agc::Toolkit.
		sce::Agc::Toolkit::Result toolkitResult2 = sce::Agc::Toolkit::clearDepthRenderTargetCs(&dcb, &dst);
		SCE_AGC_ASSERT(toolkitResult2.m_errorCode == SCE_OK);

		// Merge the two toolkit results together so that we can issue a single gpuSyncEvent for both toolkit
		// operations.
		toolkitResult1 = toolkitResult1 | toolkitResult2;

		// Wait for the clears to complete
		sce::Agc::Core::gpuSyncEvent(&dcb,
			toolkitResult1.getSyncWaitMode(),
			toolkitResult1.getSyncCacheOp(sce::Agc::Toolkit::Result::Caches::kGl2));

		// Set up the viewport and render targets
		sb.setState(rtMask);
		sb.setState(viewport);
		sb.setState(rt);
		sb.setState(dst);
		sb.setState(dsControl);
		sb.setState(baControl);
	}

	void AGCGraphicsContext::EndFrame_()
	{
		const uint32_t frameIndex = gAGCContext->GetCurrentFrameResourceIndex();
		int32_t videoHandle = gAGCContext->GetVideoHandle();
		sce::Agc::DrawCommandBuffer& dcb = gAGCContext->GetDrawCommandBuffer();
		sce::Agc::Label& flipLabel = gAGCContext->GetFlipLabel();

		// Submit a flip via the GPU.
		// Note: on PlayStation®5, RenderTargets write into the GL2 cache, but the scan-out
		// does not snoop any GPU caches. As such, it is necessary to flush these writes to memory before they can
		// be displayed. This flush is performed internally by setFlip() so we don't need to do it
		// on the application side.
		dcb.setFlip(videoHandle, frameIndex, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, 0);

		// The last thing we do in the command buffer is write 1 to the flip label to signal that command buffer
		// processing has finished. 
		//
		// While Agc provides access to the lowest level of GPU synchronization faculties, it also provides
		// functionality that builds the correct synchronization steps in an easier fashion.
		// Since synchonization should be relatively rare, spending a few CPU cycles on letting the library
		// work out what needs to be done is generally a good idea.
		sce::Agc::Core::gpuSyncEvent(
			&dcb,
			// The SyncWaitMode controls how the GPU's Command Processor (CP) handles the synchronization.
			// By setting this to kAsynchronous, we tell the CP that it doesn't have to wait for this operation
			// to finish before it can start the next frame. Instead, we could ask it to drain all graphics work
			// first, but that would be more aggressive than we need to be here.
			sce::Agc::Core::SyncWaitMode::kAsynchronous,
			// Since we are making the label write visible to the CPU, it is not necessary to flush any caches
			// and we set the cache op to 'kNone'.
			sce::Agc::Core::SyncCacheOp::kNone,
			// Write the flip label and make it visible to the CPU.
			sce::Agc::Core::SyncLabelVisibility::kCpu,
			&flipLabel,
			// We write the value "1" to the flip label.
			1);

		// Finally, we submit the work to the GPU. Since this is the only work on the GPU, we set its priority to normal.
		// The only reason to set the priority to kInterruptPriority is to make a submit expel work from the GPU we have previously
		// submitted. 
		SceError error = sce::Agc::submitGraphics(
			sce::Agc::GraphicsQueue::kNormal,
			dcb.getSubmitPointer(),
			dcb.getSubmitSize());
		SCE_AGC_ASSERT(error == SCE_OK);

		// If the application is suspended, it will happen during this call. As a side-effect, this is equivalent to
		// calling resetQueue(ResetQueueOp::kAllAccessible).
		error = sce::Agc::suspendPoint();
		SCE_AGC_ASSERT(error == SCE_OK);
	}

	void AGCGraphicsContext::Present_()
	{
		
	}

}