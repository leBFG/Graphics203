#pragma once
#include "AGCPipeline.h"
#include "AGC/Graphics/AGCTypes.h"
#include "AGC/Memory/AGCMemoryAllocator.h"
#include "Skateboard\Graphics\Resources\CommandBuffer.h"

#define SKTBD_PS5_DCB_CHUNK_SIZE 128
#define SKTBD_DCB_SIZE 2*1024*1024

#define SKTBD_MAX_VERTEX_BUFFER_COUNT 32

namespace Skateboard
{
	struct IAGCCommandBufferInterface
	{
		virtual void SetCurrentPipeline(Pipeline* pipeline) = 0;
		virtual const Pipeline* GetCurrentPipeline() const = 0;
		virtual sce::Agc::CommandBuffer& GetCommandBuffer() = 0;
		virtual void SetShaderInputGraphics(ShaderInputLayout*) = 0;
		virtual void SetShaderInputCompute(ShaderInputLayout*) = 0;


		//maybe directly add a way to write signature data as we need to rebind it when its dirty, maybe store that in the signature?

		virtual AGCShaderInputLayout* GetGraphicsLayout() = 0;
		virtual AGCShaderInputLayout* GetComputeLayout() = 0;

		virtual const AGCShaderInputLayout* GetGraphicsLayout() const = 0;
		virtual const AGCShaderInputLayout* GetComputeLayout() const = 0;
	};

	class AGCComputeCommandBuffer final : public ComputeCommandBuffer , public IAGCCommandBufferInterface
	{
	public:
		AGCComputeCommandBuffer(CommandBufferPriority_ prior);

		void SetDebugName(const std::wstring& debug_name) override;
		~AGCComputeCommandBuffer() override;
		sce::Agc::CommandBuffer& GetCommandBuffer() override { return m_ACB; }
		void SetCurrentPipeline(Pipeline* pipeline) override;
		Pipeline* GetCurrentPipeline() const override { return m_CurrentPipeline; }
		void SetShaderInputGraphics(ShaderInputLayout*) override;
		void SetShaderInputCompute(ShaderInputLayout*) override;

		AGCShaderInputLayout* GetGraphicsLayout() override { return nullptr; };
		AGCShaderInputLayout* GetComputeLayout() override;

		AGCShaderInputLayout* GetGraphicsLayout() const override { return nullptr; };
		AGCShaderInputLayout* GetComputeLayout() const override;

		//Members

		MemoryHandle<uint32_t, sce::Agc::Alignment::kCommandBuffer> m_CommandBufferMemory;

		sce::Agc::AsyncCommandBuffer m_ACB;
		AGCShaderInputLayout* m_RootCompute;

		bool m_ComputeResourcesMustBeRebound;

		AGCComputePipeline* m_CurrentPipeline;
	};

	class AGCGraphicsCommandBuffer final : public GraphicsCommandBuffer , public IAGCCommandBufferInterface
	{
	public:
		AGCGraphicsCommandBuffer(CommandBufferPriority_ prior);

		void SetDebugName(const std::wstring& debug_name) override;
		~AGCGraphicsCommandBuffer() override;

		void SetShaderInputGraphics(ShaderInputLayout* root) override { m_RootGraphics = static_cast<AGCShaderInputLayout*>(root); }
		void SetShaderInputCompute(ShaderInputLayout* root) override{ m_RootCompute = static_cast<AGCShaderInputLayout*>(root); }

		AGCShaderInputLayout* GetGraphicsLayout() override { m_GraphicsResourcesMustBeRebound = true; return m_RootGraphics; }
		AGCShaderInputLayout* GetComputeLayout() override  { m_ComputeResourcesMustBeRebound = true;  return m_RootCompute; }

		AGCShaderInputLayout* GetGraphicsLayout() const override { return m_RootGraphics; }
		AGCShaderInputLayout* GetComputeLayout() const override  { return m_RootCompute;  }

		sce::Agc::CommandBuffer& GetCommandBuffer() override { return m_DCB; }
		void SetCurrentPipeline(Pipeline* pipeline) override;
		Pipeline* GetCurrentPipeline() const override { return m_CurrentPipeline; }



		//Members
		
		sce::Agc::DrawCommandBuffer	m_DCB;
		sce::Agc::Core::StateBuffer m_STB;

		//array of pointers to vertex buffers; this information is stored in the binder to the foremost stage, we make a copy here as we cant know if a pipeline is bound exists
		//sce::Agc::Core::Buffer m_VertexBuffers[SKTBD_MAX_VERTEX_BUFFER_COUNT];

		std::array<sce::Agc::Core::Buffer, SKTBD_MAX_VERTEX_BUFFER_COUNT> m_VertexBuffers;

		Pipeline* m_CurrentPipeline;

		MemoryHandle<uint32_t, sce::Agc::Alignment::kCommandBuffer> m_CommandBufferMemory;

		AGCShaderInputLayout* m_RootGraphics;
		AGCShaderInputLayout* m_RootCompute;

		bool m_ComputeResourcesMustBeRebound;
		bool m_GraphicsResourcesMustBeRebound;

		bool m_VertexBufferDataMustBeRebound;

		//Note Async / Compute doesnt need state buffer as it doesnt use Uc or Cx registers, only Sh, and also is not capable of indirect state registers
		//Async is split between 7 logical pipes and 2 hardware units
	};
}



