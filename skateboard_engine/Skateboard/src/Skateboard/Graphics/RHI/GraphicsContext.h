#pragma once
#include "Skateboard/Mathematics.h"
#include "Skateboard/Graphics/Resources/CommonResources.h"
#include "Skateboard/Graphics/RHI/GraphicsSettingsDefines.h"
#include "sktbdpch.h"

namespace Skateboard
{
	using CopyResult = void*;
	using ShaderIdentifier = uint64_t;

	struct ShaderTableRecord
	{
		ShaderIdentifier m_identifier;
		uint64_t m_UserData;
	};

	class RenderCommand;
	class ResourceFactory;

	//Submit structure for the graphics and compute work
	struct GraphicsSubmitInfo
	{
		CommandBuffer** CommandBuffers;
		uint32_t BufferCount;
	};

	struct ComputeSubmitInfo
	{
		ComputeCommandBuffer** CommandBuffers;
		uint32_t BufferCount;
	};

	class GraphicsContext
	{
		friend class RenderCommand;
		friend class ResourceFactory;
		friend class Application;

	public:
		GraphicsContext(int32_t clientWidth, int32_t clientHeight) :
			m_ClientWidth(clientWidth),
			m_ClientHeight(clientHeight),
			m_CurrentFrameResourceIndex(0u)
		{
		}

		GraphicsContext() :
			m_ClientWidth(0),
			m_ClientHeight(0),
			m_CurrentFrameResourceIndex(0u)
		{
		}

		virtual ~GraphicsContext() {}

		/// <summary>
		/// Static Interface Of Skateboard Engine Graphics Context
		/// </summary> 

		static void SetRenderTargetToBackBuffer() { return Context->SetRenderTargetToBackBuffer_(); }
		static void Resize(int clientWidth, int clientHeight) { Context->Resize_(clientWidth, clientHeight);}

		static float GetClientAspectRatio()	{ return static_cast<float>(Context->m_ClientWidth) / Context->m_ClientHeight; }
		static void Reset() { Context->Reset_(); }
		static void Flush() { Context->Flush_(); }
		static void WaitUntilIdle() { Context->WaitUntilIdle_(); }

		static void Update() { Context->Update_(); }

		static void BeginFrame() { Context->BeginFrame_(); }
		static void EndFrame() { Context->EndFrame_(); }

		static bool IsRaytracingSupported() { return Context->IsRaytracingSupported_(); }
		static bool AreWorkGraphsSupported() { return Context->AreWorkGraphsSupported_(); }
		static bool IsUnifiedMemoryArchitecture() { return Context->IsUnifiedMemoryArchitecture_(); }

		static void SubmitGraphics(const GraphicsSubmitInfo& submit) { Context->SubmitGraphics_(submit);}
		static void SubmitCompute(const ComputeSubmitInfo& submit)   { Context->SubmitCompute_(submit); }

		static void GraphicsSignalFence(Fence* fence, uint64_t value){ Context->GraphicsSignalFence_(fence, value); }
		static void ComputeSignalFence(Fence* fence, uint64_t value) { Context->ComputeSignalFence_(fence, value); }

		static void GraphicsWaitFence(Fence* fence, uint64_t value)  { Context->GraphicsWaitFence_(fence, value); }
		static void ComputeWaitFence(Fence* fence, uint64_t value)   { Context->ComputeWaitFence_(fence, value); }

		static CopyResult CopyDataToBuffer(Buffer* dest, off_t offset, size_t size, void* src) { return Context->CopyDataToBuffer_(dest, offset, size, src); };
		static CopyResult CopyDataToBuffer(Buffer* dest, off_t offset, size_t size, std::function<void(void*)> WriterFunct) { return Context->CopyDataToBuffer_(dest, offset, size, WriterFunct); }

		//static void

		//Raytracing
		static CopyResult WriteTopLevelASInstanceDataToBuffer(Buffer* dest, off_t offset, size_t num, TLASInstanceData* data) { return Context->WriteTopLevelASInstanceDataToBuffer_(dest, offset, num, data); }
		static RaytracingASSizeInfo QueryAccelerationStructureSizeReq(const AccelerationStructureDesc& AS_Desc, SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS Flags) { return Context->QueryAccelerationStructureSizeReq_(AS_Desc, Flags); };
		static ShaderTable BuildShaderTable(const BufferRef& Target, off_t offset, ComputePipeline* pso, const RaytracingPipelineDesc& m_desc, ShaderTableGroup Group) { return Context->BuildShaderTable_(Target, offset, pso, m_desc, Group); };

		//Access to Variables

