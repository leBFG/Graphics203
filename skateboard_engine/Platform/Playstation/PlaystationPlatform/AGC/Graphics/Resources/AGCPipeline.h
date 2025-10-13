#pragma once
#include <bitset>
#include <variant>
#include <vendor/model-loading/pack_file.h>

#include "AGC/Graphics/AGCF.h"
#include "Skateboard/Graphics/Resources/Pipeline.h"
#include "AGCPipeline.h"
#include "AGC/Graphics/RHI/AGCGraphicsContext.h"
#include "Skateboard/SizedPtr.h"
#include "ShaderResourceTable.h"

namespace Skateboard
{
	struct AGCGraphicsPipeline;
	struct AGCComputePipeline;

	#define MAX_Dual_Source_RTS 1
	#define MAX_Additional_RTS 7
	#define MAX_RTS 8

	// Cx registers which are always set by the PSO.
	struct AgcConstantCXRegisters
	{
		static const sce::Agc::RegisterType m_type = sce::Agc::RegisterType::kCxVerifiable;

		// Rasterizer State Registers
		sce::Agc::CxPrimitiveSetup PrimitiveSetup;
		sce::Agc::CxPolygonOffsetFront FrontOffset;
		sce::Agc::CxPolygonOffsetBack BackOffset;
		sce::Agc::CxConservativeRasterizationControl ConservativeRasterizationControl;

		// Depth Stencil State Registers
		sce::Agc::CxDepthStencilControl DepthStencilControl;
		sce::Agc::CxStencilOpControl StencilOpControl;

		// Blend State Registers
		sce::Agc::CxDualSourceBlendControl RT0;
		std::array<sce::Agc::CxBlendControl,MAX_Additional_RTS> RT17;
		sce::Agc::CxRenderTargetMask RenderTargetMask;
		sce::Agc::CxBlendColor BlendColor;

		// Shader Linkage Registers
		sce::Agc::CxShaderLinkage ShaderLinkage;

		// Used to set DX clip space (0 <= W <= 1) rather than default OGL (-1 <= W <= 1)
		sce::Agc::CxClipControl ClipControl;

		// Cb Mode
		sce::Agc::CxCbControl CbControl;

		void assertValid() const
		{
			PrimitiveSetup.assertValid();
			FrontOffset.assertValid();
			BackOffset.assertValid();
			ConservativeRasterizationControl.assertValid();
			DepthStencilControl.assertValid();
			StencilOpControl.assertValid();
			RT0.assertValid();
			for (int32_t Index = 0; Index < MAX_Additional_RTS; ++Index)
			{
				RT17.at(Index).assertValid();
			}
			RenderTargetMask.assertValid();
			BlendColor.assertValid();
			ShaderLinkage.assertValid();
			ClipControl.assertValid();
			CbControl.assertValid();
		}
	};

	// Optional Cx registers. Used when no pixel shader is bound.
	struct AgcOptionalCXRegisters
	{
		static const sce::Agc::RegisterType m_type = sce::Agc::RegisterType::kCxVerifiable;

		sce::Agc::CxDbShaderControl DbShaderControl;
		sce::Agc::CxShaderOutputMask ShaderOutputMask;

		void assertValid() const
		{
			DbShaderControl.assertValid();
			ShaderOutputMask.assertValid();
		}
	};

	struct AgcConstantUCRegisters
	{
		static const sce::Agc::RegisterType m_type = sce::Agc::RegisterType::kUc;
		sce::Agc::UcPrimitiveState PrimitiveState;
	};

	// Stencil state that needs to be kept separate, so we can adjust the stencil ref
	struct AgcCXRegistersStencilAndRef
	{
		static const sce::Agc::RegisterType m_type = sce::Agc::RegisterType::kCxVerifiable;

		sce::Agc::CxStencilControl StencilControl;
		sce::Agc::CxStencilControlBackFace BackStencilControl;

		void assertValid() const
		{
			StencilControl.assertValid();
			BackStencilControl.assertValid();
		}
	};

	struct AgcPipelineRegisterState
	{
		typedef uint64_t Register;
		static Skateboard::Allocator<Register, sce::Agc::Alignment::kRegister, SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_CPU_RW> RegisterAlloc;

		MemoryHandle<Register, sce::Agc::Alignment::kRegister > CxRegisters;
		MemoryHandle<Register, sce::Agc::Alignment::kRegister > UcRegisters;
		MemoryHandle<Register, sce::Agc::Alignment::kRegister > ShRegisters;

		sce::Agc::CxRegister* GetCxRegisters() const { return reinterpret_cast<sce::Agc::CxRegister*>(CxRegisters.data); }
		sce::Agc::UcRegister* GetUcRegisters() const { return reinterpret_cast<sce::Agc::UcRegister*>(UcRegisters.data); }
		sce::Agc::ShRegister* GetShRegisters() const { return reinterpret_cast<sce::Agc::ShRegister*>(ShRegisters.data); }

		size_t GetCxCount() const { return CxRegisters.count; }
		size_t GetUcCount() const { return UcRegisters.count; }
		size_t GetShCount() const { return ShRegisters.count; }

		void init(size_t CxCount, size_t UcCount, size_t ShCount)
		{
			CxRegisters = { RegisterAlloc.Allocate(CxCount),CxCount };
			UcRegisters = { RegisterAlloc.Allocate(UcCount),UcCount };
			ShRegisters = { RegisterAlloc.Allocate(ShCount),ShCount };
		}

