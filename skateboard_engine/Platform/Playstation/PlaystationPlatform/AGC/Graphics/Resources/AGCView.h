#pragma once
#include <psr/psr.h>

#include "ShaderResourceTable.h"
#include "AGC/Graphics/API/AGCDescriptorHeap.h"
#include "AGC/Graphics/RHI/AGCGraphicsContext.h"
#include "Skateboard/Graphics/Resources/View.h"

namespace Skateboard
{
	class AGCConstantBufferView final : public Skateboard::ConstantBufferView
	{
	public:
		AGCConstantBufferView(const BufferViewDesc& viewDesc, BufferRef Parent = BufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.m_Index; };

		~AGCConstantBufferView() override
		{
			gAGCContext->GetDescriptorHeap().Free(m_Descriptor);
		}

		DescriptorHandle<sce::Agc::Core::ResourceDescriptor> m_Descriptor;
	};


	class AGCShaderResourceBufferView final : public ShaderResourceBufferView
	{
	public:
		AGCShaderResourceBufferView(const BufferViewDesc& viewDesc, BufferRef Parent = BufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.m_Index; };

		~AGCShaderResourceBufferView() override
		{
			gAGCContext->GetDescriptorHeap().Free(m_Descriptor);
		}

		DescriptorHandle<sce::Agc::Core::ResourceDescriptor> m_Descriptor;
	};

	class AGCUnorderedAccessBufferView final : public UnorderedAccessBufferView
	{
	public:
		AGCUnorderedAccessBufferView(const BufferViewDesc& viewDesc, BufferRef parent = BufferRef(), BufferRef counterResource = BufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.m_Index; };

		~AGCUnorderedAccessBufferView() override
		{
			gAGCContext->GetDescriptorHeap().Free(m_Descriptor);
		}

		DescriptorHandle<sce::Agc::Core::ResourceDescriptor> m_Descriptor;
	};

	class AGCShaderResourceTextureView final : public ShaderResourceTextureView
	{
	public:
		AGCShaderResourceTextureView(const TextureViewDesc& viewDesc, TextureBufferRef parent = TextureBufferRef());
		AGCShaderResourceTextureView(const TextureViewDesc& viewDesc, DescriptorHandle<sce::Agc::Core::ResourceDescriptor> m_descriptor, TextureBufferRef parent = TextureBufferRef());


		uint64_t GetViewIndex() override { return m_Descriptor.m_Index; }
		ImTextureID GetImTextureID() override { return reinterpret_cast<unsigned long long>(m_Descriptor.m_DescriptorPtr); }

		~AGCShaderResourceTextureView() override
		{
			gAGCContext->GetDescriptorHeap().Free(m_Descriptor);
		}

		DescriptorHandle<sce::Agc::Core::ResourceDescriptor> m_Descriptor;
	};

	class AGCUnorderedAccessTextureView final : public UnorderedAccessTextureView
	{
	public:
		AGCUnorderedAccessTextureView(const TextureViewDesc& viewDesc, TextureBufferRef parent = TextureBufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.m_Index; }
		ImTextureID GetImTextureID() override { return reinterpret_cast<unsigned long long>(m_Descriptor.m_DescriptorPtr); }

		~AGCUnorderedAccessTextureView() override
		{
			gAGCContext->GetDescriptorHeap().Free(m_Descriptor);
		}

		DescriptorHandle<sce::Agc::Core::ResourceDescriptor> m_Descriptor;
	};

	class AGCRenderTargetView final : public RenderTargetView
	{
	public:
		AGCRenderTargetView(const RenderTargetDesc* viewDesc, TextureBufferRef parent = TextureBufferRef());
		AGCRenderTargetView(const AGCRenderTargetView& Descriptor) : RenderTargetView(nullptr) {  };
		uint64_t GetViewIndex() override {  SKTBD_LOG_INFO("AGCRenderTargetView","AGC doesnt support indexing Render Targets") return 0; };

		~AGCRenderTargetView() override
		{

		}

		sce::Agc::CxRenderTarget* m_Descriptor;
	};

	class AGCDepthStencilView final : public DepthStencilView
	{
	public:
		AGCDepthStencilView(const DepthStencilDesc* viewDesc, TextureBufferRef parent = TextureBufferRef());
		AGCDepthStencilView(const AGCDepthStencilView& Descriptor) : DepthStencilView(nullptr) { };
		uint64_t GetViewIndex() override { SKTBD_LOG_INFO("AGCRenderTargetView", "AGC doesnt support indexing Render Targets") return 0; };

		~AGCDepthStencilView() override
		{
			
		}

		sce::Agc::CxDepthRenderTarget* m_Descriptor;
	};

	class AGCAccelerationStructView final : public AccelerationStructureView
	{
	public:
		AGCAccelerationStructView(AccelerationStructureData AS_handle);

		uint64_t GetViewIndex() override { return 0; }

		~AGCAccelerationStructView() override
		{

			gAGCContext->GetDescriptorHeap().Free(m_Descriptor);
		}

		union
		{
			sce::Psr::TopLevelBvhDescriptor m_TLAS;
			sce::Psr::BottomLevelBvhDescriptor m_BLAS;
		};

		DescriptorHandle<sce::Agc::Core::ResourceDescriptor> m_Descriptor;
	};

}