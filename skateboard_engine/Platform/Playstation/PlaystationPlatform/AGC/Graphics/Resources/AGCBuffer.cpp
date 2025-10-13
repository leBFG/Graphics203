//#include <sktbdpch.h>
#include "AGCBuffer.h"
#include "AGC/Graphics/RHI/AGCGraphicsContext.h"

#define SKTBD_LOG_COMPONENT "AgcBuffer"
#include <agc/registerstructs.h>
#include <agc/registerstructs.h>

#include "Skateboard/Log.h"

namespace Skateboard
{
	auto SKTBDResourceAccessToAGC(ResourceAccessFlag_ Flags) -> uint
	{
		uint ret=0;
		if (Flags & ResourceAccessFlag_CpuRead)  ret |= SCE_KERNEL_PROT_CPU_READ;
		if (Flags & ResourceAccessFlag_GpuRead)  ret |= SCE_KERNEL_PROT_GPU_READ;
		if (Flags & ResourceAccessFlag_CpuWrite) ret |= SCE_KERNEL_PROT_CPU_WRITE;
		if (Flags & ResourceAccessFlag_GpuRead)  ret |= SCE_KERNEL_PROT_GPU_WRITE;
		return ret;
	}

	AGCTextureBuffer::AGCTextureBuffer(const TextureDesc& desc) : TextureBuffer(desc)
	{
		sce::Agc::SizeAlign ResourceSize;

		uint AccessFlags = SKTBDResourceAccessToAGC(desc.AccessFlags);

		switch(desc.Type)
		{
		case TextureType_Default:
				{
					sce::Agc::Core::TextureSpec Spec;
					Spec.init();
					Spec.m_width = desc.Width;
					Spec.m_height = desc.Height;
					Spec.m_depth = desc.Depth;
					Spec.m_format = SkateboardDataFormatToAGC(desc.Format);
					Spec.m_type = SkateboardTextureDimensionToAGC(desc.Dimension);
					Spec.m_numMips = desc.Mips;
					Spec.setIsWriteable(desc.AccessFlags & ResourceAccessFlag_GpuWrite);

					ResourceSize = sce::Agc::Core::getSize(&Spec);
					TextureMemory = gAGCContext->GetMemAllocator()->AllocateRaw(ResourceSize, SKTBDResourceAccessToAGC(desc.AccessFlags));

					Spec.setDataAddress(TextureMemory.m_Data);
					SCE_AGC_ASSERT(sce::Agc::Core::initialize(&TextureDescriptor, &Spec) == SCE_OK);
				}
			break;
		case TextureType_RenderTarget:
				{
					sce::Agc::Core::RenderTargetSpec rtSpec;
					rtSpec.init();
					rtSpec.m_width = desc.Width;
					rtSpec.m_height = desc.Height;
					rtSpec.m_depth = desc.Depth;
					rtSpec.m_numMips = desc.Mips;
					rtSpec.m_dimension = SkateboardTextureDimensionToAGCRenderTarget(desc.Dimension);
					rtSpec.m_format = SkateboardDataFormatToAGC(desc.Format);//{ sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
					rtSpec.m_tileMode = sce::Agc::CxRenderTarget::TileMode::kRenderTarget;

					ResourceSize = sce::Agc::Core::getSize(&rtSpec);
					TextureMemory = gAGCContext->GetMemAllocator()->AllocateRaw(ResourceSize, SKTBDResourceAccessToAGC(desc.AccessFlags));

					rtSpec.setDataAddress(TextureMemory.m_Data);
					SCE_AGC_ASSERT(sce::Agc::Core::initialize(&RenderTarget, &rtSpec) == SCE_OK);
				}
			break;
		case TextureType_DepthStencil:
				{
					sce::Agc::Core::DepthRenderTargetSpec DrtSpec;
					DrtSpec.init();
					DrtSpec.m_width = desc.Width;
					DrtSpec.m_height = desc.Height;
					DrtSpec.m_numMips = desc.Mips;

					if (desc.Format == DataFormat_DEFAULT_DEPTHSTENCIL)
					{
						DrtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k32Float;
					}
					else if (desc.Format == DataFormat_D32_FLOAT_S8X24_UINT)
					{
						DrtSpec.m_stencilFormat = sce::Agc::CxDepthRenderTarget::StencilFormat::k8UInt;
						DrtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k32Float;
					}
					else if (desc.Format == DataFormat_D16_UNORM)
					{
						DrtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k16UNorm;
					}
					else if (desc.Format == DataFormat_D24_UNORM_S8_UINT)
					{
						DrtSpec.m_stencilFormat = sce::Agc::CxDepthRenderTarget::StencilFormat::k8UInt;
						DrtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k16UNorm;
						SKTBD_MSG_TRACE("PS5 doesnt support 24 bit depth, only 16 Unorm and 32 Float")
					}

					
					ResourceSize = sce::Agc::Core::getSize(&DrtSpec);
					TextureMemory = gAGCContext->GetMemAllocator()->AllocateRaw(ResourceSize, SKTBDResourceAccessToAGC(desc.AccessFlags));

					DrtSpec.setDepthReadAddress(TextureMemory.m_Data).setDepthWriteAddress(TextureMemory.m_Data);
					SCE_AGC_ASSERT(sce::Agc::Core::initialize(&DepthRenderTarget, &DrtSpec) == SCE_OK);

				}
			break;
		}
	}

