#pragma once
#include "Skateboard/Mathematics.h"
#include "Buffer.h"
#include "Skateboard/Graphics/InternalFormats.h"
#include "Skateboard/Memory/DescriptorTable.h"

#define PIPELINE_SETTINGS_DEFAULT_DEPTH_BIAS 0

namespace Skateboard
{
	class Pipeline;
	class ComputePipeline;
	class GraphicsPipeline;
	class ShaderInputLayout;

	class SamplerState;

	typedef std::shared_ptr<ShaderInputLayout> ShaderInputLayoutRef;

	typedef std::shared_ptr<ComputePipeline> ComputePipelineRef;
	typedef std::shared_ptr<GraphicsPipeline> GraphicsPipelineRef;

	typedef std::shared_ptr<SamplerState> SamplerRef;

	enum PipelineType_
	{
		PipelineType_Graphics,
		PipelineType_Compute,
	};

	enum FillMode_ : uint8_t
	{
		FillMode_Fill		= 0b00000000,
		FillMode_Wireframe	= 0b00000001,
		FillMode_Point		= 0b00000010
	};

	enum WindingOrder_ : uint8_t
	{
		WindingOrder_CCW	= 0b00000100,
		WindingOrder_CW		= 0b00001000,
	};

	enum CullMode_ : uint8_t
	{
		CullMode_Back		= 0b00010000,
		CullMode_Front		= 0b00100000,
	};

	struct Viewport
	{
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinZ;
		float MaxZ;
	};

	struct Rect
	{
		long left;
		long top;
		long right;
		long bottom;
	};

	struct ShaderModuleDesc
	{
		const wchar_t* FileName;
		const wchar_t* EntryPoint;
	};

	struct RaytracingHitGroup
	{
		const wchar_t* HitGroupName;
		const wchar_t* AnyHitShaderEntryPoint;
		const wchar_t* ClosestHitShaderEntryPoint;
		const wchar_t* IntersectionShaderEntryPoint;
		RaytracingHitGroupType_ Type;
	};

	struct RaytracingShaderLibrary
	{
		const wchar_t* FileName;
		const wchar_t* RayGenShaderEntryPoint;
		std::vector<RaytracingHitGroup> HitGroups;
		std::vector<const wchar_t*> MissShaderEntryPoints;
		std::vector<const wchar_t*> CallableShaders;
	};

	struct DescriptorRange
	{
		ShaderElementType_ DescriptorType;
		uint32_t NumOfDescriptors;
		uint32_t BaseRegister;
		uint32_t RegisterSpace;
		uint32_t OffsetInDescriptorsFromTableStart;
	};

	struct DescriptorTableLayout
	{
		uint32_t NumberOfRanges;
		DescriptorRange* Ranges;

		static DescriptorRange CreateDescriptorInfo(ShaderElementType_ Type, uint32_t BaseRegister, uint32_t NumOfDesc = 1, uint32_t RegisterSpace = 0, uint32_t OffsetInDescriptorsFromTableStart = 0xffffffff) { return { Type,NumOfDesc,BaseRegister,RegisterSpace, OffsetInDescriptorsFromTableStart }; };
	};

	struct ShaderResourceDesc
	{
		ShaderElementType_ ShaderElementType;
		ShaderVisibility_ ShaderVisibility;
		union
		{
			struct {
				uint32_t ShaderRegister;
				uint32_t RegisterSpace;
			} Descriptor;
			struct {
				uint32_t ShaderRegister;
				uint32_t RegisterSpace;
				uint32_t Num32BitValues;
			} Constant ;
			DescriptorTableLayout DescriptorTable;
		};
	};

	enum SamplerFlags_
	{
		SamplerFlags_NONE = 0,
		SamplerFlags_INTEGER_SAMPLING = 0x1,
		SamplerFlags_NON_NORMALIZED_COORDINATES = 0x2
	};
	ENUM_FLAG_OPERATORS(SamplerFlags_);

