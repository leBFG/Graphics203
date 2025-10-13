#include <sktbdpch.h>
#include "AGCPipeline.h"
#include "AGCBuffer.h"
//#include "AGCSrtSignature.h"

#define SKTBD_LOG_COMPONENT "AGCPipeline"
#include "AGCCommandBuffer.h"
#include "Skateboard/Log.h"

namespace Skateboard
{
	/*struct SRT
	{
		sce::Agc::Core::Buffer gPassBuffer;
		sce::Agc::Core::Buffer gVertices;
	};*/

	AGCShaderInputLayout::AGCShaderInputLayout(const ShaderInputLayoutDesc& desc) : ShaderInputLayout(desc)
	{

		auto ShaderResourceDescritproToSizeInBytes = [&](const ShaderResourceDesc& rd)->uint8_t {
			switch (rd.ShaderElementType)
			{
			case ShaderElementType_Unknown:
				SKTBD_LOG_ASSERT(false,"UnknownShaderComponent")
			case ShaderElementType_RootConstant:
				UserDataSizeInBytes += sizeof(uint32_t) * rd.Constant.Num32BitValues;
				return sizeof(uint32_t) * rd.Constant.Num32BitValues;
				break;
			case ShaderElementType_ConstantBufferView:
				UserDataSizeInBytes += sizeof(sce::Agc::Core::Buffer);
				return sizeof(sce::Agc::Core::Buffer);
				break;
			case ShaderElementType_ShaderResourceView:
				UserDataSizeInBytes += sizeof(sce::Agc::Core::Buffer);
				return sizeof(sce::Agc::Core::Buffer);
				break;
			case ShaderElementType_UnorderedAccessView:
				UserDataSizeInBytes += sizeof(sce::Agc::Core::Buffer);
				return sizeof(sce::Agc::Core::Buffer);
				break;
			case ShaderElementType_DescriptorTable:
				UserDataSizeInBytes += sizeof(uint8_t*); // The Pointer not specifically the buffer pointer // Descritor tables are just other srtees on ps5;
				return sizeof(uint8_t*);
				break;
			case ShaderElementType_Sampler:
				UserDataSizeInBytes += sizeof(sce::Agc::Core::Sampler);
				return sizeof(sce::Agc::Core::Texture);
				break;
			}
			return 0;
		};

		UserDataSizeInBytes = 0;

		SKTBDReservedSizeInBytes = 0;
		SKTBDReservedSlotCount = 0;

		SRT_OFFSETS_IN_BYTES.resize(desc.vPipelineInputs.size());

		SRT_OFFSETS_IN_BYTES[0] = 0; //0 is always at 0
		UserDataSlotCount = desc.vPipelineInputs.size(); //slot count always equals input count
		ShaderResourceDescritproToSizeInBytes(desc.vPipelineInputs[desc.vPipelineInputs.size()-1]);//size of the last element functor captures user data size by &

		//transform elements 1 to last 
		std::transform(desc.vPipelineInputs.begin(), desc.vPipelineInputs.end()-1, SRT_OFFSETS_IN_BYTES.begin()+1, ShaderResourceDescritproToSizeInBytes);

		//ADD RootDescriptors at the end
		if(desc.DescriptorsDirctlyAddresssed) SKTBDReservedSizeInBytes +=	sizeof(sce::Agc::Core::Buffer);
		if(desc.SamplersDirectlyAddressed)	  SKTBDReservedSizeInBytes +=	sizeof(sce::Agc::Core::Buffer);

		//resize vector for the to hold the data;
		SRT_DATA.resize(UserDataSizeInBytes + SKTBDReservedSizeInBytes);

		//Write Resource heap descriptors into the end of SRT;
		auto offset = UserDataSizeInBytes;

		if (desc.DescriptorsDirctlyAddresssed)
		{
			auto buffer = gAGCContext->GetResourceDescriptorHeapBuffer();
			memcpy(&SRT_DATA[offset], &buffer, sizeof(sce::Agc::Core::Buffer));
		}

		if (desc.SamplersDirectlyAddressed)
		{
			auto buffer = gAGCContext->GetSamplerHeap();
			memcpy(&SRT_DATA[offset], &buffer, sizeof(sce::Agc::Core::Buffer));
		}
	}

	void LoadShader(const wchar_t* filename, AGCShader& out_shader)
	{
		// Sanity checks
		if (!filename)
			return;
		SKTBD_LOG_ASSERT(!out_shader.pShader, "The shader stage has already been loaded.\
			Make sure you use only one of the Vertex, Domain or Geometry entries in your description!");

		// Check that the shader file exists
		std::filesystem::path path(SanitizeFilePath(filename));
		path.replace_extension(".ags");
		SKTBD_MSG_INFO("loading shader from {}", path.c_str());
		SKTBD_LOG_ASSERT(std::filesystem::exists(path), "Shader file does not exist!");

		// Read raw data
		size_t size = std::filesystem::file_size(path);
		char* streamBinary = new char[size];
		std::ifstream file(path, std::ios::binary);
		file.read(streamBinary, size);
		file.close();

		// Retrieve the shader header and GPU code
		// We will also retrieve the SRT signature from the shader metadata so that we can make the appropriate bindings
		// Read: https://p.siedev.net/resources/documents/SDK/7.000/Shader_Reflection-Overview/0002.html#__document_toc_00000006
		const SceShaderBinaryHandle sl = sceShaderGetBinaryHandle(streamBinary);
		GetSrtSignature(sl, out_shader);
		const size_t headerSize = sceShaderGetProgramHeaderSize(sl);
		const size_t codeSize = sceShaderGetProgramSize(sl);
		out_shader.Binary.Header = gAGCContext->GetMemAllocator()->TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kShaderHeader>(headerSize);
		out_shader.Binary.Code = gAGCContext->GetMemAllocator()->TypedAlignedAllocate<uint8_t, sce::Agc::Alignment::kShaderCode>(codeSize);
		memcpy(out_shader.Binary.Header.data, sceShaderGetProgramHeader(sl), headerSize);
		memcpy(out_shader.Binary.Code.data, sceShaderGetProgram(sl), codeSize);
		delete[] streamBinary;

		// Create the shader on AGC
		SceError error = sce::Agc::createShader(&out_shader.pShader, out_shader.Binary.Header.data, out_shader.Binary.Code.data);
		SCE_AGC_ASSERT(error == SCE_OK);

#ifndef SKTBD_SHIP
		sce::Agc::Core::registerResource(out_shader.pShader, path.filename().c_str());
#endif
	}