	AGCTextureBuffer::AGCTextureBuffer(const TextureDesc& desc, const RawMemoryHandle& mem ) : TextureBuffer(desc), TextureMemory(mem)
	{
		switch (desc.Type)
		{
		case TextureType_Default:
		{
			sce::Agc::Core::TextureSpec Spec;
			Spec.init();
			Spec.m_width = desc.Width;
			Spec.m_height = desc.Height;
			Spec.m_depth = desc.Depth;
			Spec.m_format = SkateboardDataFormatToAGC(desc.Format);
			Spec.m_type = SkateboardTextureDimensionToAGC(desc.Dimension);
			Spec.m_numMips = desc.Mips;
			Spec.setIsWriteable(desc.AccessFlags & ResourceAccessFlag_GpuWrite);
			Spec.setDataAddress(TextureMemory.m_Data);

			SCE_AGC_ASSERT(sce::Agc::Core::initialize(&TextureDescriptor, &Spec) == SCE_OK);
		}
		break;
		case TextureType_RenderTarget:
		{
			sce::Agc::Core::RenderTargetSpec rtSpec;
			rtSpec.init();
			rtSpec.m_width = desc.Width;
			rtSpec.m_height = desc.Height;
			rtSpec.m_depth = desc.Depth;
			rtSpec.m_numMips = desc.Mips;
			rtSpec.m_dimension = SkateboardTextureDimensionToAGCRenderTarget(desc.Dimension);
			rtSpec.m_format = SkateboardDataFormatToAGC(desc.Format);//{ sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
			rtSpec.m_tileMode = sce::Agc::CxRenderTarget::TileMode::kRenderTarget;
			rtSpec.setDataAddress(TextureMemory.m_Data);

			SCE_AGC_ASSERT(sce::Agc::Core::initialize(&RenderTarget, &rtSpec) == SCE_OK);
		}
		break;
		case TextureType_DepthStencil:
		{
			sce::Agc::Core::DepthRenderTargetSpec DrtSpec;
			DrtSpec.init();
			DrtSpec.m_width = desc.Width;
			DrtSpec.m_height = desc.Height;
			DrtSpec.m_numMips = desc.Mips;

			if (desc.Format == DataFormat_DEFAULT_DEPTHSTENCIL)
			{
				DrtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k32Float;
			}
			else if (desc.Format == DataFormat_D32_FLOAT_S8X24_UINT)
			{
				DrtSpec.m_stencilFormat = sce::Agc::CxDepthRenderTarget::StencilFormat::k8UInt;
				DrtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k32Float;
			}
			else if (desc.Format == DataFormat_D16_UNORM)
			{
				DrtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k16UNorm;
			}
			else if (desc.Format == DataFormat_D24_UNORM_S8_UINT)
			{
				DrtSpec.m_stencilFormat = sce::Agc::CxDepthRenderTarget::StencilFormat::k8UInt;
				DrtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k16UNorm;
				SKTBD_MSG_TRACE("PS5 doesnt support 24 bit depth, only 16 Unorm and 32 Float")
			}

			DrtSpec.setDepthReadAddress(TextureMemory.m_Data).setDepthWriteAddress(TextureMemory.m_Data);
			SCE_AGC_ASSERT(sce::Agc::Core::initialize(&DepthRenderTarget, &DrtSpec) == SCE_OK);
		}
		break;
		}
	}