	struct SamplerDesc
	{
		SamplerFilter_ Filter;
		SamplerMode_ ModeU;
		SamplerMode_ ModeV;
		SamplerMode_ ModeW;
		float MipMapLevelOffset;
		float MipMapMinSampleLevel;
		float MipMapMaxSampleLevel;
		uint32_t MaxAnisotropy;	// Valid range 1 - 16 -> uint32_t cause padding anyways
		SamplerComparisonFunction_ ComparisonFunction;
		SamplerBorderColour_ BorderColour;
		SamplerFlags_ Flags;

		static SamplerDesc InitAsDefaultTextureSampler();
		static SamplerDesc InitAsDefaultShadowSampler();
	};

	struct SamplerSlotDesc
	{
		SamplerDesc SamplerDesc;
		uint32_t ShaderRegister;
		uint32_t ShaderRegisterSpace;
		ShaderVisibility_ ShaderVisibility;
	};

	//max root signature size on DX12 is 64 Dwords max root signature (BUT ON SOME HARDWARE ITS 16 ACTUALLY (PROBABLY SMILES IN AMD)) on PS5 is 32 Dwords Graphics 16 - Compute
	struct ShaderInputLayoutDesc
	{
		//only these for now // all of these are buffers as DX12 forbids formatted data in root signature;
		void AddRootConstant(uint32_t num32bitvalues, uint32_t shaderRegister, uint32_t shaderRegisterSpace = 0u, ShaderVisibility_ shaderVisibility = ShaderVisibility_All);
		void AddConstantBufferView(uint32_t shaderRegister, uint32_t shaderRegisterSpace = 0u, ShaderVisibility_ shaderVisibility = ShaderVisibility_All);
		void AddShaderResourceView(uint32_t shaderRegister, uint32_t shaderRegisterSpace = 0u, ShaderVisibility_ shaderVisibility = ShaderVisibility_All);
		void AddUnorderedAccessView(uint32_t shaderRegister, uint32_t shaderRegisterSpace = 0u, ShaderVisibility_ shaderVisibility = ShaderVisibility_All);
		void AddStaticSampler(const SamplerDesc& desc, uint32_t ShaderRegister, uint32_t ShaderRegisterSpace =0, ShaderVisibility_ ShaderVisibility = ShaderVisibility_PixelShader);
		void AddDescriptorTable(DescriptorTableLayout DescTable, ShaderVisibility_ shaderVisibility = ShaderVisibility_All);

		bool DescriptorsDirctlyAddresssed;
		bool SamplersDirectlyAddressed;
		bool CanUseInputAssembler;

		std::vector<ShaderResourceDesc> vPipelineInputs;
		std::vector<SamplerSlotDesc> vStaticSamplers; 
		// We'll only use static samplers for now, TODO: support non-static samplers if necessary
		//std::vector<SamplerDesc> vSamplers; 
	};

	class ShaderInputLayout : public GPUResource
	{
	protected:
		ShaderInputLayout(const ShaderInputLayoutDesc& desc) {};
	public:
		GPUResourceType_ GetResourceType() override { return GpuResourceType_ShaderInputsLayout; }
	};

	//Shamelesly stolen from D3D and extended with ps5 options
	//FIXME: add annotations to options
	enum CullMode
	{
		Cull_NONE = 1,
		Cull_FRONT = 2,
		Cull_BACK = 3,
	};

	enum ConservativeRasterMode
	{
		Underestimate,
		Overestimate
	};

	struct RasterizerConfig
	{
		bool Wireframe;
		CullMode Cull;

		bool FrontCC;
		int DepthBias;
		float DepthBiasClamp;
		float SlopeScaledDepthBias;
		bool DepthClipEnable;
		bool MultisampleEnable;
		uint32_t ForcedSampleCount;
		bool ConservativeRasterEnable;
		bool AntialiasedLineEnable;

		ConservativeRasterMode ConservativeRasterization;