	void GetSrtSignature(const _SceShaderBinaryHandle* sl, AGCShader& out_shader)
	{
		// md = MetaData
		// lh = list handle
		// r = resource
		// rc = resource class
		// th = type handle
		// tc = type class
		// m = member
		const SceShaderMetadataSectionHandle md = sceShaderGetMetadataSection(sl);
		const SceShaderResourceListHandle lh = sceShaderGetResourceList(md);

		// Loop through all resources from the list
		for (SceShaderResourceHandle r = sceShaderGetFirstResource(md, lh); r != nullptr; r = sceShaderGetNextResource(md, r))
		{
			const SceShaderResourceClass rc = sceShaderGetResourceClass(md, r);
			/*int32_t resourceSlot = sceShaderGetResourceApiSlot(md, r);
			if (rc == SceShaderSrv || rc == SceShaderCb || rc == SceShaderUav || rc == SceShaderSamplerState)
			{
				const SceShaderTypeHandle th = sceShaderGetResourceType(md, r);
				const SceShaderTypeClass tc = sceShaderGetTypeClass(md, th);

				SceShaderTypeHandle const bufferElemType = sceShaderGetBufferElementType(md, th);
				SceShaderBufferClass const bufferClass = sceShaderGetBufferClass(md, bufferElemType);
				int bufferSize = sceShaderGetStructSize(md, bufferElemType);

				SignatureParameter param = {};
				param.MemberSize = bufferSize;
				param.RegisterSlot = resourceSlot;

			}*/
			if (rc != SceShaderSrt)
				continue;

			const SceShaderTypeHandle th = sceShaderGetResourceType(md, r);
			const SceShaderTypeClass tc = sceShaderGetTypeClass(md, th);
			if (tc != SceShaderStructType)
				continue;

			for (SceShaderMemberHandle m = sceShaderGetFirstMember(md, th); nullptr != m; m = sceShaderGetNextMember(md, m))
			{
				SignatureParameter param = {};
				param.MemberOffset = sceShaderGetMemberOffset(md, m);
				param.MemberSize = sceShaderGetMemberSize(md, m);

				// Descriptor tables are just pointers, so they wont contain a type, register slot or register space
				if (!sceShaderIsSrtSignatureResource(md, m))
				{
					// TODO:
					const SceShaderTypeHandle mt = sceShaderGetMemberType(md, m);
					const SceShaderTypeClass mc = sceShaderGetTypeClass(md, mt);
					if (mc != SceShaderPointerType)	// Ensure this is a descriptor table
						continue;
					param.ElementType = ShaderElementType_DescriptorTable;
				}
				// Other entries provide all the necessary information
				else
				{
					param.ElementType = AGCResourceClassToShaderElementType(sceShaderGetSrtSignatureResourceClass(md, m));
					//param.RegisterSlot = sceShaderGetSrtSignatureResourceSlotIndex(md, m);
					//param.RegisterSpace = sceShaderGetSrtSignatureResourceSpaceIndex(md, m);
				}

				//out_shader.SRT.SignatureRef().SizeInBytes += param.MemberSize;
				//SKTBD_LOG_ASSERT(out_shader.SRT.SignatureRef().SizeInBytes < 16u * 4u, "The SRT signature is too large! Did the PSSL even let it compile? (max 16 DWORDS)");
				//out_shader.SRT.SignatureRef().vParameters.emplace_back(param);
			}
		}
	}

	AGCComputePipeline::AGCComputePipeline(const ComputePipelineDesc& desc)
	{

	}

	void AGCComputePipeline::SetDebugName(const std::wstring& debug_name)
	{
		ComputePipeline::SetDebugName(debug_name);
	}

	//translation functions *inspired* by Unreal Engine i.e. directly taken from
	template<typename Enum>
	Enum TranslateBlendFactor(SKTBD_Blend BlendFactor)
	{
		switch (BlendFactor)
		{
		default:								
		case SKTBD_Blend_ZERO:					return Enum::kZero;
		case SKTBD_Blend_ONE:					return Enum::kOne;
		case SKTBD_Blend_SRC_COLOR:				return Enum::kSrcColor;
		case SKTBD_Blend_INV_SRC_COLOR:			return Enum::kOneMinusSrcColor;
		case SKTBD_Blend_SRC_ALPHA:				return Enum::kSrcAlpha;
		case SKTBD_Blend_INV_SRC_ALPHA:			return Enum::kOneMinusSrcAlpha;
		case SKTBD_Blend_DEST_ALPHA:			return Enum::kDestAlpha;
		case SKTBD_Blend_INV_DEST_ALPHA:		return Enum::kOneMinusDestAlpha;
		case SKTBD_Blend_DEST_COLOR:			return Enum::kDestColor;
		case SKTBD_Blend_INV_DEST_COLOR:		return Enum::kOneMinusDestColor;
		}
	}

	template<typename Enum>
	Enum TranslateBlendOp(SKTBD_BlendOp BlendOp)
	{
		switch (BlendOp)
		{
		default:					 // fall through
		case SKTBD_BlendOp::SKTBD_BlendOp_ADD:				return Enum::kAdd;
		case SKTBD_BlendOp::SKTBD_BlendOp_SUBTRACT:			return Enum::kSubtract;
		case SKTBD_BlendOp::SKTBD_BlendOp_MIN:				return Enum::kMin;
		case SKTBD_BlendOp::SKTBD_BlendOp_MAX:				return Enum::kMax;
		case SKTBD_BlendOp::SKTBD_BlendOp_REV_SUBTRACT:		return Enum::kReverseSubtract;
		};
	}

	template<typename Enum>
	Enum TranslateStencilOp(SKTBD_StencilOp StencilOp)
	{
		switch (StencilOp)
		{
		default:					 // fall through
		case SKTBD_StencilOp::SKTBD_StencilOp_KEEP:				return Enum::kKeep;
		case SKTBD_StencilOp::SKTBD_StencilOp_ZERO:				return Enum::kZero;
		case SKTBD_StencilOp::SKTBD_StencilOp_REPLACE:			return Enum::kReplaceTest;
		case SKTBD_StencilOp::SKTBD_StencilOp_INCR_SAT:			return Enum::kAddClamp;
		case SKTBD_StencilOp::SKTBD_StencilOp_DECR_SAT:			return Enum::kSubClamp;
		case SKTBD_StencilOp::SKTBD_StencilOp_INVERT:			return Enum::kInvert;
		case SKTBD_StencilOp::SKTBD_StencilOp_INCR:				return Enum::kAddWrap;
		case SKTBD_StencilOp::SKTBD_StencilOp_DECR:				return Enum::kSubWrap;
		};
	}

