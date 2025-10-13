#include "sktbdpch.h"
#include "D3DResourceFactory.h"

#include "Graphics/Resources/D3DBuffer.h"
#include "Graphics/Resources/D3DView.h"
#include "Graphics/Resources/D3DPipeline.h"
#include "Graphics/Resources/D3DCommandBuffer.h"


namespace Skateboard
{
	BufferRef D3DResourceFactory::CreateBuffer_(const BufferDesc& desc, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<Buffer>(std::make_shared<D3DBuffer>(desc));
		resource->SetDebugName(name);
		return resource;
	}

	TextureBufferRef D3DResourceFactory::CreateTextureBuffer_(const TextureDesc& desc, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<TextureBuffer>(std::make_shared<D3DTextureBuffer>(desc));
		resource->SetDebugName(name);
		return resource;
	}

	BufferSRVRef D3DResourceFactory::CreateBufferShaderResourceView_(const BufferViewDesc& desc, BufferRef parent, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<ShaderResourceBufferView>(std::make_shared<D3DShaderResourceBufferView>(desc, parent));
		resource->SetDebugName(name);
		return resource;
	}

	BufferUAVRef D3DResourceFactory::CreateBufferUnorderedAccessView_(const BufferViewDesc& desc, BufferRef parent,
		BufferRef counterResource, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<UnorderedAccessBufferView>(std::make_shared<D3DUnorderedAccessBufferView>(desc, parent, counterResource));
		resource->SetDebugName(name);
		return resource;
	}

	BufferCBVRef D3DResourceFactory::CreateBufferConstantView_(const BufferViewDesc& desc, BufferRef parent, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<ConstantBufferView>(std::make_shared<D3DConstantBufferView>(desc, parent));
		resource->SetDebugName(name);
		return resource;
	}

	TextureSRVRef D3DResourceFactory::CreateTextureShaderResourceView_(const TextureViewDesc& desc,
		TextureBufferRef parent, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<ShaderResourceTextureView>(std::make_shared<D3DShaderResourceTextureView>(desc, parent));
		resource->SetDebugName(name);
		return resource;
	}

	TextureUAVRef D3DResourceFactory::CreateTextureUnorderedAccessView_(const TextureViewDesc& desc,
		TextureBufferRef parent, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<UnorderedAccessTextureView>(std::make_shared<D3DUnorderedAccessTextureView>(desc, parent));
		resource->SetDebugName(name);
		return resource;
	}

	DepthStencilViewRef D3DResourceFactory::CreateDepthStencilView_(const DepthStencilDesc* desc,
		TextureBufferRef parent, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<DepthStencilView>(std::make_shared<D3DDepthStencilView>(desc, parent));
		resource->SetDebugName(name);
		return resource;
	}

	RenderTargetViewRef D3DResourceFactory::CreateRenderTargetView_(const RenderTargetDesc* desc,
		TextureBufferRef parent, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<RenderTargetView>(std::make_shared<D3DRenderTargetView>(desc, parent));
		resource->SetDebugName(name);
		return resource;
	}

	GraphicsCommandBufferRef D3DResourceFactory::CreateGraphicsCommandBuffer_(const CommandBufferPriority_& priority, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<GraphicsCommandBuffer>(std::make_shared<D3DGraphicsCommandBuffer>(priority));
		resource->SetDebugName(name);
		return resource;
	}

	ComputeCommandBufferRef D3DResourceFactory::CreateComputeCommandBuffer_(const CommandBufferPriority_& priority, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<ComputeCommandBuffer>(std::make_shared<D3DComputeCommandBuffer>(priority));
		resource->SetDebugName(name);
		return resource;
	}

	FenceRef D3DResourceFactory::CreateFence_(uint64_t initialValue, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<Fence>(std::make_shared<D3DFence>(initialValue));
		resource->SetDebugName(name);
		return resource;
	}

	ComputePipelineRef D3DResourceFactory::CreateComputePipeline_(const ComputePipelineDesc& desc,
	                                                              ShaderInputLayout* layout, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<ComputePipeline>(std::make_shared<D3DComputePipeline>(desc, layout));
		resource->SetDebugName(name);
		return resource;
	}

	GraphicsPipelineRef D3DResourceFactory::CreateGraphicsPipeline_(const GraphicsPipelineDesc& desc,
		ShaderInputLayout* layout, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<GraphicsPipeline>(std::make_shared<D3DGraphicsPipeline>(desc, layout));
		resource->SetDebugName(name);
		return resource;
	}


	ShaderInputLayoutRef D3DResourceFactory::CreateShaderInputLayout_(const ShaderInputLayoutDesc& desc, const wchar_t* name)
	{
		auto resource = std::static_pointer_cast<ShaderInputLayout>(std::make_shared<D3DShaderInputLayout>(desc));
		resource->SetDebugName(name);
		return resource;
	}


	ASViewRef D3DResourceFactory::CreateAccelerationStructureView_(
		const AccelerationStructure& StructureHandle, const wchar_t* name)
	{
		SKTBD_LOG_ASSERT(StructureHandle.m_Type == TopLevel, "D3D only supports views for top level acceleration structures, put your BLAS into single instance TLAS")

		auto resource = std::static_pointer_cast<AccelerationStructureView>(std::make_shared<D3DAccelerationStructView>(std::get<TopLevelAccelerationStructure>(StructureHandle.m_Handle).GetData()));
		resource->SetDebugName(name);
		return resource;
	}

	SamplerRef D3DResourceFactory::CreateSampler_(const SamplerDesc& desc, const wchar_t* name)
	{
		auto resource = std::make_shared<D3DSamplerState>(desc);
		resource->SetDebugName(name);
		return resource;
	}

	ComputePipelineRef D3DResourceFactory::CreateRaytracingPipeline_(const RaytracingPipelineDesc& desc,
	                                                                 ShaderInputLayout* layout, const wchar_t* name)
	{
		auto resource = std::make_shared<D3DRaytracingPipeline>(desc, layout);
		resource->SetDebugName(name);
		return resource;
	}
}
