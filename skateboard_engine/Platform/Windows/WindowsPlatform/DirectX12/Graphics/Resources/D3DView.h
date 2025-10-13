#pragma once
#include "Graphics/RHI/D3DGraphicsContext.h"
#include "Graphics/Resources/D3DBuffer.h"
#include "Graphics/API/D3DDescriptorHeap.h"

#include "Skateboard/Graphics/Resources/View.h"

namespace Skateboard
{
	class D3DConstantBufferView final : public ConstantBufferView
	{
	public:
		D3DConstantBufferView(const BufferViewDesc& viewDesc, BufferRef Parent = BufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.GetIndex(); }

		~D3DConstantBufferView() override
		{
			gD3DContext->GetGPUSRVDescriptorHeap().Free(m_Descriptor);
		}
	
		D3DDescriptorHandle m_Descriptor;
	};

	class D3DShaderResourceBufferView final : public ShaderResourceBufferView
	{
	public:
		D3DShaderResourceBufferView(const BufferViewDesc& viewDesc, BufferRef Parent = BufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.GetIndex(); }

		~D3DShaderResourceBufferView() override
		{
			gD3DContext->GetGPUSRVDescriptorHeap().Free(m_Descriptor);
		}
	
		D3DDescriptorHandle m_Descriptor;
	};

	class D3DUnorderedAccessBufferView final : public UnorderedAccessBufferView
	{
	public:
		D3DUnorderedAccessBufferView(const BufferViewDesc& viewDesc, BufferRef parent = BufferRef(), BufferRef counterResource = BufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.GetIndex(); }

		~D3DUnorderedAccessBufferView() override
		{
			gD3DContext->GetGPUSRVDescriptorHeap().Free(m_Descriptor);
		}
	
		D3DDescriptorHandle m_Descriptor;
	};

	class D3DShaderResourceTextureView final : public ShaderResourceTextureView
	{
	public:
		D3DShaderResourceTextureView(const TextureViewDesc& viewDesc, TextureBufferRef parent = TextureBufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.GetIndex(); }
		ImTextureID GetImTextureID() override { return m_Descriptor.GetGPUPointer(); }

		~D3DShaderResourceTextureView() override
		{
			gD3DContext->GetGPUSRVDescriptorHeap().Free(m_Descriptor);
		}
	
		D3DDescriptorHandle m_Descriptor;
	};

	class D3DUnorderedAccessTextureView final : public UnorderedAccessTextureView
	{
	public:
		D3DUnorderedAccessTextureView(const TextureViewDesc& viewDesc, TextureBufferRef parent = TextureBufferRef());
		uint64_t GetViewIndex() override { return m_Descriptor.GetIndex(); }
		ImTextureID GetImTextureID() override { return m_Descriptor.GetGPUPointer(); }

		~D3DUnorderedAccessTextureView() override
		{
			gD3DContext->GetGPUSRVDescriptorHeap().Free(m_Descriptor);
		}
	
		D3DDescriptorHandle m_Descriptor;
	};

	class D3DRenderTargetView final : public RenderTargetView
	{
	public:
		D3DRenderTargetView(const RenderTargetDesc* viewDesc, TextureBufferRef parent = TextureBufferRef());
		D3DRenderTargetView(const D3DDescriptorHandle& Descriptor) : RenderTargetView(nullptr) { m_Descriptor = Descriptor; }
		uint64_t GetViewIndex() override { return m_Descriptor.GetIndex(); }

		~D3DRenderTargetView() override
		{
			gD3DContext->GetRTVDescriptorHeap().Free(m_Descriptor);
		}
	
		D3DDescriptorHandle m_Descriptor;
	};

	class D3DDepthStencilView final : public DepthStencilView
	{
	public:
		D3DDepthStencilView(const DepthStencilDesc* viewDesc, TextureBufferRef parent = TextureBufferRef());
		D3DDepthStencilView(const D3DDescriptorHandle& Descriptor) : DepthStencilView(nullptr) { m_Descriptor = Descriptor; }
		uint64_t GetViewIndex() override { return m_Descriptor.GetIndex(); }

		~D3DDepthStencilView() override
		{
			gD3DContext->GetDSVDescriptorHeap().Free(m_Descriptor);
		}
	
		D3DDescriptorHandle m_Descriptor;
	};

	class D3DAccelerationStructView final : public AccelerationStructureView
	{
	public:
		D3DAccelerationStructView(AccelerationStructureData AS_handle);

		uint64_t GetViewIndex() override { return m_Descriptor.GetIndex(); }

		~D3DAccelerationStructView() override
		{
			gD3DContext->GetGPUSRVDescriptorHeap().Free(m_Descriptor);
		}
	
		D3DDescriptorHandle m_Descriptor;
	};

}