	template<typename Enum>
	Enum TranslateCompareOp(SKTBD_CompareOp StencilOp)
	{
		switch (StencilOp)
		{
		default:														// fall through
		case SKTBD_CompareOp::SKTBD_CompareOp_NONE:
		case SKTBD_CompareOp::SKTBD_CompareOp_ALWAYS:				return Enum::kAlways;
		case SKTBD_CompareOp::SKTBD_CompareOp_EQUAL:				return Enum::kEqual;
		case SKTBD_CompareOp::SKTBD_CompareOp_GREATER:				return Enum::kGreater;
		case SKTBD_CompareOp::SKTBD_CompareOp_GREATER_EQUAL:		return Enum::kGreaterEqual;
		case SKTBD_CompareOp::SKTBD_CompareOp_LESS:					return Enum::kLess;
		case SKTBD_CompareOp::SKTBD_CompareOp_LESS_EQUAL:			return Enum::kLessEqual;
		case SKTBD_CompareOp::SKTBD_CompareOp_NEVER:				return Enum::kNever;
		case SKTBD_CompareOp::SKTBD_CompareOp_NOT_EQUAL:			return Enum::kNotEqual;
		};
	}

	sce::Agc::CxPrimitiveSetup::CullFace TranslateCull(CullMode Mode)
	{
		switch (Mode)
		{
		case Cull_NONE:		return sce::Agc::CxPrimitiveSetup::CullFace::kNone;
		case Cull_FRONT:	return sce::Agc::CxPrimitiveSetup::CullFace::kFront;
		case Cull_BACK:		return sce::Agc::CxPrimitiveSetup::CullFace::kBack;
		default: ;
		};
	}

	template<typename Enum>
	Enum TranslateEnable(bool Enable)
	{
		return (Enable) ? Enum::kEnable : Enum::kDisable;
	}

	sce::Agc::Core::VertexAttribute::Format TranslateVAFormat(ShaderDataType_ type)
	{
		switch(type)
		{
		default:
		case ShaderDataType_::None:		return sce::Agc::Core::VertexAttribute::Format::kVertexBuffer;
		case ShaderDataType_::Bool:		return sce::Agc::Core::VertexAttribute::Format::k8UInt;
		case ShaderDataType_::Int:		return sce::Agc::Core::VertexAttribute::Format::k32SInt;
		case ShaderDataType_::Int2:		return sce::Agc::Core::VertexAttribute::Format::k32_32SInt;
		case ShaderDataType_::Int3:		return sce::Agc::Core::VertexAttribute::Format::k32_32_32SInt;
		case ShaderDataType_::Int4:		return sce::Agc::Core::VertexAttribute::Format::k32_32_32_32SInt;
		case ShaderDataType_::Uint:		return sce::Agc::Core::VertexAttribute::Format::k32UInt;
		case ShaderDataType_::Uint2:	return sce::Agc::Core::VertexAttribute::Format::k32_32UInt;
		case ShaderDataType_::Uint3:	return sce::Agc::Core::VertexAttribute::Format::k32_32_32UInt;
		case ShaderDataType_::Uint4:	return sce::Agc::Core::VertexAttribute::Format::k32_32_32_32UInt;
		case ShaderDataType_::Float:	return sce::Agc::Core::VertexAttribute::Format::k32Float;
		case ShaderDataType_::Float2:	return sce::Agc::Core::VertexAttribute::Format::k32_32Float;
		case ShaderDataType_::Float3:	return sce::Agc::Core::VertexAttribute::Format::k32_32_32Float;
		case ShaderDataType_::Float4:	return sce::Agc::Core::VertexAttribute::Format::k32_32_32_32Float;
		}
	}

	static sce::Agc::UcPrimitiveType::Type TranslatePrimitiveType(SKTBD_PRIMITIVE_TOPOLOGY Type, bool UsingMeshShader)
	{
		if (UsingMeshShader)
		{
			return sce::Agc::UcPrimitiveType::Type::kPointList;
		}
		switch (Type)
		{
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST:						return sce::Agc::UcPrimitiveType::Type::kTriList;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:						return sce::Agc::UcPrimitiveType::Type::kTriStrip;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLEFAN:						return sce::Agc::UcPrimitiveType::Type::kTriFan;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_LINELIST:							return sce::Agc::UcPrimitiveType::Type::kLineList;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_LINESTRIP:							return sce::Agc::UcPrimitiveType::Type::kLineStrip;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_POINTLIST:							return sce::Agc::UcPrimitiveType::Type::kPointList;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:					return sce::Agc::UcPrimitiveType::Type::kTriListAdjacency;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:					return sce::Agc::UcPrimitiveType::Type::kTriStripAdjacency;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:						return sce::Agc::UcPrimitiveType::Type::kLineListAdjacency;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:						return sce::Agc::UcPrimitiveType::Type::kLineStripAdjacency;
		case SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST:			return sce::Agc::UcPrimitiveType::Type::kPatch;
		default:
			SKTBD_LOG_ASSERT(false, "Unknown Primitive type");
			return sce::Agc::UcPrimitiveType::Type::kPatch;
		}
	}

	template<typename Enum>
	Enum TranslatePolygonMode(PrimitiveTopologyType_ Type)
	{
		switch (Type)
		{
		default:
		case PrimitiveTopologyType_none:		return Enum::kFill;
		case PrimitiveTopologyType_Point:    	return Enum::kPoint;
		case PrimitiveTopologyType_Line:		return Enum::kLine;
		case PrimitiveTopologyType_Triangle:	return Enum::kFill;
		}
	}

	void AGCGraphicsPipeline::SetDebugName(const std::wstring& debug_name)
	{
		GraphicsPipeline::SetDebugName(debug_name);
	}

