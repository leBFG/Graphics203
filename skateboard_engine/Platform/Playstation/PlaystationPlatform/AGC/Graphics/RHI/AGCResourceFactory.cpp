#include "AGCResourceFactory.h"

#include "AGC/Graphics/Resources/AGCBuffer.h"
#include "AGC/Graphics/Resources/AGCPipeline.h"
#include "AGC/Graphics/Resources/AGCCommandBuffer.h"
#include "AGC/Graphics/Resources/AGCView.h"
#include "Skateboard/Log.h"

namespace Skateboard
{
	BufferRef AGCResourceFactory::CreateBuffer_(const BufferDesc& desc, const wchar_t* name)
	{
		return std::static_pointer_cast<Buffer>(std::make_shared<AGCBuffer>(desc));
	}

	TextureBufferRef AGCResourceFactory::CreateTextureBuffer_(const TextureDesc& desc, const wchar_t* name)
	{
		return std::static_pointer_cast<TextureBuffer>(std::make_shared<AGCTextureBuffer>(desc));
	}

	BufferSRVRef AGCResourceFactory::CreateBufferShaderResourceView_(const BufferViewDesc& desc,
		BufferRef parent, const wchar_t* name)
	{
		return std::static_pointer_cast<BufferSRV>(std::make_shared<AGCShaderResourceBufferView>(desc, parent));
	}

	BufferUAVRef AGCResourceFactory::CreateBufferUnorderedAccessView_(const BufferViewDesc& desc,
		BufferRef parent, BufferRef counterResource, const wchar_t* name)
	{
		return std::static_pointer_cast<BufferUAV>(std::make_shared<AGCUnorderedAccessBufferView>(desc, parent));
	}

	BufferCBVRef AGCResourceFactory::CreateBufferConstantView_(const BufferViewDesc& desc,
		BufferRef parent, const wchar_t* name)
	{
		return std::static_pointer_cast<BufferCBV>(std::make_shared<AGCConstantBufferView>(desc,parent));
	}

	TextureSRVRef AGCResourceFactory::CreateTextureShaderResourceView_(const TextureViewDesc& desc,
		TextureBufferRef parent, const wchar_t* name)
	{
		return std::make_shared<AGCShaderResourceTextureView>(desc, parent);
	}

	TextureUAVRef AGCResourceFactory::CreateTextureUnorderedAccessView_(const TextureViewDesc& desc,
		TextureBufferRef parent, const wchar_t* name)
	{
		SKTBD_LOG_ASSERT(false, "ResourceFactoryAPI", "UNIMPLEMENTED");
		return std::make_shared<AGCUnorderedAccessTextureView>(desc, parent);
	}

	DepthStencilViewRef AGCResourceFactory::CreateDepthStencilView_(const DepthStencilDesc* desc,
		TextureBufferRef parent, const wchar_t* name)
	{
		SKTBD_LOG_ASSERT(false, "ResourceFactoryAPI", "UNIMPLEMENTED");
		return std::make_shared<AGCDepthStencilView>(desc, parent);
	}

	RenderTargetViewRef AGCResourceFactory::CreateRenderTargetView_(const RenderTargetDesc* desc,
		TextureBufferRef parent, const wchar_t* name)
	{
		SKTBD_LOG_ASSERT(false, "ResourceFactoryAPI", "UNIMPLEMENTED");
		return std::make_shared<AGCRenderTargetView>(desc, parent);
	}

	GraphicsCommandBufferRef AGCResourceFactory::CreateGraphicsCommandBuffer_(const CommandBufferPriority_& priority, const wchar_t* name)
	{
		return std::make_shared<AGCGraphicsCommandBuffer>(priority);
	}

	ComputeCommandBufferRef AGCResourceFactory::CreateComputeCommandBuffer_(const CommandBufferPriority_& priority, const wchar_t* name)
	{
		return std::make_shared<AGCComputeCommandBuffer>(priority);
	}

	ShaderInputLayoutRef AGCResourceFactory::CreateShaderInputLayout_(const ShaderInputLayoutDesc& desc, const wchar_t* name)
	{
		return std::make_shared<AGCShaderInputLayout>(desc);
	}

	ComputePipelineRef AGCResourceFactory::CreateComputePipeline_(const ComputePipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name)
	{
		return std::make_shared<AGCComputePipeline>(desc);
	}

	GraphicsPipelineRef AGCResourceFactory::CreateGraphicsPipeline_(const GraphicsPipelineDesc& desc,ShaderInputLayout* layout, const wchar_t* name)
	{
		return std::make_shared<AGCGraphicsPipeline>(desc);
	}

	ASViewRef AGCResourceFactory::CreateAccelerationStructureView_(
		const AccelerationStructure& StructureHandle, const wchar_t* name)
	{

		return ASViewRef();
	}

	ComputePipelineRef AGCResourceFactory::CreateRaytracingPipeline_(const RaytracingPipelineDesc& desc,
		ShaderInputLayout* layout, const wchar_t* name)
	{
		return ComputePipelineRef();
	}
}