		static RasterizerConfig Default()
		{
			return
			{
			.Wireframe = false,
			.Cull = Cull_BACK,
			.FrontCC = true,
			.DepthBias = 0,
			.DepthBiasClamp = 0.f,
			.SlopeScaledDepthBias = 0.f,
			.DepthClipEnable = false,
			.MultisampleEnable = false,
			.ForcedSampleCount = 0,
			.ConservativeRasterEnable = false,
			.AntialiasedLineEnable = false,
			//config.ConservativeRasterization; not set
			};
		}
	};

	enum SKTBD_Blend
	{
		SKTBD_Blend_ZERO = 1,
		SKTBD_Blend_ONE = 2,
		SKTBD_Blend_SRC_COLOR = 3,
		SKTBD_Blend_INV_SRC_COLOR = 4,
		SKTBD_Blend_SRC_ALPHA = 5,
		SKTBD_Blend_INV_SRC_ALPHA = 6,
		SKTBD_Blend_DEST_ALPHA = 7,
		SKTBD_Blend_INV_DEST_ALPHA = 8,
		SKTBD_Blend_DEST_COLOR = 9,
		SKTBD_Blend_INV_DEST_COLOR = 10,
		SKTBD_Blend_SRC_ALPHA_SAT = 11,
		SKTBD_Blend_BLEND_FACTOR = 14,
		SKTBD_Blend_INV_BLEND_FACTOR = 15,
		SKTBD_Blend_SRC1_COLOR = 16,
		SKTBD_Blend_INV_SRC1_COLOR = 17,
		SKTBD_Blend_SRC1_ALPHA = 18,
		SKTBD_Blend_INV_SRC1_ALPHA = 19,
		SKTBD_Blend_ALPHA_FACTOR = 20,
		SKTBD_Blend_INV_ALPHA_FACTOR = 21
	};

	enum SKTBD_LogicOp
	{
		SKTBD_LogicOp_CLEAR = 0,
		SKTBD_LogicOp_SET =				(SKTBD_LogicOp_CLEAR + 1),
		SKTBD_LogicOp_COPY =			(SKTBD_LogicOp_SET + 1),
		SKTBD_LogicOp_COPY_INVERTED =	(SKTBD_LogicOp_COPY + 1),
		SKTBD_LogicOp_NOOP =			(SKTBD_LogicOp_COPY_INVERTED + 1),
		SKTBD_LogicOp_INVERT =			(SKTBD_LogicOp_NOOP + 1),
		SKTBD_LogicOp_AND =				(SKTBD_LogicOp_INVERT + 1),
		SKTBD_LogicOp_NAND =			(SKTBD_LogicOp_AND + 1),
		SKTBD_LogicOp_OR =				(SKTBD_LogicOp_NAND + 1),
		SKTBD_LogicOp_NOR =				(SKTBD_LogicOp_OR + 1),
		SKTBD_LogicOp_XOR =				(SKTBD_LogicOp_NOR + 1),
		SKTBD_LogicOp_EQUIV =			(SKTBD_LogicOp_XOR + 1),
		SKTBD_LogicOp_AND_REVERSE =		(SKTBD_LogicOp_EQUIV + 1),
		SKTBD_LogicOp_AND_INVERTED =	(SKTBD_LogicOp_AND_REVERSE + 1),
		SKTBD_LogicOp_OR_REVERSE =		(SKTBD_LogicOp_AND_INVERTED + 1),
		SKTBD_LogicOp_OR_INVERTED =		(SKTBD_LogicOp_OR_REVERSE + 1)
	};

	enum SKTBD_BlendOp
	{
		SKTBD_BlendOp_ADD = 1,
		SKTBD_BlendOp_SUBTRACT = 2,
		SKTBD_BlendOp_REV_SUBTRACT = 3,
		SKTBD_BlendOp_MIN = 4,
		SKTBD_BlendOp_MAX = 5
	};

	struct RT_BlendConfig
	{
		bool BlendEnable;
		bool LogicOpEnable;
		SKTBD_Blend SrcBlend;
		SKTBD_Blend DestBlend;
		SKTBD_BlendOp BlendOp;
		SKTBD_Blend SrcBlendAlpha;
		SKTBD_Blend DestBlendAlpha;
		SKTBD_BlendOp BlendOpAlpha;
		SKTBD_LogicOp LogicOp;
		uint8_t RenderTargetWriteMask;
	};