	AGCGraphicsPipeline::AGCGraphicsPipeline(const GraphicsPipelineDesc& desc)
	{
		uint32_t cx_registers_req=0;
		uint32_t sh_registers_req=0;
		uint32_t uc_registers_req=0;

		uint shader_num = 0;
		sce::Agc::Shader* Shaders[2];

		//Create vertex Attributes if there are any
		if (!desc.InputVertexLayout.GetElements().empty())
		{
			m_VATable.init(desc.InputVertexLayout.GetElements().size());
			std::transform(desc.InputVertexLayout.begin(), desc.InputVertexLayout.end(), m_VATable.GetVertexAttributeTable(), [](const BufferElement& element) -> sce::Agc::Core::VertexAttribute
				{
					return { element.InputSlot, TranslateVAFormat(element.DataType), element.Offset, (element.AttributeType == PerVertexData) ? sce::Agc::Core::VertexAttribute::Index::kVertexId : sce::Agc::Core::VertexAttribute::Index::kInstanceId };
				});
		}

		if (desc.VertexShader.FileName)
		{
			LoadShader(desc.VertexShader.FileName, m_VertexShader); m_StatesToSet.flip(AGCPipelineStates::GS);

			cx_registers_req += m_VertexShader.pShader->m_numCxRegisters;
			sh_registers_req += m_VertexShader.pShader->m_numShRegisters;

			SKTBD_LOG_INFO("GraphicsPipeline", "CXRegister Count Requested VS {}", m_VertexShader.pShader->m_numCxRegisters)
			SKTBD_LOG_INFO("GraphicsPipeline", "SHRegister Count Requested VS {}", m_VertexShader.pShader->m_numShRegisters)
				
			Shaders[shader_num] = m_VertexShader.pShader;
			shader_num++;
		}

		if (desc.PixelShader.FileName)
		{
			LoadShader(desc.PixelShader.FileName, m_PixelShader); m_StatesToSet.flip(AGCPipelineStates::PS);

			cx_registers_req += m_PixelShader.pShader->m_numCxRegisters;
			sh_registers_req += m_PixelShader.pShader->m_numShRegisters;

			SKTBD_LOG_INFO("GraphicsPipeline", "CXRegister Count Requested PS {}", m_PixelShader.pShader->m_numCxRegisters)
			SKTBD_LOG_INFO("GraphicsPipeline", "SHRegister Count Requested PS {}", m_PixelShader.pShader->m_numShRegisters)

			Shaders[shader_num] = m_PixelShader.pShader;
			shader_num++;
		}

		cx_registers_req += sizeof(AgcConstantCXRegisters) / sizeof(sce::Agc::CxRegister);
		uc_registers_req += sizeof(AgcConstantUCRegisters) / sizeof(sce::Agc::UcRegister);

		SKTBD_LOG_INFO("GraphicsPipeline", "CXRegister Count Requested PSO {}", sizeof(AgcConstantCXRegisters) / sizeof(sce::Agc::CxRegister))
		SKTBD_LOG_INFO("GraphicsPipeline", "UcRegister Count Requested PSO {}", sizeof(AgcConstantUCRegisters) / sizeof(sce::Agc::UcRegister))

		m_RegisterStates.init(cx_registers_req, uc_registers_req, sh_registers_req);

		auto cxState = (AgcConstantCXRegisters*)m_RegisterStates.GetCxRegisters();
		auto ucState = (AgcConstantUCRegisters*)m_RegisterStates.GetUcRegisters();
		auto shState = m_RegisterStates.GetShRegisters();

		//Register Population Structure
		//Constant state -> Shader State

		//interpreting desc
		{
			//DepthStencil
			{
				bool separateStencil = desc.DepthStencil.FrontFace != desc.DepthStencil.BackFace;

				auto& DepthStencil = desc.DepthStencil;

				//Initilize depth testing states
				m_StatesToSet.flip(AGCPipelineStates::Depth);
				cxState->DepthStencilControl.init()
					.setDepth(TranslateEnable<sce::Agc::CxDepthStencilControl::Depth>(DepthStencil.DepthEnable))
					.setDepthFunction(TranslateCompareOp<sce::Agc::CxDepthStencilControl::DepthFunction>(DepthStencil.DepthFunc))
					.setDepthWrite(TranslateEnable<sce::Agc::CxDepthStencilControl::DepthWrite>(DepthStencil.DepthWriteAll))
					.setStencil(TranslateEnable<sce::Agc::CxDepthStencilControl::Stencil>(DepthStencil.StencilEnable))
					.setSeparateStencil(TranslateEnable<sce::Agc::CxDepthStencilControl::SeparateStencil>(separateStencil))
					.setStencilFunction(TranslateCompareOp < sce::Agc::CxDepthStencilControl::StencilFunction>(DepthStencil.BackFace.StencilFunc))
					.setStencilFunctionBack(TranslateCompareOp < sce::Agc::CxDepthStencilControl::StencilFunctionBack>(DepthStencil.FrontFace.StencilFunc))
					.setDepthBounds(TranslateEnable<sce::Agc::CxDepthStencilControl::DepthBounds>(DepthStencil.DepthBoundsTestEnable));

				cxState->StencilOpControl.init()
					.setStencilFailBackOp(TranslateStencilOp<sce::Agc::CxStencilOpControl::StencilFailBackOp>(DepthStencil.BackFace.StencilFailOp))
					.setStencilFailOp(TranslateStencilOp<sce::Agc::CxStencilOpControl::StencilFailOp>(DepthStencil.FrontFace.StencilFailOp))
					.setStencilZFailBackOp(TranslateStencilOp<sce::Agc::CxStencilOpControl::StencilZFailBackOp>(DepthStencil.BackFace.StencilDepthFailOp))
					.setStencilZFailOp(TranslateStencilOp<sce::Agc::CxStencilOpControl::StencilZFailOp>(DepthStencil.FrontFace.StencilFailOp))
					.setStencilZPassBackOp(TranslateStencilOp<sce::Agc::CxStencilOpControl::StencilZPassBackOp>(DepthStencil.BackFace.StencilPassOp))
					.setStencilZPassOp(TranslateStencilOp<sce::Agc::CxStencilOpControl::StencilZPassOp>(DepthStencil.FrontFace.StencilPassOp));
			}

			// Blend State Initialization
			{
				const auto& Blend = desc.Blend;
				auto& PSO = *cxState;

				bool bHasActiveRenderTargets = false;
				PSO.RenderTargetMask.init();
				for (uint32_t Index = 0; Index < MAX_RTS; Index++)
				{
					const auto& InitState = Blend.RTBlendConfigs[Index];
					if (Index == 0)
					{
						auto& State = PSO.RT0;
						State.init();
					}
					else
					{
						auto& State = PSO.RT17[Index - 1];
						State.init();
						State.setSlot(Index);
					}

					if (Index < desc.RenderTargetCount && desc.RenderTargetDataFormats[Index] != DataFormat_UNKNOWN)
					{
						/*bool bBlendEnabled =
							InitState.ColorBlendOp != BO_Add || InitState.ColorDestBlend != BF_Zero || InitState.ColorSrcBlend != BF_One ||
							InitState.AlphaBlendOp != BO_Add || InitState.AlphaDestBlend != BF_Zero || InitState.AlphaSrcBlend != BF_One;*/

						// translate settings, if blending is enabled. otherwise, the defaults will disable blending
						if (InitState.BlendEnable)
						{
							bool bSeparateBlend = InitState.BlendOpAlpha != InitState.BlendOp && InitState.SrcBlend != InitState.SrcBlendAlpha && InitState.DestBlend != InitState.DestBlendAlpha;

							if (Index == 0)
							{
								auto& State = PSO.RT0;
								State.setBlend(sce::Agc::CxDualSourceBlendControl::Blend::kEnable)
								.setSeparateAlphaBlend(TranslateEnable < sce::Agc::CxDualSourceBlendControl::SeparateAlphaBlend>(bSeparateBlend))
								.setColorSourceMultiplier(TranslateBlendFactor<sce::Agc::CxDualSourceBlendControl::ColorSourceMultiplier>(InitState.SrcBlend))
								.setColorDestMultiplier(TranslateBlendFactor<sce::Agc::CxDualSourceBlendControl::ColorDestMultiplier>(InitState.DestBlend))
								.setColorBlendFunc(TranslateBlendOp<sce::Agc::CxDualSourceBlendControl::ColorBlendFunc>(InitState.BlendOp))

								.setAlphaSourceMultiplier(TranslateBlendFactor<sce::Agc::CxDualSourceBlendControl::AlphaSourceMultiplier>(InitState.SrcBlendAlpha))
								.setAlphaDestMultiplier(TranslateBlendFactor<sce::Agc::CxDualSourceBlendControl::AlphaDestMultiplier>(InitState.DestBlendAlpha))
								.setAlphaBlendFunc(TranslateBlendOp<sce::Agc::CxDualSourceBlendControl::AlphaBlendFunc>(InitState.BlendOpAlpha));
							}
							else
							{
								auto& State = PSO.RT17[Index - 1];
								State.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
									.setSeparateAlphaBlend(sce::Agc::CxBlendControl::SeparateAlphaBlend::kEnable)
									.setColorSourceMultiplier(TranslateBlendFactor<sce::Agc::CxBlendControl::ColorSourceMultiplier>(InitState.SrcBlend))
									.setColorDestMultiplier(TranslateBlendFactor<sce::Agc::CxBlendControl::ColorDestMultiplier>(InitState.DestBlend))
									.setColorBlendFunc(TranslateBlendOp<sce::Agc::CxBlendControl::ColorBlendFunc>(InitState.BlendOp))

									.setAlphaSourceMultiplier(TranslateBlendFactor<sce::Agc::CxBlendControl::AlphaSourceMultiplier>(InitState.SrcBlendAlpha))
									.setAlphaDestMultiplier(TranslateBlendFactor<sce::Agc::CxBlendControl::AlphaDestMultiplier>(InitState.DestBlendAlpha))
									.setAlphaBlendFunc(TranslateBlendOp<sce::Agc::CxBlendControl::AlphaBlendFunc>(InitState.BlendOpAlpha));
							}
						}

						// set the write masking for this RT
						uint32_t Mask = InitState.RenderTargetWriteMask & 0xF;
						PSO.RenderTargetMask.setMask(Index, Mask);

						bHasActiveRenderTargets = bHasActiveRenderTargets || Mask != 0;
					}
					else
					{
						// Disable writes to unbound render targets
						PSO.RenderTargetMask.setMask(Index, 0);
					}
				}

				// Disable the Cb entirely if there are no active render targets.
				if(bHasActiveRenderTargets)
				m_StatesToSet.flip(AGCPipelineStates::Blend);

				PSO.CbControl.init()
					.setRasterOp(sce::Agc::CxCbControl::RasterOp::kCopy)
					.setMode(bHasActiveRenderTargets
						? sce::Agc::CxCbControl::Mode::kNormal
						: sce::Agc::CxCbControl::Mode::kDisable);

				// Use a solid white blend color.
				PSO.BlendColor.init()
					.setRed(1.0f)
					.setGreen(1.0f)
					.setBlue(1.0f)
					.setAlpha(1.0f);
			} //End Blend state

			// Rasterizer
			{
				auto& raster = desc.Rasterizer;

				cxState->PrimitiveSetup.init();
				cxState->FrontOffset.init();
				cxState->BackOffset.init();
				cxState->ConservativeRasterizationControl.init();

				// what face what get rid of?
				cxState->PrimitiveSetup
					.setFrontFace(raster.FrontCC ? sce::Agc::CxPrimitiveSetup::FrontFace::kCcw : sce::Agc::CxPrimitiveSetup::FrontFace::kCw)
					.setCullFace(TranslateCull(raster.Cull));

				//if (bUseGsFlags)
				//{
				//	GsFlags = Agc::GsFlags::kCullOffScreen;
				//	if (Initializer.CullMode == CM_CW)
				//	{
				//		GsFlags |= Agc::GsFlags::kCullBack;
				//	}
				//	else if (Initializer.CullMode == CM_CCW)
				//	{
				//		GsFlags |= Agc::GsFlags::kCullFront;
				//	}
				//}

				// set wireframe/solid
				if (desc.InputPrimitiveType != PrimitiveTopologyType_Triangle || raster.Wireframe)
					cxState->PrimitiveSetup
					.setPolygonMode(sce::Agc::CxPrimitiveSetup::PolygonMode::kEnable)
					.setFrontPolygonMode(TranslatePolygonMode<sce::Agc::CxPrimitiveSetup::FrontPolygonMode>(desc.InputPrimitiveType))
					.setBackPolygonMode(TranslatePolygonMode<sce::Agc::CxPrimitiveSetup::BackPolygonMode>(desc.InputPrimitiveType));

				// is polygon offset enabled?
				float PolyOffset = raster.DepthBias;

				// @todo agp: Warning: this assumes depth bits == 24, and won't be correct with 32 (This matches D3D & OpenGL behaviour!)
				float PolyScale = raster.SlopeScaledDepthBias * float((1 << 24) - 1);
				if (PolyOffset != 0.0f || PolyScale != 0.0f)
				{
					// setup polygon offset / depth bias
					cxState->PrimitiveSetup.setFrontPolygonOffset(sce::Agc::CxPrimitiveSetup::FrontPolygonOffset::kEnable);
					cxState->PrimitiveSetup.setBackPolygonOffset(sce::Agc::CxPrimitiveSetup::BackPolygonOffset::kEnable);

					cxState->FrontOffset.setOffset(PolyOffset);
					cxState->FrontOffset.setScale(PolyScale);

					cxState->BackOffset.setOffset(PolyOffset);
					cxState->BackOffset.setScale(PolyScale);
				}

				cxState->ClipControl.init();

				// Use the DX clip space (0 <= Z <= W) rather than default OGL (-W <= Z <= W)
				cxState->ClipControl.setClipSpace(sce::Agc::CxClipControl::ClipSpace::kDX);

				// Match clipping behavior of D3D11. Without it clipping will only happen on perspective attributes.
				cxState->ClipControl.setLinearAttributeClip(sce::Agc::CxClipControl::LinearAttributeClip::kEnable);

				cxState->ClipControl.setZClipNear(TranslateEnable<sce::Agc::CxClipControl::ZClipNear>(raster.DepthClipEnable));
				cxState->ClipControl.setZClipFar(TranslateEnable<sce::Agc::CxClipControl::ZClipFar>(raster.DepthClipEnable));

				if(raster.ConservativeRasterEnable)
				switch (raster.ConservativeRasterization)
				{
				case Underestimate:
					cxState->ConservativeRasterizationControl.setMode(sce::Agc::CxConservativeRasterizationControl::Mode::kUnderestimating);
					break;
				case Overestimate:
					cxState->ConservativeRasterizationControl.setMode(sce::Agc::CxConservativeRasterizationControl::Mode::kOverestimating);
					break;
				default: ;
				}
				else
					cxState->ConservativeRasterizationControl.setMode(sce::Agc::CxConservativeRasterizationControl::Mode::kDisable);
			}
		}

		auto shaderCxRegs = (sce::Agc::CxRegister*)(cxState + 1);
		for (int i = 0; i < shader_num; ++i) {
			sce::Agc::Shader* shader = Shaders[i];
			std::copy_n(shader->m_cxRegisters, shader->m_numCxRegisters, shaderCxRegs);
			shaderCxRegs += shader->m_numCxRegisters;
		}

		auto shaderShRegs = shState;
		for (int i = 0; i < shader_num; ++i) {
			sce::Agc::Shader* shader = Shaders[i];
			std::copy_n(shader->m_shRegisters, shader->m_numShRegisters, shaderShRegs);
			shaderShRegs += shader->m_numShRegisters;
		}

		//auto err = sce::Agc::Core::linkShaders(&cxState->ShaderLinkage, &UcState->PrimitiveState,
		//	nullptr, m_VertexShader.pShader, m_PixelShader.pShader,
		//	TranslatePrimitiveType(desc->InputPrimitiveType);

		auto err = sce::Agc::Core::linkShaders(&cxState->ShaderLinkage, &ucState->PrimitiveState,
			nullptr, m_VertexShader.pShader, m_PixelShader.pShader,
			sce::Agc::UcPrimitiveType::Type::kTriList);

		SCE_AGC_ASSERT(err == SCE_OK);

		cxState->assertValid();
		ucState->PrimitiveState.assertValid();

	}