	void AGCTextureBuffer::SetDebugName(const std::wstring& debug_name)
	{
		switch (Type)
		{
		case TextureType_Default:
			sce::Agc::Core::registerResource(&TextureDescriptor, ToString(debug_name).c_str());
			break;
		case TextureType_RenderTarget:
			sce::Agc::Core::registerResource(&RenderTarget, ToString(debug_name).c_str());
			break;
		case TextureType_DepthStencil:
			sce::Agc::Core::registerResource(&DepthRenderTarget, ToString(debug_name).c_str());
			break;
		}
		
	}

	void AGCBuffer::SetDebugName(const std::wstring& debug_name)
	{
		sce::Agc::Core::registerResource(&BufferDescriptor, ToString(debug_name).c_str());
	}

	AGCBuffer::AGCBuffer(const BufferDesc& desc) : Buffer(desc)
	{
		sce::Agc::Core::BufferSpec Spec;
		Spec.initAsByteBuffer(nullptr, desc.Size);

		auto sizealign = sce::Agc::Core::getSize(&Spec);
		BufferMemory = gAGCContext->GetMemAllocator()->AllocateRaw(sizealign, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);
		
		Spec.initAsByteBuffer(BufferMemory.m_Data, BufferMemory.m_SizeAlign.m_size);
		SCE_AGC_ASSERT(sce::Agc::Core::initialize(&BufferDescriptor, &Spec) == SCE_OK);
	}

	AGCBuffer::AGCBuffer(const BufferDesc& desc, const RawMemoryHandle& Mem) : Buffer(desc), BufferMemory(Mem)
	{
		sce::Agc::Core::BufferSpec Spec;
		Spec.initAsByteBuffer(Mem.m_Data, Mem.m_SizeAlign.m_size);
		Spec.m_dataAddress = BufferMemory.m_Data;

		SCE_AGC_ASSERT(sce::Agc::Core::initialize(&BufferDescriptor, &Spec) == SCE_OK);
	}
}


//#include "Assets/AGCAssetManager.h"
//