	struct BlendConfig
	{
		bool AlphaToCoverage;
		bool IndependentBlendEnable;
		std::array<RT_BlendConfig, 8> RTBlendConfigs;
	};

	enum SKTBD_CompareOp
	{
		SKTBD_CompareOp_NONE = 0,
		SKTBD_CompareOp_NEVER = 1,
		SKTBD_CompareOp_LESS = 2,
		SKTBD_CompareOp_EQUAL = 3,
		SKTBD_CompareOp_LESS_EQUAL = 4,
		SKTBD_CompareOp_GREATER = 5,
		SKTBD_CompareOp_NOT_EQUAL = 6,
		SKTBD_CompareOp_GREATER_EQUAL = 7,
		SKTBD_CompareOp_ALWAYS = 8
	};

	enum SKTBD_StencilOp
	{
		SKTBD_StencilOp_KEEP = 1,
		SKTBD_StencilOp_ZERO = 2,
		SKTBD_StencilOp_REPLACE = 3,
		SKTBD_StencilOp_INCR_SAT = 4,
		SKTBD_StencilOp_DECR_SAT = 5,
		SKTBD_StencilOp_INVERT = 6,
		SKTBD_StencilOp_INCR = 7,
		SKTBD_StencilOp_DECR = 8
	};

	struct DepthStencilConfig
	{
		bool DepthEnable;
		bool DepthWriteAll;
		bool DepthBoundsTestEnable;
		SKTBD_CompareOp DepthFunc;

		bool StencilEnable;

		struct SKTBD_DepthStencilOpDesc
		{
			SKTBD_StencilOp StencilFailOp, StencilDepthFailOp, StencilPassOp;
			SKTBD_CompareOp StencilFunc;
			uint8_t StencilReadMask;
			uint8_t StencilWriteMask;

			bool operator==(const SKTBD_DepthStencilOpDesc& other) const
			{
				return  StencilFailOp		== other.StencilFailOp &&
						StencilDepthFailOp	== other.StencilDepthFailOp &&
						StencilPassOp		== other.StencilPassOp &&
						StencilFunc			== other.StencilFunc;
			};
		};

		SKTBD_DepthStencilOpDesc FrontFace, BackFace;
	};

	enum SKTBD_IBStripCutValue {
		SKTBD_IBStripCutValue_none = 0,
		SKTBD_IBStripCutValue_0xFFFF = 1,
		SKTBD_IBStripCutValue_0xFFFFFFFF = 2
	};

	struct GraphicsPipelineDesc
	{
		//Input Assembly Stage Configuration;
		BufferLayout InputVertexLayout;				// IA
		PrimitiveTopologyType_ InputPrimitiveType;	// IA
		SKTBD_IBStripCutValue TriangleStripCutValue;

		//Rasterization Stage Configuration
		RasterizerConfig Rasterizer;				// RASTERIZER
		
		//Output Merger Stage 
		DepthStencilConfig DepthStencil;			// OM
		BlendConfig Blend;							// OM
		uint32_t RenderTargetCount;					// OM
		std::array<DataFormat_,8> RenderTargetDataFormats; // OM
		DataFormat_ DepthstencilTargetFormat; // OM
		
		uint32_t SampleMask;	// MSAA OM?	
		uint32_t SampleCount;	// MSAA OM?
		uint32_t SampleQuality;	// MSAA OM?

		ShaderModuleDesc VertexShader;
		ShaderModuleDesc HullShader;
		ShaderModuleDesc DomainShader;
		ShaderModuleDesc GeometryShader;
		ShaderModuleDesc PixelShader;
		ShaderModuleDesc AmplificationShader;
		ShaderModuleDesc MeshShader;