	//AGCRasterizationPipeline::AGCRasterizationPipeline(const std::wstring& debugName, const GraphicsPipelineDesc& desc) :
	//	RasterizationPipeline(debugName, desc),
	//	m_HullShader{},
	//	m_PrimitiveShader{},
	//	m_PixelShader{},
	//	m_VertexBufferSlot(0u)
	//{
	//	// Load Shaders and Link them
	//	LoadAllShaders();
	//	
	//	// The previous stage also retrieved the SRT signature from the shader metadata,
	//	// so we can now create the Shader Resource Table from the input resources in the
	//	// description and verify that all resources are bound at the correct locations
	//	CreateShaderResourceTable();


	//	// Completely describe how we want our primitives to be rendered.
	//	ConfigurePipelineSetup();

	//	// Initialise Binders (avoid redundancy)
	//	m_HullBinder.init();
	//	m_PrimitiveBinder.init();
	//	m_PixelBinder.init();

	//}

	//AGCRasterizationPipeline::~AGCRasterizationPipeline()
	//{
	//	// Release the shaders direct memory
	//	if (m_HullShader.pShader) gAGCContext->GetMemAllocator()->TypedAlignedFree(m_HullShader.Binary.Header), gAGCContext->GetMemAllocator()->TypedAlignedFree(m_HullShader.Binary.Code);
	//	if (m_PrimitiveShader.pShader) gAGCContext->GetMemAllocator()->TypedAlignedFree(m_PrimitiveShader.Binary.Header), gAGCContext->GetMemAllocator()->TypedAlignedFree(m_PrimitiveShader.Binary.Code);
	//	if (m_PixelShader.pShader) gAGCContext->GetMemAllocator()->TypedAlignedFree(m_PixelShader.Binary.Header), gAGCContext->GetMemAllocator()->TypedAlignedFree(m_PixelShader.Binary.Code);
	//}