		~AgcPipelineRegisterState()
		{
			RegisterAlloc.Free(CxRegisters.data, CxRegisters.count);
			RegisterAlloc.Free(UcRegisters.data, UcRegisters.count);
			RegisterAlloc.Free(ShRegisters.data, ShRegisters.count);
		}
	};

	struct AgcPipelineVertexAttributeState
	{
		static Skateboard::Allocator<sce::Agc::Core::VertexAttribute, sce::Agc::Alignment::kVertexAttribute, SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_CPU_RW> RegisterAlloc;
		MemoryHandle<sce::Agc::Core::VertexAttribute, sce::Agc::Alignment::kVertexAttribute > VertexAttributes;

		~AgcPipelineVertexAttributeState()
		{
			RegisterAlloc.Free(VertexAttributes.data, VertexAttributes.count);
		}

		sce::Agc::Core::VertexAttribute* GetVertexAttributeTable() const { return VertexAttributes.data; }

		size_t GetAttributeCount() const { return VertexAttributes.count; }

		void init(size_t AtribCount)
		{
			VertexAttributes = { RegisterAlloc.Allocate(AtribCount), AtribCount };
		}
	};

	enum AGCPipelineStates : uint32_t
	{
		PS,
		GS,
	//	HS,
		Depth,
		Blend,
			BlendRT0,
			BlendRT1,
			BlendRT2,
			BlendRT3,
			BlendRT4,
			BlendRT5,
			BlendRT6,
			BlendRT7,
		Primitive,
			
		SizeOfAGCPipelineStates
	};

	class AGCShaderInputLayout final : public ShaderInputLayout
	{
	public:
		AGCShaderInputLayout(const ShaderInputLayoutDesc& desc);

		//allocated on the dcb when shader input is bound or bindings change
		uint8_t UserDataSlotCount;
		uint8_t UserDataSizeInBytes;

		uint8_t SKTBDReservedSlotCount;
		uint8_t SKTBDReservedSizeInBytes;

		std::vector<uint8_t> SRT_DATA;
		std::vector<uint8_t> SRT_OFFSETS_IN_BYTES;

		//Metadata about the SRT;
		//ShaderInputLayoutDesc Description;
	};


	struct AGSBinary
	{
		MemoryHandle<uint8_t, sce::Agc::Alignment::kShaderHeader> Header;
		MemoryHandle<uint8_t, sce::Agc::Alignment::kShaderCode> Code;

		~AGSBinary()
		{
			gAGCContext->GetMemAllocator()->TypedAlignedFree(Header);
			gAGCContext->GetMemAllocator()->TypedAlignedFree(Code);
		}
	};

	struct AGCShader
	{
		AGCShader() : Binary({}), pShader(nullptr)
		{
		};

		AGSBinary Binary;
		sce::Agc::Shader* pShader;
	/*	ShaderResourceTable SRT;*/
	};

	void LoadShader(const wchar_t* filename, AGCShader& out_shader);
	void GetSrtSignature(const struct _SceShaderBinaryHandle* sl, AGCShader& out_shader);

	struct IAGCPipelineInterface
	{
		virtual ~IAGCPipelineInterface() = default;
		virtual const AgcPipelineRegisterState& GetRegisterState() const = 0;

	};

	struct AGCComputePipeline final : public ComputePipeline, public IAGCPipelineInterface
	{
		AGCComputePipeline(const ComputePipelineDesc& desc);

		AGCShader m_ComputeShader;
		AgcPipelineRegisterState m_State;
		const AgcPipelineRegisterState& GetRegisterState() const override { return m_State; }

		void SetDebugName(const std::wstring& debug_name) override;
	};

	struct AGCGraphicsPipeline final : public GraphicsPipeline , public IAGCPipelineInterface
	{
	public:
		AGCGraphicsPipeline(const GraphicsPipelineDesc& desc);

		void SetDebugName(const std::wstring& debug_name) override;

		const AgcPipelineRegisterState& GetRegisterState() const override { return m_RegisterStates; };

		AGCShader								m_VertexShader,
												m_PixelShader,
												m_GeometryShader;

		AgcPipelineRegisterState				m_RegisterStates;
		AgcPipelineVertexAttributeState			m_VATable;

		std::bitset<SizeOfAGCPipelineStates>    m_StatesToSet;
	};

	

	//class AGCComputePipeline final : public ComputePipeline
	//{
	//public:
	//	AGCComputePipeline(const std::wstring& debugName, const ComputePipelineDesc& desc);
	//	virtual ~AGCComputePipeline() final override;

	//	virtual void Bind() final override;

	//	sce::Agc::DispatchModifier GetDispatchModifier() const { return m_ComputeShader.pShader->m_specials->m_dispatchModifier; }

	//private:
	//	void CreateShaderResourceTable();
	//	void Release() final override;

	//private:
	//	AGCShader								m_ComputeShader;
	//	sce::Agc::Core::IndirectStageBinder		m_Binder;
	//};

	/*class AGCRaytracingPipeline final : public RaytracingPipeline
	{
	public:
		AGCRaytracingPipeline(const std::wstring& debugName, const RaytracingPipelineDesc& desc);
		virtual ~AGCRaytracingPipeline() final override;

		virtual void Bind() final override;
		void Release() final override;

		virtual void ResizeDispatchAndOutputUAV(uint32_t newWidth, uint32_t newHeight, uint32_t newDepth = 1u) final override;

	private:

	};
	*/
}