		void SetVertexShader(const wchar_t* filename, const wchar_t* entrypoint = nullptr) { VertexShader.FileName = filename, VertexShader.EntryPoint = entrypoint; }
		void SetHullShader(const wchar_t* filename, const wchar_t* entrypoint = nullptr) { HullShader.FileName = filename, HullShader.EntryPoint = entrypoint; }
		void SetDomainShader(const wchar_t* filename, const wchar_t* entrypoint = nullptr) { DomainShader.FileName = filename, DomainShader.EntryPoint = entrypoint; }
		void SetGeometryShader(const wchar_t* filename, const wchar_t* entrypoint = nullptr) { GeometryShader.FileName = filename, GeometryShader.EntryPoint = entrypoint; }
		void SetAmplificationShader(const wchar_t* filename, const wchar_t* entrypoint = nullptr) { AmplificationShader.FileName = filename, AmplificationShader.EntryPoint = entrypoint; }
		void SetMeshShader(const wchar_t* filename, const wchar_t* entrypoint = nullptr) { MeshShader.FileName = filename, MeshShader.EntryPoint = entrypoint; }
		void SetPixelShader(const wchar_t* filename, const wchar_t* entrypoint = nullptr) { PixelShader.FileName = filename, PixelShader.EntryPoint = entrypoint; }
	};

	struct ComputePipelineDesc
	{
		ShaderModuleDesc ComputeShader;
		void SetComputeShader(const wchar_t* filename, const wchar_t* entrypoint) { ComputeShader.FileName = filename, ComputeShader.EntryPoint = entrypoint; }
	};

	enum RaytracingPipelineFlags
	{
		RaytracingPipelineFlags_None = 0,
		RaytracingPipelineFlags_SkipTriangles = 0x100,
		RaytracingPipelineFlags_SkipProcedurals = 0x200,
	};
	ENUM_FLAG_OPERATORS(RaytracingPipelineFlags)

	struct RaytracingPipelineDesc
	{
		RaytracingShaderLibrary RaytracingShaders;
		uint32_t MaxPayloadSize;
		uint32_t MaxAttributeSize;
		uint32_t MaxTraceRecursionDepth;
		uint32_t MaxCallableShaderRecursionDepth;
		RaytracingPipelineFlags Flags;
		

		void SetRaytracingLibrary(const wchar_t* libraryFilename, const wchar_t* raygenEntryPoint);
		void AddHitGroup(const wchar_t* hitGroupName, const wchar_t* anyHitEntryPoint, const wchar_t* closestHitEntryPoint, const wchar_t* intersectionEntryPoint, RaytracingHitGroupType_ type);
		void AddMissShader(const wchar_t* missEntryPoint);
		void AddCallableShader(const wchar_t* shaderEntryPoint);
		void SetConfig(uint32_t maxPayloadSize, uint32_t maxAttributeSize, uint32_t maxRecursionDepth, uint32_t maxCallableShaderRecursionDepth = 1u);
	};

	/*struct PipelineDesc
	{
		PipelineType_ Type;
		ShaderInputLayout* GlobalLayoutSignature;

		std::variant<GraphicsPipelineDesc*, ComputePipelineDesc*, RaytracingPipelineDesc*> TypeDesc;
	};*/

	class SamplerState : public GPUResource
	{
	protected:
		SamplerState() {}
	public:
		virtual uint32_t GetSamplerIndex() = 0;
		GPUResourceType_ GetResourceType() override { return GpuResourceType_SamplerState; }
	};

	class Pipeline : public GPUResource
	{
	protected:
		Pipeline() = default;

	public:
		GPUResourceType_ GetResourceType() override { return GpuResourceType_PipelineState; }

		constexpr virtual PipelineType_ GetType() = 0;
		ShaderInputLayout* GetInputLayout() const { return m_ShaderInputs; }

	protected:
		ShaderInputLayout* m_ShaderInputs;
	};

	class ComputePipeline : public Pipeline
	{
		constexpr PipelineType_ GetType() override { return PipelineType_Compute; }
	};

	class GraphicsPipeline : public Pipeline
	{
		constexpr PipelineType_ GetType() override { return PipelineType_Graphics; }
	};

}

