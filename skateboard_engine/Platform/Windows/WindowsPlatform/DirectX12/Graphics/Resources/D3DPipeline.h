#pragma once
#include "Skateboard/Graphics/Resources/Pipeline.h"
#include "Graphics/D3D.h"
#include "Graphics/API/D3DDescriptorTable.h"

using namespace Microsoft::WRL;

namespace Skateboard
{
	class D3DShaderInputLayout final : public ShaderInputLayout
	{
	public:
		D3DShaderInputLayout(const ShaderInputLayoutDesc& desc);

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSig;

#ifndef SKTBD_SHIP
		void SetDebugName(const std::wstring& debug_name) override {
			GPUResource::SetDebugName(debug_name);
			m_RootSig->SetName(debug_name.data());
			
		};
#endif
	};

	struct ID3DPipelineInterface
	{
		virtual ID3D12StateObject* GetState() const = 0;
		virtual const D3D12_SET_PROGRAM_DESC* GetProgram() const = 0;
	};

	class D3DGraphicsPipeline final : public GraphicsPipeline, public ID3DPipelineInterface
	{
	public:
		D3DGraphicsPipeline(const GraphicsPipelineDesc& desc, const ShaderInputLayout* layout);
		ID3D12StateObject* GetState() const override { return m_State.Get(); }
		const D3D12_SET_PROGRAM_DESC* GetProgram() const override { return  &m_Program; };
	public:
#ifndef SKTBD_SHIP
		void SetDebugName(const std::wstring& debug_name) override {
			GPUResource::SetDebugName(debug_name);
			m_State->SetName(debug_name.data());
		}
#endif

		ComPtr<ID3D12StateObject> m_State;
		D3D12_SET_PROGRAM_DESC m_Program;
	};

	class D3DComputePipeline final : public ComputePipeline, public ID3DPipelineInterface
	{
	public:
		D3DComputePipeline(const ComputePipelineDesc& desc, const ShaderInputLayout* layout);
		ID3D12StateObject* GetState() const override { return m_State.Get(); }
		const D3D12_SET_PROGRAM_DESC* GetProgram() const override { return  &m_Program; };
	public:
#ifndef SKTBD_SHIP
		void SetDebugName(const std::wstring& debug_name) override {
			GPUResource::SetDebugName(debug_name);
			m_State->SetName(debug_name.data());

		};
#endif

		ComPtr<ID3D12StateObject> m_State;
		D3D12_SET_PROGRAM_DESC m_Program;
	};

	class D3DRaytracingPipeline final : public ComputePipeline, public ID3DPipelineInterface
	{
	public:
		D3DRaytracingPipeline(const RaytracingPipelineDesc& desc, ShaderInputLayout* layout);
		ID3D12StateObject* GetState() const override { return m_State.Get(); }
		const D3D12_SET_PROGRAM_DESC* GetProgram() const override { return  &m_Program; };
#ifndef SKTBD_SHIP
		void SetDebugName(const std::wstring& debug_name) override {
			GPUResource::SetDebugName(debug_name);
			m_State->SetName(debug_name.data());

		};
#endif

		ComPtr<ID3D12StateObject> m_State;
		D3D12_SET_PROGRAM_DESC m_Program;
	};

	class D3DSamplerState final : public SamplerState
	{
	public:
		D3DSamplerState(const SamplerDesc& desc);
		uint32_t GetSamplerIndex() override { return m_SamplerDescriptor.GetIndex(); };
		~D3DSamplerState() override;

		D3DDescriptorHandle m_SamplerDescriptor;
	};

}
