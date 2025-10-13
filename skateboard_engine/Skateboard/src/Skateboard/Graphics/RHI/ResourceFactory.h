#pragma once
#include "Skateboard/Graphics/Resources/CommonResources.h"

namespace Skateboard
{
	class ResourceFactory
	{
		friend class Application;
	public:
		virtual ~ResourceFactory() = default;
		static BufferRef CreateBuffer(const BufferDesc& desc, const wchar_t* name = L"") { return m_Factory->CreateBuffer_(desc, name); }
		static TextureBufferRef CreateTextureBuffer(const TextureDesc& desc, const wchar_t* name = L"") { return m_Factory->CreateTextureBuffer_(desc, name); }

		static  BufferSRVRef CreateBufferShaderResourceView(const BufferViewDesc& desc, BufferRef parent = BufferRef(), const wchar_t* name = L"") { return m_Factory->CreateBufferShaderResourceView_(desc, parent, name); }
		static  BufferUAVRef CreateBufferUnorderedAccessView(const BufferViewDesc& desc, BufferRef parent = BufferRef(), BufferRef counterResource = BufferRef(), const wchar_t* name = L"") { return m_Factory->CreateBufferUnorderedAccessView_(desc, parent, counterResource,name); }
		static  BufferCBVRef CreateBufferConstantView(const BufferViewDesc& desc, BufferRef parent = BufferRef(), const wchar_t* name = L"") { return m_Factory->CreateBufferConstantView_(desc, parent, name); }

		static  TextureSRVRef CreateTextureShaderResourceView(const TextureViewDesc& desc, TextureBufferRef parent = TextureBufferRef(), const wchar_t* name = L"") { return m_Factory->CreateTextureShaderResourceView_(desc, parent, name); }
		static  TextureUAVRef CreateTextureUnorderedAccessView(const TextureViewDesc& desc, TextureBufferRef parent = TextureBufferRef(), const wchar_t* name = L"") { return m_Factory->CreateTextureUnorderedAccessView_(desc, parent, name); }

		static  DepthStencilViewRef CreateDepthStencilView(const DepthStencilDesc* desc, TextureBufferRef parent = TextureBufferRef(), const wchar_t* name = L"") { return m_Factory->CreateDepthStencilView_(desc, parent, name); }
		static  RenderTargetViewRef CreateRenderTargetView(const RenderTargetDesc* desc, TextureBufferRef parent = TextureBufferRef(), const wchar_t* name = L"") { return m_Factory->CreateRenderTargetView_(desc, parent, name); }

		static  GraphicsCommandBufferRef CreateGraphicsCommandBuffer(const CommandBufferPriority_& priority, const wchar_t* name = L"") { return m_Factory->CreateGraphicsCommandBuffer_(priority, name); }
		static  ComputeCommandBufferRef  CreateComputeCommandBuffer(const CommandBufferPriority_& priority , const wchar_t* name = L"") { return m_Factory->CreateComputeCommandBuffer_(priority, name); }

		static FenceRef CreateFence(uint64_t intialValue, const wchar_t* name) { return m_Factory->CreateFence_(intialValue, name); }

		static  ShaderInputLayoutRef CreateShaderInputLayout(const ShaderInputLayoutDesc& desc, const wchar_t* name = L"") { return m_Factory->CreateShaderInputLayout_(desc, name); }
	
		static  ComputePipelineRef  CreateComputePipelineState(const ComputePipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name = L"") { return m_Factory->CreateComputePipeline_(desc, layout, name); }
		static  ComputePipelineRef  CreateRaytracingPipelineState(const RaytracingPipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name = L"") { return m_Factory->CreateRaytracingPipeline_(desc, layout, name); }

		static  GraphicsPipelineRef CreateGraphicsPipelineState(const GraphicsPipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name = L"") { return m_Factory->CreateGraphicsPipeline_(desc, layout, name); }

		static ASViewRef CreateAccelerationStructureView(const AccelerationStructure& StructureHandle, const wchar_t* name = L"") { return m_Factory->CreateAccelerationStructureView_(StructureHandle, name); }

		static SamplerRef CreateSampler(const SamplerDesc& desc, const wchar_t* name) { return m_Factory->CreateSampler_(desc, name); }

	protected:
		//resource creation
		virtual SamplerRef CreateSampler_(const SamplerDesc& desc, const wchar_t* name) = 0;
		virtual BufferRef CreateBuffer_(const BufferDesc& desc, const wchar_t* name) = 0;
		virtual TextureBufferRef CreateTextureBuffer_(const TextureDesc& desc, const wchar_t* name) = 0;

		virtual BufferSRVRef CreateBufferShaderResourceView_(const BufferViewDesc& desc, BufferRef parent = BufferRef(), const wchar_t* name = L"") = 0;
		virtual BufferUAVRef CreateBufferUnorderedAccessView_(const BufferViewDesc& desc, BufferRef parent = BufferRef(), BufferRef counterResource = BufferRef(), const wchar_t* name = L"") = 0;
		virtual BufferCBVRef CreateBufferConstantView_(const BufferViewDesc& desc, BufferRef parent = BufferRef(), const wchar_t* name = L"") = 0;

		virtual TextureSRVRef CreateTextureShaderResourceView_(const TextureViewDesc& desc, TextureBufferRef parent = TextureBufferRef(), const wchar_t* name = L"") = 0;
		virtual TextureUAVRef CreateTextureUnorderedAccessView_(const TextureViewDesc& desc, TextureBufferRef parent = TextureBufferRef(), const wchar_t* name = L"") = 0;

		virtual DepthStencilViewRef CreateDepthStencilView_(const DepthStencilDesc* desc, TextureBufferRef parent = TextureBufferRef(), const wchar_t* name = L"") = 0;
		virtual RenderTargetViewRef CreateRenderTargetView_(const RenderTargetDesc* desc, TextureBufferRef parent = TextureBufferRef(), const wchar_t* name = L"") = 0;

		virtual GraphicsCommandBufferRef CreateGraphicsCommandBuffer_(const CommandBufferPriority_& priority, const wchar_t* name) = 0;
		virtual ComputeCommandBufferRef CreateComputeCommandBuffer_(const CommandBufferPriority_& priority, const wchar_t* name) = 0;

		virtual FenceRef CreateFence_(uint64_t intialValue, const wchar_t* name) =0;

		virtual ShaderInputLayoutRef CreateShaderInputLayout_(const ShaderInputLayoutDesc&, const wchar_t* name) = 0;

		virtual ComputePipelineRef  CreateComputePipeline_(const ComputePipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name) = 0;

		virtual ComputePipelineRef  CreateRaytracingPipeline_(const RaytracingPipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name) = 0;

		virtual GraphicsPipelineRef CreateGraphicsPipeline_(const GraphicsPipelineDesc& desc, ShaderInputLayout* layout, const wchar_t* name) = 0;

		virtual ASViewRef CreateAccelerationStructureView_(const AccelerationStructure& StructureHandle, const wchar_t* name) = 0;

	protected:
		static void RegisterResourceFactory(ResourceFactory* PlatformFactory) { m_Factory = PlatformFactory; }
		static inline ResourceFactory* m_Factory = nullptr;
	};
};