#pragma once

#include "Skateboard/Graphics/RHI/ResourceFactory.h"

namespace Skateboard
{
	class D3DResourceFactory final : public  ResourceFactory
	{
		friend class D3DGraphicsContext;
		constexpr D3DResourceFactory() { ResourceFactory::m_Factory = this; }

	protected:
		virtual BufferRef CreateBuffer_(const BufferDesc& desc, const wchar_t* name) final override;
		virtual TextureBufferRef CreateTextureBuffer_(const TextureDesc& desc, const wchar_t* name) final override;

		virtual BufferSRVRef CreateBufferShaderResourceView_(const BufferViewDesc& desc, BufferRef parent, const wchar_t* name) override;
		virtual BufferUAVRef CreateBufferUnorderedAccessView_(const BufferViewDesc& desc, BufferRef parent, BufferRef counterResource, const wchar_t* name) override;
		virtual BufferCBVRef CreateBufferConstantView_(const BufferViewDesc& desc, BufferRef parent, const wchar_t* name) override;

		virtual TextureSRVRef CreateTextureShaderResourceView_(const TextureViewDesc& desc, TextureBufferRef parent, const wchar_t* name) override;
		virtual TextureUAVRef CreateTextureUnorderedAccessView_(const TextureViewDesc& desc, TextureBufferRef parent, const wchar_t* name) override;

		virtual DepthStencilViewRef CreateDepthStencilView_(const DepthStencilDesc* desc, TextureBufferRef parent,const wchar_t* name) override;
		virtual RenderTargetViewRef CreateRenderTargetView_(const RenderTargetDesc* desc, TextureBufferRef parent,const wchar_t* name) override;

		virtual GraphicsCommandBufferRef CreateGraphicsCommandBuffer_(const CommandBufferPriority_& priority, const wchar_t* name) final override;
		virtual ComputeCommandBufferRef  CreateComputeCommandBuffer_ (const CommandBufferPriority_& priority, const wchar_t* name) final override;

		virtual FenceRef CreateFence_(uint64_t initialValue, const wchar_t* name) final override;

		virtual ShaderInputLayoutRef CreateShaderInputLayout_(const ShaderInputLayoutDesc& desc, const wchar_t* name) final override;

		virtual ComputePipelineRef CreateComputePipeline_(const ComputePipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name) final override;
		virtual ComputePipelineRef CreateRaytracingPipeline_(const RaytracingPipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name) final override;

		virtual GraphicsPipelineRef CreateGraphicsPipeline_(const GraphicsPipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name) final override;

		virtual ASViewRef CreateAccelerationStructureView_(const AccelerationStructure& as, const wchar_t* name) override;

		virtual SamplerRef CreateSampler_(const SamplerDesc& desc, const wchar_t* name) override;
	};
}