	//void AGCRasterizationPipeline::Bind()
	//{
	//	// Get API objects
	//	sce::Agc::DrawCommandBuffer& dcb = gAGCContext->GetDrawCommandBuffer();
	//	sce::Agc::Core::StateBuffer& sb = gAGCContext->GetStateBuffer();

	//	// Bind the shaders
	//	// We need to first supply the linkages. These do not supply the shaders, but rather describe the pipeline that will be executed
	//	sb.setState(m_CxLinkage);
	//	sb.setState(m_UcLinkage);

	//	//recreate a resource table
	//	CreateShaderResourceTable();
	//	
	//	// Now we can bind the shaders to the state buffer & to the binders
	//	// When using a particular binder, we will also allocate memory on the draw command buffer for the shader resources
	//	if (m_HullShader.pShader)
	//	{
	//		// Set the shader on the state
	//		sb.setShader(m_HullShader.pShader);

	//		// Reset the binder with the shader to associate with
	//		m_HullBinder.reset().setShader(m_HullShader.pShader);

	//		if (m_HullBinder.getUserDataSizeInBytes())
	//		{
	//			// Allocate memory on the dcb for user data
	//			sce::Agc::ShRegister* pHullUserData = (sce::Agc::ShRegister*)dcb.allocateTopDown({ m_HullBinder.getUserDataSizeInBytes(), sce::Agc::Alignment::kRegister });
	//			m_HullBinder.setUserDataPointer(pHullUserData);

	//			// Set the Shader Resource Table (contains all the expected resources)
	//			m_HullShader.SRT.GenerateTableOnDCB(dcb);
	//			m_HullBinder.setUserSrtBuffer(m_HullShader.SRT.GetTable(), m_HullShader.SRT.GetSizeInDwords());

	//			// Since the blocks have already been allocated and filled with our shader resource of time the only
	//			// thing we have to do each draw is just insert the packets to set the indirect register arrays.
	//			dcb.setShRegistersIndirect(pHullUserData, m_HullBinder.getUserDataSizeInElements());
	//		}
	//	}
	//	if (m_PrimitiveShader.pShader)
	//	{
	//		// Set the shader on the state
	//		sb.setShader(m_PrimitiveShader.pShader);

	//		// Reset the binder with the shader to associate with
	//		m_PrimitiveBinder.reset().setShader(m_PrimitiveShader.pShader);

	//		if (m_PrimitiveBinder.getUserDataSizeInBytes())
	//		{
	//			// Allocate memory on the dcb for user data
	//			sce::Agc::ShRegister* pPrimitiveUserData = (sce::Agc::ShRegister*)dcb.allocateTopDown({ m_PrimitiveBinder.getUserDataSizeInBytes(), sce::Agc::Alignment::kRegister });
	//			m_PrimitiveBinder.setUserDataPointer(pPrimitiveUserData);

	//			// Set the Shader Resource Table (contains all the expected resources)
	//			m_PrimitiveShader.SRT.GenerateTableOnDCB(dcb);
	//			m_PrimitiveBinder.setUserSrtBuffer(m_PrimitiveShader.SRT.GetTable(), m_PrimitiveShader.SRT.GetSizeInDwords());

	//			// Since the blocks have already been allocated and filled with our shader resource of time the only
	//			// thing we have to do each draw is just insert the packets to set the indirect register arrays.
	//			dcb.setShRegistersIndirect(pPrimitiveUserData, m_PrimitiveBinder.getUserDataSizeInElements());
	//		}
	//	}
	//	if (m_PixelShader.pShader)
	//	{
	//		// Set the shader on the state
	//		sb.setShader(m_PixelShader.pShader);

