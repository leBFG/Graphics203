#pragma once
#include "AGCRenderingAPI.h"
#include "AGCResourceFactory.h"
#include "AGC/Graphics/AGCF.h"
#include "AGC/Graphics/API/AGCDescriptorHeap.h"

#include "Skateboard/Graphics/RHI/GraphicsContext.h"
#include "Skateboard/Platform.h"

#include "AGC/Memory/AGCMemoryAllocator.h"

namespace Skateboard
{
	struct AGCGraphicsCommandBuffer;
	class AGCGraphicsCommandBuffer;

	class AGCGraphicsContext final : public GraphicsContext
	{
	public:
		AGCGraphicsContext(const PlatformProperties& props);
		virtual ~AGCGraphicsContext() final override;

		virtual void SetRenderTargetToBackBuffer_() final override {}

		virtual void Resize_(int clientWidth, int clientHeight) final override {}
		virtual void OnResized_() final override {}

		virtual void Flush_() final override {}
		virtual void WaitUntilIdle_() final override {}
		
		// Const getters
		int32_t GetVideoHandle() const { return m_VideoHandle; }
		const SceVideoOutLatencyControl& GetLatencyControl() const { return m_LatencyControl; }
		const sce::Agc::CxRenderTargetMask& GetRenderTargetMask() const { return m_RenderTargetMask; }
		const sce::Agc::CxDepthStencilControl& getDepthStencilControl() const { return m_DepthStencilControl; }
		const sce::Agc::CxBlendControl& getBlendControl() const { return m_BlendAlphaControl; }
		const sce::Agc::CxViewport& GetViewport() const { return m_Viewport; }

		// Non-const getters
		sce::Agc::DrawCommandBuffer& GetDrawCommandBuffer();
		sce::Agc::Core::StateBuffer& GetStateBuffer();
		sce::Agc::CxRenderTarget& GetRenderTarget() { return m_RenderTargets[m_CurrentFrameResourceIndex]; }
		sce::Agc::CxDepthRenderTarget& GetDepthStencilTarget() { return m_DepthStencilTarget; }
		sce::Agc::Label& GetFlipLabel() { return m_FlipLabels[m_CurrentFrameResourceIndex]; }

		TemplatedGPUMemoryPoolAllocator* GetMemAllocator() { return &m_MemoryAllocator; }
		//AGCMemoryPoolAllocator* GetMemoryPoolManager() { return &m_MemoryPool_Resources; }

		sce::Agc::Core::Buffer GetResourceDescriptorHeapBuffer() const	{ return m_DescriptorHeap.GetHeapBaseDescriptor(); }
		sce::Agc::Core::Buffer GeSamplerDescriptorHeapBuffer()	 const	{ return m_SamplerHeap.GetHeapBaseDescriptor(); }

		DescriptorHeap<sce::Agc::Core::ResourceDescriptor>& GetDescriptorHeap()  { return m_DescriptorHeap; }
		DescriptorHeap<sce::Agc::Core::Sampler>& GetSamplerHeap()				 { return m_SamplerHeap; }

	private:
		void InitialiseAGC();
		void InitVideoOutAndPollResolution();
		void CreateDrawCommandAndStateBuffers();
		void CreateFilpLabels();
		void CreateRenderTargets();
		void CreateDepthTarget();
		void BindRenderTargetsToVideoOut();
		void CreateViewPort();

	protected:
		bool IsRaytracingSupported_() override { return false; }
		bool AreWorkGraphsSupported_() override { return false;  }
		bool IsUnifiedMemoryArchitecture_() override { return true; }

		void Reset_() override { GetDrawCommandBuffer().resetBuffer(); }

		ShaderTable BuildShaderTable_(const BufferRef& Target, off_t offset, ComputePipeline* pso, const RaytracingPipelineDesc& m_desc, ShaderTableGroup Group) override;
		RaytracingASSizeInfo QueryAccelerationStructureSizeReq_(const AccelerationStructureDesc& AS_Desc, SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS Flags) override;

		RenderCommand* GetAPI() override;
		ResourceFactory* GetResourceFactory() override;
		void BeginFrame_() override;
		void EndFrame_() override;
		void Present_() override;

		//Data Shuffling
		CopyResult CopyDataToBuffer_(Buffer* dest, off_t offset, size_t size, void* src) override;
		CopyResult CopyDataToBuffer_(Buffer* dest, off_t offset, size_t size, std::function<void(void*)> WriterFunct) override;

		//Raytracing Data
		CopyResult WriteTopLevelASInstanceDataToBuffer_(Buffer* dest, off_t offset, size_t num, TLASInstanceData* data) override;



	private:
		// Video Out
		int32_t m_VideoHandle;
		const int32_t m_VideoSetIndex;
		SceVideoOutLatencyControl m_LatencyControl;

		// Command lists and state
		/*AGCGraphicsCommandBuffer m_DefaultCommandBuffers[GRAPHICS_SETTINGS_NUMFRAMERESOURCES];*/

		/*sce::Agc::DrawCommandBuffer m_DrawCommandBuffers[GRAPHICS_SETTINGS_NUMFRAMERESOURCES];
		sce::Agc::Core::StateBuffer m_StateBuffers[GRAPHICS_SETTINGS_NUMFRAMERESOURCES];*/

		// Render targets
		sce::Agc::CxRenderTarget m_RenderTargets[GRAPHICS_SETTINGS_NUMFRAMERESOURCES];	// Because the RT are flipped on the DCB we need one per frame resource
		sce::Agc::CxDepthRenderTarget m_DepthStencilTarget;
		sce::Agc::CxRenderTargetMask m_RenderTargetMask;
		sce::Agc::CxDepthStencilControl m_DepthStencilControl;
		sce::Agc::CxBlendControl m_BlendAlphaControl;


		// Flib labels for synchronisation (kinda like the swap chain i suppose)
		sce::Agc::Label* m_FlipLabels;

		// Viewport
		sce::Agc::CxViewport m_Viewport;

		// Context extentions
		AGCRenderingAPI AGC_API;
		AGCResourceFactory AGC_RESOURCE_FACTORY;

		DescriptorHeap<sce::Agc::Core::ResourceDescriptor>	 m_DescriptorHeap;
		DescriptorHeap<sce::Agc::Core::Sampler>			     m_SamplerHeap;

		TemplatedGPUMemoryPoolAllocator m_MemoryAllocator;

		MemoryHandle<uint8_t, sce::Agc::Alignment::kResourceRegistration> m_ResourceRegistrationMemory;

		typedef uint32_t CommandBufferMem; //struct to separate the command buffers into their own individual pool

		MemoryHandle<CommandBufferMem,sce::Agc::Alignment::kCommandBuffer> m_DrawCommandBuffersMemory[GRAPHICS_SETTINGS_NUMFRAMERESOURCES];
		MemoryHandle<uint8_t,sce::Agc::Alignment::kMaxTiledAlignment> m_RenderTargetsMemory[GRAPHICS_SETTINGS_NUMFRAMERESOURCES];
		MemoryHandle<uint8_t,sce::Agc::Alignment::kMaxTiledAlignment> m_DepthStencilTargetMemory;
		MemoryHandle<uint8_t,sce::Agc::Alignment::kMaxTiledAlignment> m_DepthStencilHTileMemory;


		MemoryHandle<sce::Agc::Label,sce::Agc::Alignment::kLabel> m_FlipLabelsMemory;
	};
}