//
//namespace Skateboard
//{
//	AGCTexture::AGCTexture(const std::wstring& debugName, const TextureDesc& desc) :
//		Texture(debugName, desc)
//	{
//		//TODO implement for post prcess chain
//	}
//
//	AGCTexture::AGCTexture(const std::wstring& debugName, const TextureDesc& desc, sce::Agc::Core::Texture texture) :
//		Texture(debugName, desc),
//		m_Texture(texture)
//	{
//	}
//
//	AGCTexture::AGCTexture(const std::wstring& debugName, const TextureDesc& desc, sce::Agc::Core::Texture texture, MemoryHandle<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment> textureMemory) :
//		Texture(debugName, desc),
//		m_Texture(texture),
//		m_ResourceHandle(textureMemory)
//	{
//	}
//
//	AGCTexture::~AGCTexture()
//	{
//		//// TODO: Is this safe to do this i wonder... it is and you gotta
//		auto allocator = gAGCContext->GetMemAllocator();
//		allocator->TypedAlignedFree(m_ResourceHandle);
//	}
//
//	void* AGCTexture::GetTexture() const
//	{
//		return (void*)&m_Texture;
//	}
//
//	void AGCTexture::Release()
//	{
//		//auto allocator = gAGCContext->GetMemAllocator();
//		//allocator->Free(m_ResourceHandle);
//	}
//
//	AGCIndexBuffer::AGCIndexBuffer(const std::wstring& debugName, uint32_t* indices, uint32_t count) :
//		IndexBuffer(count),
//		m_IndexBuffer(gAGCContext->GetMemAllocator()->TypedAlignedAllocate<uint32_t, sce::Agc::Alignment::kBuffer>(count)) // Contrary to D3D, index buffers in AGC are expected as void*
//	{
//		memcpy(m_IndexBuffer.data, indices, count * sizeof(uint32_t));
//	}
//
//	AGCIndexBuffer::~AGCIndexBuffer()
//	{
//	//	gAGCContext->GetMemAllocator()->Free(m_IndexBuffer);
//	}
//
//	void AGCIndexBuffer::Bind() const
//	{
//		gAGCContext->GetDrawCommandBuffer().setIndexSize(sce::Agc::IndexSize::k32);
//		gAGCContext->GetDrawCommandBuffer().setIndexBuffer(m_IndexBuffer.data);
//
//		gAGCContext->GetDrawCommandBuffer();
//	}
//
//	void AGCIndexBuffer::Unbind() const
//	{
//	}
//
//	void AGCIndexBuffer::Release()
//	{
//		gAGCContext->GetMemAllocator()->TypedAlignedFree(m_IndexBuffer);
//	}
//
//	AGCVertexBuffer::AGCVertexBuffer(const std::wstring& debugName, void* vertices, uint32_t count, const BufferLayout& layout) :
//		VertexBuffer(layout)
//	{
//		const uint32_t size = count * layout.GetStride();
//
//		sce::Agc::Core::BufferSpec spec = {};
//		auto allocator = gAGCContext->GetMemAllocator();
//		m_ResourceHandle = allocator->TypedAlignedAllocate<uint32_t , sce::Agc::Alignment::kBuffer>(size);
//
//		memcpy(m_ResourceHandle.data, vertices, size);
//		spec.initAsVertexBuffer(m_ResourceHandle.data, sce::Agc::Gnmp::Extras::kDataFormatR32G32B32Float, layout.GetStride(), count);
//
//		SceError error = sce::Agc::Core::initialize(&m_Resource, &spec);
//		SCE_AGC_ASSERT(error == SCE_OK);
//
//#ifndef SKTBD_SHIP
//		char b[255] = { '\0' };
//		std::wcstombs(b, debugName.c_str(), debugName.size());
//		sce::Agc::Core::registerResource(&m_Resource, b);
//#endif // !SKTBD_SHIP
//	}
//
//	AGCVertexBuffer::~AGCVertexBuffer()
//	{
//	}
//
//	void AGCVertexBuffer::Bind() const
//	{
//	}
//
//	void AGCVertexBuffer::Unbind() const
//	{
//	}
//
//	void AGCVertexBuffer::Release()
//	{
//		gAGCContext->GetMemAllocator()->TypedAlignedFree(m_ResourceHandle);
//	}
//
//	AGCFrameBuffer::AGCFrameBuffer(const std::wstring& debugName, const FrameBufferDesc& desc) :
//		FrameBuffer(debugName, desc)
//	{
//		TemplatedGPUMemoryPoolAllocator* allocator = gAGCContext->GetMemAllocator();
//
//		// Initialise the render target
//		CreateRenderTarget(allocator);
//
//		// Initialise the depth render target
//		CreateDepthRenderTarget(allocator);
//
//		// Set up a viewport using a helper function from Core.
//		sce::Agc::Core::setViewport(&m_Viewport, desc.Width, desc.Height, 0u, 0u, -1.0f, 1000.0f);
//
//		// Enable writing to all the channels of the render target
//		m_RenderTargetMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xf);
//	}
//
//	AGCFrameBuffer::~AGCFrameBuffer()
//	{
//	}
//
//	void AGCFrameBuffer::Bind(uint32_t renderTargetIndex) const
//	{
//		sce::Agc::DrawCommandBuffer& dcb = gAGCContext->GetDrawCommandBuffer();
//		sce::Agc::Core::StateBuffer& stateBuffer = gAGCContext->GetStateBuffer();
//		sce::Agc::CxRenderTarget& rt = const_cast<sce::Agc::CxRenderTarget&>(v_RenderTargets[renderTargetIndex]);
//
//		// Clear our current RenderTarget by using Agc::Toolkit.
//		sce::Agc::Toolkit::Result toolkitResult1 = sce::Agc::Toolkit::clearRenderTargetCs(&dcb, &rt, sce::Agc::Core::Encoder::encode({ (double)m_Desc.RenderTarget.ClearColour.x, (double)m_Desc.RenderTarget.ClearColour.y, (double)m_Desc.RenderTarget.ClearColour.z, (double)m_Desc.RenderTarget.ClearColour.w }));
//		SCE_AGC_ASSERT(toolkitResult1.m_errorCode == SCE_OK);
//
//		// Clear our current DepthRenderTarget by using Agc::Toolkit.
//		sce::Agc::Toolkit::Result toolkitResult2 = sce::Agc::Toolkit::clearDepthRenderTargetCs(&dcb, &m_DepthStencilTarget);
//		SCE_AGC_ASSERT(toolkitResult2.m_errorCode == SCE_OK);
//
//		// Merge the two toolkit results together so that we can issue a single gpuSyncEvent for both toolkit
//		// operations.
//		toolkitResult1 = toolkitResult1 | toolkitResult2;
//
//		// Wait for the clears to complete
//		sce::Agc::Core::gpuSyncEvent(&dcb,
//			toolkitResult1.getSyncWaitMode(),
//			toolkitResult1.getSyncCacheOp(sce::Agc::Toolkit::Result::Caches::kGl2));
//
//		// Set the states
//		stateBuffer.setState(m_RenderTargetMask);					// bind the render target mask
//		stateBuffer.setState(m_Viewport);							// bind the viewport
//		stateBuffer.setState(v_RenderTargets[renderTargetIndex]);	// bind the chosen render target
//		stateBuffer.setState(m_DepthStencilTarget);					// bind depth stencil
//		stateBuffer.setState(m_DepthStencilControl);				// bind our depth stencil configuration
//	}
//
//	void AGCFrameBuffer::Unbind() const
//	{
//		// When unbinding a frame buffer, we must tell the GPU's command processor (CP) that it can continue onto the next workload - kAsynchronous.
//		// It is possible to use kDrainGraphics, which will force the CP to wait untill all work up until this point is complete, before processing 
//		// the next workload.
//		//sce::Agc::Core::gpuSyncEvent(&dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kNone);
//	}
//
//	void AGCFrameBuffer::Resize(uint32_t newWidth, uint32_t newHeight)
//	{
//		TemplatedGPUMemoryPoolAllocator* allocator = gAGCContext->GetMemAllocator();
//
//		// Register resize
//		m_Desc.Width = newWidth;
//		m_Desc.Height = newHeight;
//
//		// Release previous render targets
//		//Release();
//
//		// Create new render target
//		CreateRenderTarget(allocator);
//
//		// Create new depth stencil target
//		CreateDepthRenderTarget(allocator);
//	}
//
//	void* AGCFrameBuffer::GetRenderTargetAsImGuiTextureID(uint32_t renderTargetIndex) const
//	{
//		return static_cast<void*>(&v_RenderTargetAsTextures[renderTargetIndex]);
//	}
//
//	void* AGCFrameBuffer::GetDepthStencilTargetAsImGuiTextureID(uint32_t renderTargetIndex) const
//	{
//		return static_cast<void*>(&m_DepthStencilTargetAsTexture);
//	}
//
//	//void AGCFrameBuffer::Release()
//	//{
//	//	// Release Direct memory
//	//	TemplatedGPUMemoryPoolAllocator* allocator = gAGCContext->GetMemAllocator();
//	//	for (uint32_t i = 0u; i < m_Desc.NumRenderTargets; ++i)
//	//		allocator->TypedAlignedFree(v_RenderTargetResourceHandles[i]);
//	//	allocator->TypedAlignedFree(m_DepthResourceHandle);
//	//	allocator->TypedAlignedFree(m_DepthHTileHandle);
//	//	v_RenderTargetResourceHandles.clear();
//	//	v_RenderTargetAsTextures.clear();
//	//	v_RenderTargets.clear();
//	//}
//
//	/*const void* AGCFrameBuffer::GetDefaultDescriptor()
//	{
//		return v_RenderTargetAsTextures.data();
//	}*/
//
//	const void* AGCFrameBuffer::GetResourceMemory()
//	{
//
//	}
//
//	void AGCFrameBuffer::CreateRenderTarget(TemplatedGPUMemoryPoolAllocator* allocator)
//	{
//		sce::Agc::Core::RenderTargetSpec rtSpec;
//		rtSpec.init();
//		rtSpec.m_width = m_Desc.Width;
//		rtSpec.m_height = m_Desc.Height;
//		//rtSpec.m_format = { sce::Agc::Core::TypedFormat::k32_32_32_32Float, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
//		rtSpec.m_format = { sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
//		rtSpec.m_tileMode = sce::Agc::CxRenderTarget::TileMode::kRenderTarget;
//
//		sce::Agc::Core::BufferSpec bfspec;
//		sce::Agc::Core::TextureSpec TextureSpec;
//	
//
//		v_RenderTargets.resize(m_Desc.NumRenderTargets);
//
//		v_RenderTargetResourceHandles.resize(m_Desc.NumRenderTargets);
//	
//		sce::Agc::SizeAlign rtSize = sce::Agc::Core::getSize(&rtSpec);
//		v_RenderTargetResourceHandles[0] = allocator->TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment>(rtSize.m_size);
//		rtSpec.m_dataAddress = v_RenderTargetResourceHandles[0].data;
//
//		// Initialise our render target, triple buffered.
//		SceError err = sce::Agc::Core::initialize(&v_RenderTargets[0], &rtSpec);
//		SCE_AGC_ASSERT_MSG(err == SCE_OK, "Failed to initialise RenderTarget");
//
//		for (uint32_t i = 1u; i < m_Desc.NumRenderTargets; ++i)
//		{
//			// Copy specs into next render target
//			v_RenderTargets[i] = v_RenderTargets[0];
//
//			// Allocate more space for the next render target
//			v_RenderTargetResourceHandles[i] = allocator->TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment>(rtSize.m_size);
//			v_RenderTargets[i].setDataAddress(v_RenderTargetResourceHandles[i].data);
//
//#ifndef SKTBD_SHIP
//			char b[255] = { '\0' };
//			std::wcstombs(b, m_DebugName.c_str(), m_DebugName.size());
//			sce::Agc::Core::registerResource(&v_RenderTargets[i], b);
//#endif // !SKTBD_SHIP
//		}
//
//		v_RenderTargetAsTextures.resize(m_Desc.NumRenderTargets);
//		for (uint32_t i = 0u; i < m_Desc.NumRenderTargets; ++i)
//		{
//			v_RenderTargetAsTextures[i].init();
//			err = sce::Agc::Core::translate(&v_RenderTargetAsTextures[i], &v_RenderTargets[i], sce::Agc::Core::RenderTargetComponent::kData);
//			//error = sce::Agc::Core::translate(&m_RenderTargetToImGuiImage.texture, &v_RenderTargets[i], sce::Agc::Core::RenderTargetComponent::kData);
//		}
//	}
//
//	void AGCFrameBuffer::CreateDepthRenderTarget(TemplatedGPUMemoryPoolAllocator* allocator)
//	{
//		sce::Agc::Core::DepthRenderTargetSpec drtSpec = {};
//		drtSpec.init();																	// Always call init()
//		drtSpec.m_width = m_Desc.Width;
//		drtSpec.m_height = m_Desc.Height;
//		drtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k32Float;	// We want a 32-bit floating point depth buffer
//		//drtSpec.m_compression = sce::Agc::Core::MetadataCompression::kHtileDepthTextureCompatible;		// Compression for depth and stencil
//		drtSpec.m_compression = sce::Agc::Core::MetadataCompression::kNone;
//		//drtSpec.m_depthFormat = m_Desc.DepthFormat;
//		//drtSpec.m_compression = m_Desc.Compression;
//
//		// Create the depth stencil resource handles.
//		sce::Agc::SizeAlign dtSize = sce::Agc::Core::getSize(&drtSpec, sce::Agc::Core::DepthRenderTargetComponent::kDepth);
//		m_DepthResourceHandle = allocator->TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment>(dtSize.m_size);
//		drtSpec.m_depthReadAddress = drtSpec.m_depthWriteAddress = m_DepthResourceHandle.data;
//
//		// Retrieve the size of htile buffer for this depth render target and allocate the required memory backing.
//		// The HTILE buffer is optional. It contains a DWORD that serves as a summary of the depth buffer,
//		// which can be used to speed up depth clears or even sometimes avoid depth reads entirely!
//		//// Read: https://p.siedev.net/resources/documents/SDK/7.000/Agc-Reference/0795.html
//		//sce::Agc::SizeAlign htileSize = sce::Agc::Core::getSize(&drtSpec, sce::Agc::Core::DepthRenderTargetComponent::kHtile);
//		//m_DepthHTileHandle = allocator->TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment>(htileSize.m_size / sce::Agc::Alignment::kMaxTiledAlignment+1);
//		//drtSpec.m_htileAddress = m_DepthHTileHandle.data;
//
//		// We can now initialize the depth render target. This will check that the addresses are properly aligned
//		SceError error = sce::Agc::Core::initialize(&m_DepthStencilTarget, &drtSpec);
//		SCE_AGC_ASSERT_MSG(error == SCE_OK, "Failed to initialise Depth/Stencil Target");
//
//#ifndef SKTBD_SHIP
//		char b[255] = { '\0' };
//		std::wcstombs(b, m_DebugName.c_str(), m_DebugName.size());
//		sce::Agc::Core::registerResource(&m_DepthStencilTarget, b);
//#endif // !SKTBD_SHIP
//
//		// The depth/stencil render targets also own the clear value
//		m_DepthStencilTarget.setDepthClearValue(1.0f);
//
//		// Initialise and enable depth stencil
//		// Enable depth testing and writing, as well as stencil testing
//		m_DepthStencilControl.init();
//		m_DepthStencilControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kEnable);
//		m_DepthStencilControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable);
//		m_DepthStencilControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLess);
//
//		error = sce::Agc::Core::translate(&m_DepthStencilTargetAsTexture, &m_DepthStencilTarget, sce::Agc::Core::DepthRenderTargetComponent::kDepth);
//		SCE_AGC_ASSERT(error == SCE_OK);
//	}
//
//	AGCBottomLevelAccelerationStructure::AGCBottomLevelAccelerationStructure(const std::wstring& debugName, const BottomLevelAccelerationStructureDesc& desc) :
//		BottomLevelAccelerationStructure(debugName, desc)
//	{
//	}
//
//	AGCBottomLevelAccelerationStructure::~AGCBottomLevelAccelerationStructure()
//	{
//	}
//
//	//void AGCBottomLevelAccelerationStructure::Release()
//	//{
//	//	gAGCContext->GetMemAllocator()->TypedAlignedFree(m_ResourceHandle);
//	//}
//
//	AGCTopLevelAccelerationStructure::AGCTopLevelAccelerationStructure(const std::wstring& debugName, const TopLevelAccelerationStructureDesc& desc) :
//		TopLevelAccelerationStructure(debugName, desc)
//	{
//	}
//
//	AGCTopLevelAccelerationStructure::~AGCTopLevelAccelerationStructure()
//	{
//	}
//
//	void AGCTopLevelAccelerationStructure::PerformUpdate(uint32_t instanceID, const float4x4& transform)
//	{
//	}
//
//	//void AGCTopLevelAccelerationStructure::Release()
//	//{
//	//	gAGCContext->GetMemAllocator()->TypedAlignedFree(m_ResourceHandle);
//	//}
//
//	AGCIndirectArgumentBuffer::AGCIndirectArgumentBuffer(const std::wstring& debugName, const IndirectArgumentBufferDesc& desc)
//		:
//		IndirectArgumentBuffer(debugName, desc)
//	{
//
//		m_ArgumentBufferHandle = gAGCContext->GetMemAllocator()->TypedAlignedAllocate<sce::Agc::DrawIndexIndirectArgs, sce::Agc::Alignment::kIndirectArgs>(1);
//		m_IndexIndirectArgs = (sce::Agc::DrawIndexIndirectArgs*)m_ArgumentBufferHandle.data;
//
//		m_IndexIndirectArgs->m_baseVertexLocation		= desc.BaseVertexLocation;
//		m_IndexIndirectArgs->m_indexCountPerInstance	= desc.IndexCountPerInstance;
//		m_IndexIndirectArgs->m_instanceCount			= desc.InstanceCount;
//		m_IndexIndirectArgs->m_startIndexLocation		= desc.StartIndexLocation;
//		m_IndexIndirectArgs->m_startInstanceLocation    = desc.StartInstanceLocation;
//	}
//
//	void* AGCIndirectArgumentBuffer::GetIndirectArgumentBuffer()
//	{
//		return static_cast<void*>(m_IndexIndirectArgs);
//	}
//
//	//void AGCIndirectArgumentBuffer::Release()
//	//{
//	//	gAGCContext->GetMemAllocator()->TypedAlignedFree(m_ArgumentBufferHandle);
//	//	m_IndexIndirectArgs = nullptr;
//	//}
//
//}