	//		// Reset the binder with the shader to associate with
	//		m_PixelBinder.reset().setShader(m_PixelShader.pShader);

	//		if (m_PixelBinder.getUserDataSizeInBytes())
	//		{
	//			// Allocate memory on the dcb for user data
	//			sce::Agc::ShRegister* pPixelUserData = (sce::Agc::ShRegister*)dcb.allocateTopDown({ m_PixelBinder.getUserDataSizeInBytes(), sce::Agc::Alignment::kRegister });
	//			m_PixelBinder.setUserDataPointer(pPixelUserData);

	//			// Set the Shader Resource Table (contains all the expected resources)
	//			m_PixelShader.SRT.GenerateTableOnDCB(dcb);
	//			m_PixelBinder.setUserSrtBuffer(m_PixelShader.SRT.GetTable(), m_PixelShader.SRT.GetSizeInDwords());

	//			// Since the blocks have already been allocated and filled with our shader resource of time the only
	//			// thing we have to do each draw is just insert the packets to set the indirect register arrays.
	//			dcb.setShRegistersIndirect(pPixelUserData, m_PixelBinder.getUserDataSizeInElements());
	//		}
	//	}
	//}

	//void AGCRasterizationPipeline::Unbind()
	//{
	//	sce::Agc::Core::StateBuffer& sb = gAGCContext->GetStateBuffer();

	//	// We need to call postDraw on the components. We don't need to call postBatch on the
	//	// IndirectStageBinders since we didn't use them at all between draw calls.
	//	sb.postDraw();
	//}

	//void AGCRasterizationPipeline::Release()
	//{
	//}

	//void AGCRasterizationPipeline::SetVertexBuffer(AGCVertexBuffer* pVB, uint32_t vertexOffset)
	//{
	//	// TODO: I dont like this, find a better way
	//	//m_PrimitiveBinder.setBuffers(m_PrimitiveShader.SRT.GetVertexBufferSlot(), 1, &pVB->GetResource());

	//	/*if (vertexOffset != 0)
	//		m_PrimitiveBinder.setVertexOffset(vertexOffset);*/
	//}

	//void AGCRasterizationPipeline::LoadAllShaders()
	//{
	//	LoadShader(m_Desc.HullShader.FileName, m_HullShader);
	//	LoadShader(m_Desc.VertexShader.FileName, m_PrimitiveShader);		// All of these 3 are combined in the same stage! only provide one!
	//	LoadShader(m_Desc.DomainShader.FileName, m_PrimitiveShader);		//
	//	LoadShader(m_Desc.GeometryShader.FileName, m_PrimitiveShader);		//
	//	LoadShader(m_Desc.PixelShader.FileName, m_PixelShader);

	//	
	//	// Link our GS/PS ahead of time so that we don't have to do it every frame.
	//	SceError error = sce::Agc::Core::linkShaders(
	//		&m_CxLinkage,
	//		&m_UcLinkage,
	//		m_HullShader.pShader,
	//		m_PrimitiveShader.pShader,
	//		m_PixelShader.pShader,
	//		sce::Agc::UcPrimitiveType::Type::kTriList
	//	);
	//	SCE_AGC_ASSERT(error == SCE_OK);
	//}

	//void AGCRasterizationPipeline::CreateShaderResourceTable()
	//{
	//	// THIS IS REDUNDANT, WE COULD JUST COPPY DESCRIPTORS STRAIGHT INTO THE SRT AND THEN POINT TO THEM
	//	// BIND  UNBIND CONCEPT?
	//	// BIND ALL DESCRIPTORS
	//	// DRAW DRAW
	//	// SWAP DESCRIPTOR TABLE / DESCRIPTOR
	//	// DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW
	//	// UNBIND


	//	// POSSIBLE OPTIMIZATIONS
	//	// CONCEPT OF CONST RESOURCES 
	//	// PER FRAME
	//	// PER Instance
	//	// PER PerObject

	//	// UNSUPPPORTED: Root Constants -> Need to be an inlined CBV!
	//	// CBVs
	//	// SRVs
	//	// UAVs
	//	// Descriptor Tables

	//	// I do not want to allocate the SRTs!
	//	// What I need to do is to sort my input buffers based on the signature (kinda).
	//	// That way, when calling Bind(), I can create the SRT on the top-down DCB with my input resources.
	//	// It may be nice to create the SRTs on here to avoid expensive operations, but the problem will be (as always) frame resources.
	//	// In short:
	//	// 1) Create a system to match an input resource with a shader.Signature.param
	//	//    -> If it matches with any param, then hoorays
	//	//    -> If it does not match with any param, then assert(false)
	//	//    -> This system should also probably put the input resources in the Signature order for easy binding
	//	// 2) Create the SRTs on the top-down DCB and memcpy the step 1)
	//	// 3) Figure out the frame resource problem
	//	auto AddToShaderSRTBasedOnVisibility = [&](const ShaderResourceDesc& desc) -> void {
	//		switch (desc.ShaderVisibility)
	//		{
	//		case ShaderVisibility_All:
	//			m_HullShader.SRT.AddResource(desc);
	//			m_PrimitiveShader.SRT.AddResource(desc);
	//			m_PixelShader.SRT.AddResource(desc);
	//			break;
	//		case ShaderVisibility_HullShader:
	//			m_HullShader.SRT.AddResource(desc);
	//			break;
	//		case ShaderVisibility_VertexShader:
	//		case ShaderVisibility_DomainShader:
	//		case ShaderVisibility_GeometryShader:
	//			m_PrimitiveShader.SRT.AddResource(desc);
	//			break;
	//		case ShaderVisibility_PixelShader:
	//			m_PixelShader.SRT.AddResource(desc);
	//			break;
	//		default:
	//			SKTBD_LOG_ASSERT(false, "The input Shader Visibility for this resource is invalid. Make sure you are not using raytracing visibilities!");
	//			break;
	//		}
	//	};
	//	auto AddSamplerToShaderSRTBasedOnVisibility = [&](const SamplerDesc& desc) -> void {
	//		switch (desc.ShaderVisibility)
	//		{
	//		case ShaderVisibility_All:
	//			m_HullShader.SRT.AddSampler(desc);
	//			m_PrimitiveShader.SRT.AddSampler(desc);
	//			m_PixelShader.SRT.AddSampler(desc);
	//			break;
	//		case ShaderVisibility_HullShader:
	//			m_HullShader.SRT.AddSampler(desc);
	//			break;
	//		case ShaderVisibility_VertexShader:
	//		case ShaderVisibility_DomainShader:
	//		case ShaderVisibility_GeometryShader:
	//			m_PrimitiveShader.SRT.AddSampler(desc);
	//			break;
	//		case ShaderVisibility_PixelShader:
	//			m_PixelShader.SRT.AddSampler(desc);
	//			break;
	//		default:
	//			SKTBD_LOG_ASSERT(false, "The input Shader Visibility for this resource is invalid. Make sure you are not using raytracing visibilities!");
	//			break;
	//		}
	//	};