		static const int32_t GetClientWidth() { return Context->m_ClientWidth; }
		static const int32_t GetClientHeight()  { return Context->m_ClientHeight; }
		static void NextFrame() { Context->m_CurrentFrameResourceIndex = (Context->m_CurrentFrameResourceIndex + 1) % GRAPHICS_SETTINGS_NUMFRAMERESOURCES; ++Context->m_DefaultGraphicsCB; ++Context->m_SwapChainRTVs; }
		static uint64_t GetCurrentFrameResourceIndex() { return Context->m_CurrentFrameResourceIndex; }
		static GraphicsCommandBuffer* GetDefaultCommandBuffer() { return Context->m_DefaultGraphicsCB.Get().get(); }

		static RenderTargetView* GetBackBuffer() { return Context->m_SwapChainRTVs.Get().get(); }
		static DepthStencilView* GetDefaultDepthBuffer() { return Context->m_DefaultDSV.get(); }

		static void SetBackBufferClearColour(float4 nClearColour) { Context->m_ClearColour = nClearColour; }
		static void SetClearBackBuffer(bool ClearBackBuffer) { Context->m_bClearBackBuffer = ClearBackBuffer; }

		/// <summary>
		/// Virtual Interface of Skateboard Engine Graphics Context
		/// </summary>
	protected:

		virtual void SetRenderTargetToBackBuffer_() = 0;

		virtual void Resize_(int clientWidth, int clientHeight) {}
		//virtual void OnResized_() {}
		
		virtual void Reset_() {}
		virtual void Flush_() {}
		virtual void WaitUntilIdle_() {}

		virtual void Update_() {}

		virtual void BeginFrame_() {}
		virtual void EndFrame_() {}

		virtual bool IsRaytracingSupported_() = 0;
		virtual bool AreWorkGraphsSupported_() = 0;
		virtual bool IsUnifiedMemoryArchitecture_() = 0;

		//Work submission and synchronisation
		virtual void SubmitCompute_(const ComputeSubmitInfo& submit) = 0;
		virtual void SubmitGraphics_(const GraphicsSubmitInfo& submit) = 0;

		virtual void GraphicsSignalFence_(Fence* fence, uint64_t value) = 0;
		virtual void ComputeSignalFence_(Fence* fence, uint64_t value) = 0;

		virtual void GraphicsWaitFence_(Fence* fence, uint64_t value) = 0;
		virtual void ComputeWaitFence_(Fence* fence, uint64_t value) = 0;

		//Data Shuffling
		virtual CopyResult CopyDataToBuffer_(Buffer* dest, off_t offset, size_t size, void* src) = 0;
		virtual CopyResult CopyDataToBuffer_(Buffer* dest, off_t offset, size_t size, std::function<void(void*)> WriterFunct) = 0;

		//Raytracing Data
		virtual CopyResult WriteTopLevelASInstanceDataToBuffer_(Buffer* dest, off_t offset, size_t num, TLASInstanceData* data) = 0;

		//virtual ShaderID GetShaderIDFromPSO(Pipeline* RT_pipeline, const wchar_t* ShaderName);
		//virtual void WriteShaderRecordToShaderTable (ShaderTable* dest, ShaderRecord record);

		virtual ShaderIdentifier GetShaderIdentifier(const Pipeline* Pipeline, const std::wstring& ShaderName) = 0;

		virtual ShaderTable BuildShaderTable_(const BufferRef& Target, off_t offset, ComputePipeline* pso, const RaytracingPipelineDesc& m_desc, ShaderTableGroup Group) = 0;
		virtual RaytracingASSizeInfo QueryAccelerationStructureSizeReq_(const AccelerationStructureDesc& AS_Desc, SKTBD_ACCELERATION_STRUCT_BUILD_FLAGS Flags) = 0;

	protected:
		virtual RenderCommand* GetAPI() = 0;
		virtual ResourceFactory* GetResourceFactory() = 0;

	public:
		//context is accessible directly from this class but its 
		static GraphicsContext* Context;
	protected:

		int32_t m_ClientWidth, m_ClientHeight;		// Width and height of the client area (does not include the top bar and menus, this is the drawable surface)
		uint64_t m_CurrentFrameResourceIndex;

		float4 m_ClearColour = GRAPHICS_BACKBUFFER_DEFAULT_CLEAR_COLOUR;
		bool m_bClearBackBuffer = true;

		MultiResource<GraphicsCommandBufferRef> m_DefaultGraphicsCB;

		//Default Render Targets
		MultiResource<RenderTargetViewRef, GRAPHICS_SETTINGS_NUMFRAMERESOURCES>	m_SwapChainRTVs;			// only views can be accessed

		//Default DSV
		TextureBufferRef										m_DepthStencilBuffer;						// The depth/stencil buffer
		DepthStencilViewRef										m_DefaultDSV;
	};
}
