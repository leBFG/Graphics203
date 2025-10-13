#pragma once
#include "Skateboard/Graphics/RHI/ResourceFactory.h"

namespace Skateboard
{
    class AGCResourceFactory :
        public ResourceFactory
    {
    public:
		constexpr AGCResourceFactory() { ResourceFactory::m_Factory = this; }
    protected:
	    BufferRef CreateBuffer_(const BufferDesc& desc, const wchar_t* name) override;
	    TextureBufferRef CreateTextureBuffer_(const TextureDesc& desc, const wchar_t* name) override;
	    BufferSRVRef CreateBufferShaderResourceView_(const BufferViewDesc& desc, BufferRef parent, const wchar_t* name) override;
	    BufferUAVRef CreateBufferUnorderedAccessView_(const BufferViewDesc& desc, BufferRef parent, BufferRef counterResource, const wchar_t* name) override;
	    BufferCBVRef CreateBufferConstantView_(const BufferViewDesc& desc, BufferRef parent, const wchar_t* name) override;
	    TextureSRVRef CreateTextureShaderResourceView_(const TextureViewDesc& desc, TextureBufferRef parent, const wchar_t* name) override;
	    TextureUAVRef CreateTextureUnorderedAccessView_(const TextureViewDesc& desc, TextureBufferRef parent, const wchar_t* name) override;
	    DepthStencilViewRef CreateDepthStencilView_(const DepthStencilDesc* desc, TextureBufferRef parent,const wchar_t* name) override;
	    RenderTargetViewRef CreateRenderTargetView_(const RenderTargetDesc* desc, TextureBufferRef parent,const wchar_t* name) override;

	    ComputeCommandBufferRef CreateComputeCommandBuffer_( const CommandBufferPriority_& priority, const wchar_t* name) override;
	    GraphicsCommandBufferRef CreateGraphicsCommandBuffer_( const CommandBufferPriority_& priority, const wchar_t* name) override;
	    ShaderInputLayoutRef CreateShaderInputLayout_(const ShaderInputLayoutDesc&, const wchar_t* name) override;
	    ComputePipelineRef CreateComputePipeline_(const ComputePipelineDesc& desc, ShaderInputLayout* Layout, const wchar_t* name) override;
	    GraphicsPipelineRef CreateGraphicsPipeline_(const GraphicsPipelineDesc& desc, ShaderInputLayout* Layout, const wchar_t* name) override;
	    ASViewRef CreateAccelerationStructureView_(const AccelerationStructure& StructureHandle, const wchar_t* name) override;
		ComputePipelineRef CreateRaytracingPipeline_(const RaytracingPipelineDesc& desc, ShaderInputLayout* layout,
			const wchar_t* name) override;
    };

}