	//	for (const ShaderResourceDesc desc : m_Desc.vRootConstants)
	//	{
	//		SKTBD_LOG_ASSERT(false, "");
	//	}

	//	for (const ShaderResourceDesc& desc : m_Desc.vCBV)
	//		AddToShaderSRTBasedOnVisibility(desc);

	//	for (const ShaderResourceDesc& desc : m_Desc.vSRV)
	//		AddToShaderSRTBasedOnVisibility(desc);

	//	for (const ShaderResourceDesc& desc : m_Desc.vUAV)
	//		AddToShaderSRTBasedOnVisibility(desc);

	//	for (const ShaderResourceDesc& desc : m_Desc.vDescriptorTables)
	//		AddToShaderSRTBasedOnVisibility(desc);

	//	for (const SamplerDesc& desc : m_Desc.vStaticSamplers)
	//		AddSamplerToShaderSRTBasedOnVisibility(desc);

	//	m_HullShader.SRT.VerifyTable();
	//	m_PrimitiveShader.SRT.VerifyTable();
	//	m_PixelShader.SRT.VerifyTable();
	//}

	//void AGCRasterizationPipeline::ConfigurePipelineSetup()
	//{

	//	if (m_Desc.Wireframe){
	//		m_PrimitiveSetup.setFrontPolygonMode(sce::Agc::CxPrimitiveSetup::FrontPolygonMode::kLine);
	//	}
	//	else{
	//		m_PrimitiveSetup.setFrontPolygonMode(sce::Agc::CxPrimitiveSetup::FrontPolygonMode::kFill);
	//	}

	//	// TODO: Switch to using flags? Can describe pipeline in a more compact way...
	//	/*if (m_Desc.PipelineFlags & FillMode_Fill) {
	//		m_PrimitiveSetup.setFrontPolygonMode(sce::Agc::CxPrimitiveSetup::FrontPolygonMode::kFill);
	//	}
	//	else if (m_Desc.PipelineFlags & FillMode_Wireframe) {
	//		m_PrimitiveSetup.setFrontPolygonMode(sce::Agc::CxPrimitiveSetup::FrontPolygonMode::kLine);
	//	}
	//	else if (m_Desc.PipelineFlags & FillMode_Point) {
	//		m_PrimitiveSetup.setFrontPolygonMode(sce::Agc::CxPrimitiveSetup::FrontPolygonMode::kPoint);
	//	}

	//	if (m_Desc.PipelineFlags & CullMode_Front){
	//		if (m_Desc.PipelineFlags & WindingOrder_CCW) {
	//			m_PrimitiveSetup.setFrontFace(sce::Agc::CxPrimitiveSetup::FrontFace::kCcw);
	//		}
	//		else if (m_Desc.PipelineFlags & WindingOrder_CW) {
	//			m_PrimitiveSetup.setFrontFace(sce::Agc::CxPrimitiveSetup::FrontFace::kCw);
	//		}
	//		m_PrimitiveSetup.setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kBack);
	//	}
	//	else{
	//		if (m_Desc.PipelineFlags & CullMode_Back) {
	//			m_PrimitiveSetup.setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kFront);
	//		}
	//	}*/
	//	
	//}

	//AGCComputePipeline::AGCComputePipeline(const std::wstring& debugName, const ComputePipelineDesc& desc) :
	//	ComputePipeline(debugName, desc),
	//	m_ComputeShader{}
	//{
	//	// Load the shader
	//	LoadShader(m_Desc.ComputeShader.FileName, m_ComputeShader);

	//	// Create the shader resource table
	//	CreateShaderResourceTable();

	//	// Init binder
	//	m_Binder.init();
	//}

	//AGCComputePipeline::~AGCComputePipeline()
	//{
	//}

	//void AGCComputePipeline::Bind()
	//{
	//	// Get API objects
	//	sce::Agc::DrawCommandBuffer& dcb = gAGCContext->GetDrawCommandBuffer();
	//	sce::Agc::Core::StateBuffer& sb = gAGCContext->GetStateBuffer();

	//	// Set the shader
	//	sb.setShader(m_ComputeShader.pShader);
	//	m_Binder.reset().setShader(m_ComputeShader.pShader);

	//	// Allocate memory on the dcb for user data
	//	sce::Agc::ShRegister* pPixelUserData = (sce::Agc::ShRegister*)dcb.allocateTopDown({ m_Binder.getUserDataSizeInBytes(), sce::Agc::Alignment::kRegister });
	//	m_Binder.setUserDataPointer(pPixelUserData);

	//	// Set the Shader Resource Table (contains all the expected resources)
	//	m_ComputeShader.SRT.GenerateTableOnDCB(dcb);
	//	m_Binder.setUserSrtBuffer(m_ComputeShader.SRT.GetTable(), m_ComputeShader.SRT.GetSizeInDwords());

	//	// Since the blocks have already been allocated and filled with our shader resource of time the only
	//	// thing we have to do each draw is just insert the packets to set the indirect register arrays.
	//	dcb.setShRegistersIndirect(pPixelUserData, m_Binder.getUserDataSizeInElements());
	//}

	//void AGCComputePipeline::CreateShaderResourceTable()
	//{
	//	for (const ShaderResourceDesc desc : m_Desc.vRootConstants)
	//	{
	//		SKTBD_LOG_ASSERT(false, "");
	//	}

	//	for (const ShaderResourceDesc desc : m_Desc.vCBV)
	//		m_ComputeShader.SRT.AddResource(desc);

	//	for (const ShaderResourceDesc desc : m_Desc.vSRV)
	//		m_ComputeShader.SRT.AddResource(desc);

	//	for (const ShaderResourceDesc desc : m_Desc.vUAV)
	//		m_ComputeShader.SRT.AddResource(desc);

	//	for (const ShaderResourceDesc desc : m_Desc.vDescriptorTables)
	//		m_ComputeShader.SRT.AddResource(desc);

	//	for (const SamplerDesc desc : m_Desc.vStaticSamplers)
	//		m_ComputeShader.SRT.AddSampler(desc);
	//}

	//void AGCComputePipeline::Release()
	//{
	//}

	/*AGCRaytracingPipeline::AGCRaytracingPipeline(const std::wstring& debugName, const RaytracingPipelineDesc& desc) :
		RaytracingPipeline(debugName, desc)
	{
	}

	AGCRaytracingPipeline::~AGCRaytracingPipeline()
	{
	}

	void AGCRaytracingPipeline::Bind()
	{
	}

	void AGCRaytracingPipeline::Release()
	{
	}

	void AGCRaytracingPipeline::ResizeDispatchAndOutputUAV(uint32_t newWidth, uint32_t newHeight, uint32_t newDepth)
	{
	}*/
}