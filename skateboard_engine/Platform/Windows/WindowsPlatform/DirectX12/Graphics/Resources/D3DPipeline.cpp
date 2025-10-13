#include "windowspch.h"
#include "D3DPipeline.h"
#include "Graphics/RHI/D3DGraphicsContext.h"
#include "Graphics/Resources/D3DBuffer.h"
#include "Graphics/API/D3DDescriptorTable.h"

#define SKTBD_LOG_COMPONENT "D3DPipeline"

namespace Skateboard
{
	auto ValidatePathNLoad(const wchar_t* Filename, IDxcBlobEncoding** Blob) -> bool
	{
		if (!Filename) return false;
		std::filesystem::path file(Filename);
		file.replace_extension(L".cso");

		auto abs_path = (std::filesystem::current_path() += L"\\") += file;

		if (std::filesystem::exists(abs_path))
		{
			gD3DContext->GetDxcUtils()->LoadFile(file.c_str(), nullptr, Blob);
			return true;
		}
		return false;
	};


	const char* TranslateSemantic(const VertexSemantic& semantic)
	{
		switch (semantic) {
		default:
		case POSITION:		return "POSITION";
		case NORMAL:		return "NORMAL";
		case TANGENT:		return "TANGENT";
		case COLOUR:		return "COLOR";
		case TEXCOORD:		return "TEXCOORD";
		case BONE_INDEX:	return "BLENDINDICES";
		case BONE_WEIGHTS:	return "BLENDWEIGHT";
		case CUSTOM:		return "CUSTOM";
		}
	}

	D3DShaderInputLayout::D3DShaderInputLayout(const ShaderInputLayoutDesc& desc) : ShaderInputLayout(desc)
	{
		std::vector<D3D12_ROOT_PARAMETER1> vRootParams(desc.vPipelineInputs.size());
		std::vector<D3D12_STATIC_SAMPLER_DESC1> vStaticSamplers(desc.vStaticSamplers.size());

		std::vector<D3D12_DESCRIPTOR_RANGE1> vDescriptorTables;

		//RootParams
		std::transform(desc.vPipelineInputs.begin(), desc.vPipelineInputs.end(), vRootParams.begin(), [&vDescriptorTables](const ShaderResourceDesc& desc) -> D3D12_ROOT_PARAMETER1
			{
				D3D12_ROOT_PARAMETER1 param = {};

				param.ShaderVisibility = ShaderVisibilityToD3D(desc.ShaderVisibility);

				switch (desc.ShaderElementType)
				{
				case ShaderElementType_RootConstant:
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
					param.Constants.Num32BitValues = desc.Constant.Num32BitValues;
					param.Constants.RegisterSpace = desc.Constant.RegisterSpace;
					param.Constants.ShaderRegister = desc.Constant.ShaderRegister;
					break;

				case ShaderElementType_ConstantBufferView:
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV;
					//		param.Descriptor.Flags =
					param.Descriptor.RegisterSpace = desc.Descriptor.RegisterSpace;
					param.Descriptor.ShaderRegister = desc.Descriptor.ShaderRegister;
					break;

				case ShaderElementType_ShaderResourceView:
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV;
					//		param.Descriptor.Flags =
					param.Descriptor.RegisterSpace = desc.Descriptor.RegisterSpace;
					param.Descriptor.ShaderRegister = desc.Descriptor.ShaderRegister;
					break;

				case ShaderElementType_UnorderedAccessView:
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_UAV;
					//		param.Descriptor.Flags =
					param.Descriptor.RegisterSpace = desc.Descriptor.RegisterSpace;
					param.Descriptor.ShaderRegister = desc.Descriptor.ShaderRegister;
					break;

				case ShaderElementType_DescriptorTable:
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					param.DescriptorTable.NumDescriptorRanges = desc.DescriptorTable.NumberOfRanges;
					param.DescriptorTable.pDescriptorRanges = &vDescriptorTables.back();

					for (uint32_t i = 0; i < desc.DescriptorTable.NumberOfRanges; i++)
					{
						auto& R = desc.DescriptorTable.Ranges[i];

						auto Convert = [](ShaderElementType_ type)->D3D12_DESCRIPTOR_RANGE_TYPE
							{
								switch (type)
								{
								case ShaderElementType_Unknown:
									SKTBD_MSG_ERROR("ON DX12 Only supported descriptor Element types are CBV,  SRV, UAV")
										break;
								case ShaderElementType_RootConstant:
									SKTBD_MSG_ERROR("ON DX12 Only supported descriptor Element types are CBV,  SRV, UAV")
										break;
								case ShaderElementType_ConstantBufferView:
									return D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
									break;
								case ShaderElementType_ShaderResourceView:
									return D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
									break;
								case ShaderElementType_UnorderedAccessView:
									return D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
									break;
								case ShaderElementType_DescriptorTable:
									SKTBD_MSG_ERROR("ON DX12 Only supported descriptor Element types are CBV,  SRV, UAV")
										break;
								case ShaderElementType_Sampler:
									return D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
									break;
								};

								return D3D12_DESCRIPTOR_RANGE_TYPE();
							};

						vDescriptorTables.push_back(
							{ .RangeType = Convert(R.DescriptorType),
									 .NumDescriptors = R.NumOfDescriptors,
									 .BaseShaderRegister = R.BaseRegister,
									 .RegisterSpace = R.RegisterSpace,
									 .Flags = D3D12_DESCRIPTOR_RANGE_FLAGS::D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE,
									 .OffsetInDescriptorsFromTableStart = R.OffsetInDescriptorsFromTableStart
							});
					}

					break;
				default:
					break;
				}

				return param;
			});

		//StaticSamplers
		std::transform(desc.vStaticSamplers.begin(), desc.vStaticSamplers.end(), vStaticSamplers.begin(), [](const SamplerSlotDesc& slot) -> D3D12_STATIC_SAMPLER_DESC1
			{
				D3D12_STATIC_SAMPLER_DESC1 SD{};

				SD.ShaderRegister = slot.ShaderRegister;
				SD.RegisterSpace = slot.ShaderRegisterSpace;
				SD.Filter = SamplerFilterToD3D(slot.SamplerDesc.Filter);
				SD.AddressU = SamplerModeToD3D(slot.SamplerDesc.ModeU);
				SD.AddressV = SamplerModeToD3D(slot.SamplerDesc.ModeV);
				SD.AddressW = SamplerModeToD3D(slot.SamplerDesc.ModeW);
				SD.MipLODBias = slot.SamplerDesc.MipMapLevelOffset;
				SD.MaxAnisotropy = slot.SamplerDesc.MaxAnisotropy;
				SD.ComparisonFunc = SamplerComparisonFunctionToD3D(slot.SamplerDesc.ComparisonFunction);
				SD.BorderColor = SamplerBorderColourToD3D(slot.SamplerDesc.BorderColour);
				SD.MinLOD = slot.SamplerDesc.MipMapMinSampleLevel;
				SD.MaxLOD = slot.SamplerDesc.MipMapMaxSampleLevel;
				SD.ShaderVisibility = ShaderVisibilityToD3D(slot.ShaderVisibility);
				SD.Flags = (D3D12_SAMPLER_FLAGS)slot.SamplerDesc.Flags;

				return SD;
			}
		);

		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
		flags |= (desc.DescriptorsDirctlyAddresssed) ? D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED : D3D12_ROOT_SIGNATURE_FLAG_NONE;
		flags |= (desc.CanUseInputAssembler) ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE;
		flags |= (desc.SamplersDirectlyAddressed) ? D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED : D3D12_ROOT_SIGNATURE_FLAG_NONE;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSigDesc;
		RootSigDesc.Init_1_2(RootSigDesc, vRootParams.size(), vRootParams.data(), vStaticSamplers.size(), vStaticSamplers.data(), flags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		auto err = D3DX12SerializeVersionedRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_2, &signature, &error);

		D3D_CHECK_FAILURE(err);

		D3D_CHECK_FAILURE(gD3DContext->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_RootSig.ReleaseAndGetAddressOf())));
	}

	D3DGraphicsPipeline::D3DGraphicsPipeline(const GraphicsPipelineDesc& desc, const ShaderInputLayout* layout)
	{
		CD3DX12_STATE_OBJECT_DESC SODesc;
		SODesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

		// Optional flag to allow state object additions
		auto pConfig = SODesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
		pConfig->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

		auto pGenericProgram = SODesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();

		if (desc.InputVertexLayout.GetElementCount())
		{
			// Define the building blocks for the program - the individual subobjects / shaders
			auto pIL = SODesc.CreateSubobject<CD3DX12_INPUT_LAYOUT_SUBOBJECT>();

			// Create the input layout based on the given description layout

			const BufferLayout& layout = desc.InputVertexLayout;

			for (const BufferElement& element : layout)
			{
				const DXGI_FORMAT format = ShaderDataTypeToD3D(element.DataType);
				pIL->AddInputLayoutElementDesc({ TranslateSemantic(element.Semantic), element.SemanticIndex, format, element.InputSlot , element.Offset, (element.AttributeType == PerVertexData) ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 0 });
			}

			pGenericProgram->AddSubobject(*pIL);
		}

		if (layout)
		{
			auto pRootSig = SODesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
			pRootSig->SetRootSignature(static_cast<const D3DShaderInputLayout*>(layout)->m_RootSig.Get());
		}

		std::array<IDxcBlobEncoding*, 7> Blobs = {}; //store blobs as a temp array as we need the for lifetime

		if (ValidatePathNLoad(desc.VertexShader.FileName, &Blobs[0]))
		{
			auto pVS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			CD3DX12_SHADER_BYTECODE bcVS(Blobs[0]->GetBufferPointer(), Blobs[0]->GetBufferSize());
			pVS->SetDXILLibrary(&bcVS);
			if (!desc.VertexShader.EntryPoint)
			{
				pVS->DefineExport(desc.VertexShader.FileName, L"*");
				pGenericProgram->AddExport(desc.VertexShader.FileName);
			}
			else pGenericProgram->AddExport(desc.VertexShader.EntryPoint);
		}
		if (ValidatePathNLoad(desc.HullShader.FileName, &Blobs[1]))
		{
			auto pHS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			CD3DX12_SHADER_BYTECODE bcHS(Blobs[1]->GetBufferPointer(), Blobs[1]->GetBufferSize());
			pHS->SetDXILLibrary(&bcHS);
			if (!desc.HullShader.EntryPoint)
			{
				pHS->DefineExport(desc.HullShader.FileName, L"*");
				pGenericProgram->AddExport(desc.HullShader.FileName);
			}
			else pGenericProgram->AddExport(desc.HullShader.EntryPoint);
		}
		if (ValidatePathNLoad(desc.DomainShader.FileName, &Blobs[2]))
		{
			auto pDS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			CD3DX12_SHADER_BYTECODE bcDS(Blobs[2]->GetBufferPointer(), Blobs[2]->GetBufferSize());
			pDS->SetDXILLibrary(&bcDS);
			if (!desc.DomainShader.EntryPoint)
			{
				pDS->DefineExport(desc.DomainShader.FileName, L"*");
				pGenericProgram->AddExport(desc.DomainShader.FileName);
			}
			else pGenericProgram->AddExport(desc.DomainShader.EntryPoint);
		}
		if (ValidatePathNLoad(desc.GeometryShader.FileName, &Blobs[3]))
		{
			auto pGS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			CD3DX12_SHADER_BYTECODE bcGS(Blobs[3]->GetBufferPointer(), Blobs[3]->GetBufferSize());
			pGS->SetDXILLibrary(&bcGS);
			if (!desc.GeometryShader.EntryPoint)
			{
				pGS->DefineExport(desc.GeometryShader.FileName, L"*");
				pGenericProgram->AddExport(desc.GeometryShader.FileName);
			}
			else pGenericProgram->AddExport(desc.GeometryShader.EntryPoint);
		}
		if (ValidatePathNLoad(desc.PixelShader.FileName, &Blobs[4]))
		{
			auto pPS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			CD3DX12_SHADER_BYTECODE bcPS(Blobs[4]->GetBufferPointer(), Blobs[4]->GetBufferSize());
			pPS->SetDXILLibrary(&bcPS);
			if (!desc.PixelShader.EntryPoint)
			{
				pPS->DefineExport(desc.PixelShader.FileName, L"*");
				pGenericProgram->AddExport(desc.PixelShader.FileName);
			}
			else pGenericProgram->AddExport(desc.PixelShader.EntryPoint);
		}

		if (ValidatePathNLoad(desc.MeshShader.FileName, &Blobs[5]))
		{
			auto pMs = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			CD3DX12_SHADER_BYTECODE bcMS(Blobs[5]->GetBufferPointer(), Blobs[5]->GetBufferSize());
			pMs->SetDXILLibrary(&bcMS);
			if (!desc.MeshShader.EntryPoint)
			{
				pMs->DefineExport(desc.MeshShader.FileName, L"*");
				pGenericProgram->AddExport(desc.MeshShader.FileName);
			}
			else pGenericProgram->AddExport(desc.MeshShader.EntryPoint);
		}

		if (ValidatePathNLoad(desc.AmplificationShader.FileName, &Blobs[6]))
		{
			auto pAS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			CD3DX12_SHADER_BYTECODE bcAS(Blobs[6]->GetBufferPointer(), Blobs[6]->GetBufferSize());
			pAS->SetDXILLibrary(&bcAS);
			if (!desc.MeshShader.EntryPoint)
			{
				pAS->DefineExport(desc.AmplificationShader.FileName, L"*");
				pGenericProgram->AddExport(desc.AmplificationShader.FileName);
			}
			else pGenericProgram->AddExport(desc.AmplificationShader.EntryPoint);
		}

		auto pRast = SODesc.CreateSubobject<CD3DX12_RASTERIZER_SUBOBJECT>();
		auto& RSconfig = desc.Rasterizer;

		// Describe the rasterizer state (how will objects be rasterized ?)
		D3D12_RASTERIZER_DESC rasterizerDesc = {};
		pRast->SetFillMode(RSconfig.Wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID);		// Specify which fill mode to use when rendering
		pRast->SetCullMode((D3D12_CULL_MODE)RSconfig.Cull);										// Specify which face to cull (none, front, back)
		pRast->SetFrontCounterClockwise(RSconfig.FrontCC);											// Determines if a triangle is front or back facing
		pRast->SetDepthBias(RSconfig.DepthBias);													// Depth value added to a given pixel
		pRast->SetDepthBiasClamp(RSconfig.DepthBiasClamp);											// Maximum depth bias for a given pixel
		pRast->SetSlopeScaledDepthBias(RSconfig.SlopeScaledDepthBias);								// Scalar on a given pixel's slope. More info on Depth Bias on MSDN
		pRast->SetDepthClipEnable(RSconfig.DepthClipEnable);										// Specifies whether or not to enable clipping based on distance
		pRast->SetForcedSampleCount(RSconfig.ForcedSampleCount);									// The sample count that is forced while UAV rendering or rasterizing. 0 is not forced
		pRast->SetConservativeRaster((RSconfig.ConservativeRasterEnable) ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);									// Identifies whether conservative rasterization is on or off

		pGenericProgram->AddSubobject(*pRast);

		auto pPrimitiveTopology = SODesc.CreateSubobject<CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT>();
		pPrimitiveTopology->SetPrimitiveTopologyType((D3D12_PRIMITIVE_TOPOLOGY_TYPE)desc.InputPrimitiveType);

		pGenericProgram->AddSubobject(*pPrimitiveTopology);

		if (desc.RenderTargetCount)
		{
			auto pRTFormats = SODesc.CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>();
			pRTFormats->SetNumRenderTargets(desc.RenderTargetCount);

			for (int renderTarget = 0; renderTarget < desc.RenderTargetCount; renderTarget++)
			{
				pRTFormats->SetRenderTargetFormat(renderTarget, SkateboardBufferFormatToD3D(desc.RenderTargetDataFormats[renderTarget]));
			}

			pGenericProgram->AddSubobject(*pRTFormats);
		}

		auto& Blendconfig = desc.Blend;

		if (Blendconfig.RTBlendConfigs[0].BlendEnable && desc.RenderTargetCount)
		{
			auto pBlend = SODesc.CreateSubobject<CD3DX12_BLEND_SUBOBJECT>();

			// Describe a blend state
			D3D12_BLEND_DESC blendDesc = {};
			pBlend->SetAlphaToCoverageEnable(Blendconfig.AlphaToCoverage);											// Specifies whether to use alpha-to-coverage as a multisampling technique when setting a pixel to a render target
			pBlend->SetIndependentBlendEnable(Blendconfig.IndependentBlendEnable);									// Specifies whether to enable independent blending in simultaneous render targets (FALSE only uses RenderTarget[0])#

			for (int renderTarget = 0; renderTarget < desc.RenderTargetCount; renderTarget++)
			{
				auto& config = desc.Blend.RTBlendConfigs[renderTarget];

				D3D12_RENDER_TARGET_BLEND_DESC rtdesc;
				rtdesc.BlendEnable = config.BlendEnable;
				rtdesc.BlendOp = (D3D12_BLEND_OP)config.BlendOp;
				rtdesc.BlendOpAlpha = (D3D12_BLEND_OP)config.BlendOpAlpha;
				rtdesc.DestBlend = (D3D12_BLEND)config.DestBlend;
				rtdesc.DestBlendAlpha = (D3D12_BLEND)config.DestBlendAlpha;
				rtdesc.LogicOp = (D3D12_LOGIC_OP)config.LogicOp;
				rtdesc.LogicOpEnable = config.LogicOpEnable;
				rtdesc.RenderTargetWriteMask = config.RenderTargetWriteMask;
				rtdesc.SrcBlend = (D3D12_BLEND)config.SrcBlend;
				rtdesc.SrcBlendAlpha = (D3D12_BLEND)config.SrcBlendAlpha;

				pBlend->SetRenderTarget(renderTarget, rtdesc);
			}

			pGenericProgram->AddSubobject(*pBlend);
		}

		auto& DSconfig = desc.DepthStencil;
		if (DSconfig.DepthEnable || DSconfig.StencilEnable)
		{
			auto pDepth = SODesc.CreateSubobject<CD3DX12_DEPTH_STENCIL2_SUBOBJECT>();

			pDepth->SetDepthEnable(DSconfig.DepthEnable);									// Specify whether to enable depth testing
			pDepth->SetDepthWriteMask (DSconfig.DepthWriteAll ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO);		// Enable or disable writing to sections or all of the depth testing buffer
			pDepth->SetDepthBoundsTestEnable(DSconfig.DepthBoundsTestEnable);
			pDepth->SetDepthFunc((D3D12_COMPARISON_FUNC)DSconfig.DepthFunc);				// Function to compare new depth data to existing depth data
			
			pDepth->SetStencilEnable(DSconfig.StencilEnable);								// Spicify whether to enable stencil testing
			

			D3D12_DEPTH_STENCILOP_DESC1 FrontDesc;

			FrontDesc.StencilDepthFailOp = (D3D12_STENCIL_OP)DSconfig.FrontFace.StencilDepthFailOp;
			FrontDesc.StencilFailOp = (D3D12_STENCIL_OP)DSconfig.FrontFace.StencilFailOp;
			FrontDesc.StencilFunc = (D3D12_COMPARISON_FUNC)DSconfig.FrontFace.StencilFunc;
			FrontDesc.StencilPassOp = (D3D12_STENCIL_OP)DSconfig.FrontFace.StencilPassOp;
			FrontDesc.StencilReadMask = DSconfig.FrontFace.StencilReadMask;							// Identify a portion of the depth-stencil buffer for reading stencil data
			FrontDesc.StencilWriteMask = DSconfig.FrontFace.StencilWriteMask;						// Identify a portion of the depth-stencil buffer for writing stencil data

			pDepth->SetFrontFace(FrontDesc);

			D3D12_DEPTH_STENCILOP_DESC1 BackDesc;
			BackDesc.StencilDepthFailOp = (D3D12_STENCIL_OP)DSconfig.BackFace.StencilDepthFailOp;
			BackDesc.StencilFailOp = (D3D12_STENCIL_OP)DSconfig.BackFace.StencilFailOp;
			BackDesc.StencilFunc = (D3D12_COMPARISON_FUNC)DSconfig.BackFace.StencilFunc;
			BackDesc.StencilPassOp = (D3D12_STENCIL_OP)DSconfig.BackFace.StencilPassOp;
			BackDesc.StencilReadMask = DSconfig.BackFace.StencilReadMask;							// Identify a portion of the depth-stencil buffer for reading stencil data
			BackDesc.StencilWriteMask = DSconfig.BackFace.StencilWriteMask;						// Identify a portion of the depth-stencil buffer for writing stencil data

			pDepth->SetBackFace(BackDesc);

			auto pDSFormat = SODesc.CreateSubobject<CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT>();
			pDSFormat->SetDepthStencilFormat(SkateboardBufferFormatToD3D(desc.DepthstencilTargetFormat));

			pGenericProgram->AddSubobject(*pDepth);
			pGenericProgram->AddSubobject(*pDSFormat);
		}


		if (desc.Rasterizer.MultisampleEnable)
		{
			auto pSampleMask = SODesc.CreateSubobject<CD3DX12_SAMPLE_MASK_SUBOBJECT>();
			auto pSampleDesc = SODesc.CreateSubobject<CD3DX12_SAMPLE_DESC_SUBOBJECT>();

			pSampleMask->SetSampleMask(desc.SampleMask);
			pSampleDesc->SetCount(desc.SampleCount);
			pSampleDesc->SetQuality(desc.SampleQuality);

			pGenericProgram->AddSubobject(*pSampleDesc);
			pGenericProgram->AddSubobject(*pSampleMask);
		}

		if(desc.TriangleStripCutValue)
		{
			auto pStripCutValue = SODesc.CreateSubobject<CD3DX12_IB_STRIP_CUT_VALUE_SUBOBJECT>();
			pStripCutValue->SetIBStripCutValue((D3D12_INDEX_BUFFER_STRIP_CUT_VALUE)desc.TriangleStripCutValue);

			pGenericProgram->AddSubobject(*pStripCutValue);
		}

		pGenericProgram->SetProgramName(L"GraphicsPipeline");

		D3D_CHECK_FAILURE(gD3DContext->GetDevice()->CreateStateObject(SODesc, IID_PPV_ARGS(&m_State)));

		ComPtr<ID3D12StateObjectProperties1> pSOProperties;
		D3D_CHECK_FAILURE(m_State->QueryInterface(IID_PPV_ARGS(&pSOProperties)));
		m_Program = { D3D12_PROGRAM_TYPE_GENERIC_PIPELINE, pSOProperties->GetProgramIdentifier(L"GraphicsPipeline") };

		//cleaning
		for(auto& Blob  : Blobs)
		{
			if(Blob)Blob->Release();
		}
	}

	D3DComputePipeline::D3DComputePipeline(const ComputePipelineDesc& desc, const ShaderInputLayout* layout)
	{
		CD3DX12_STATE_OBJECT_DESC SODesc;
		SODesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

		// Optional flag to allow state object additions
		auto pConfig = SODesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
		pConfig->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

		auto pRootSig = SODesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		pRootSig->SetRootSignature(static_cast<const D3DShaderInputLayout*>(layout)->m_RootSig.Get());

		// Define the building blocks for the program - the individual subobjects / shaders
		auto pCS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

		IDxcBlobEncoding* Blob;
		
		if (ValidatePathNLoad(desc.ComputeShader.FileName, &Blob))
		{
			CD3DX12_SHADER_BYTECODE bcCS(Blob->GetBufferPointer(), Blob->GetBufferSize());
			pCS->SetDXILLibrary(&bcCS);
			pCS->DefineExport(desc.ComputeShader.FileName, L"*");
		}
		else
		{
			SKTBD_MSG_ERROR("Failed to load {}", ToString(desc.ComputeShader.FileName).c_str())
		};

		auto pGenericProgram = SODesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
		pGenericProgram->SetProgramName(L"ComputePipeline");
		pGenericProgram->AddExport(desc.ComputeShader.FileName);

		D3D_CHECK_FAILURE(gD3DContext->GetDevice()->CreateStateObject(SODesc, IID_PPV_ARGS(&m_State)));

		ComPtr<ID3D12StateObjectProperties1> pSOProperties;
		D3D_CHECK_FAILURE(m_State->QueryInterface(IID_PPV_ARGS(&pSOProperties)));
		m_Program = { D3D12_PROGRAM_TYPE_GENERIC_PIPELINE, pSOProperties->GetProgramIdentifier(L"ComputePipeline") };

		//clean up
		if (Blob) Blob->Release();
	}

	D3DRaytracingPipeline::D3DRaytracingPipeline(const RaytracingPipelineDesc& m_Desc, ShaderInputLayout* layout)
	{
		ID3D12Device5* pDevice = gD3DContext->GetDevice();

		std::vector<D3D12_STATE_SUBOBJECT> subObjects;
		constexpr const uint32_t maxSubobjects = 5;
		subObjects.reserve(maxSubobjects);

		// Perform a few sanity checks here; there needs to be at least one raygen shader, and all
		SKTBD_LOG_ASSERT(m_Desc.RaytracingShaders.RayGenShaderEntryPoint != nullptr, "A Ray Generation Shader is mandatory for the execution of the pipeline. Ensure you identify your RayGen shader.");

		//// DXIL SUBOBJECT
		std::vector<D3D12_EXPORT_DESC> shaderExports;	// Each export defines the name of the individual shaders
		shaderExports.reserve(1u + m_Desc.RaytracingShaders.HitGroups.size() * 3u + m_Desc.RaytracingShaders.MissShaderEntryPoints.size());
		D3D12_EXPORT_DESC exportDesc = {};
		exportDesc.ExportToRename = nullptr; // TODO: If multiple shaders use identical function names, move it here instead and use the .Name field to give a unique identifier
		exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

		// Start by pushing the Ray Generation shader export
		exportDesc.Name = m_Desc.RaytracingShaders.RayGenShaderEntryPoint;
		shaderExports.push_back(exportDesc);

		// Then export all hit groups
		std::unordered_set<const wchar_t*> existingExports;
		for (const RaytracingHitGroup& group : m_Desc.RaytracingShaders.HitGroups)
		{
			if (group.IntersectionShaderEntryPoint != nullptr && !existingExports.contains(group.IntersectionShaderEntryPoint))
			{
				exportDesc.Name = group.IntersectionShaderEntryPoint;
				existingExports.insert(group.IntersectionShaderEntryPoint);
				shaderExports.push_back(exportDesc);
			}
			if (group.AnyHitShaderEntryPoint != nullptr && !existingExports.contains(group.AnyHitShaderEntryPoint))
			{
				exportDesc.Name = group.AnyHitShaderEntryPoint;
				existingExports.insert(group.AnyHitShaderEntryPoint);
				shaderExports.push_back(exportDesc);
			}
			if (group.ClosestHitShaderEntryPoint != nullptr && !existingExports.contains(group.ClosestHitShaderEntryPoint))
			{
				exportDesc.Name = group.ClosestHitShaderEntryPoint;
				existingExports.insert(group.ClosestHitShaderEntryPoint);
				shaderExports.push_back(exportDesc);
			}
		}

		// Export all miss shaders
		for (const wchar_t* missEntryPoint : m_Desc.RaytracingShaders.MissShaderEntryPoints)
		{
			if (existingExports.contains(missEntryPoint))
				continue;
			exportDesc.Name = missEntryPoint;
			existingExports.insert(missEntryPoint);
			shaderExports.push_back(exportDesc);
		}

		// Export all callable shaders
		for (const wchar_t* callableShader : m_Desc.RaytracingShaders.CallableShaders)
		{
			if (existingExports.contains(callableShader))
				continue;
			exportDesc.Name = callableShader;
			existingExports.insert(callableShader);
			shaderExports.push_back(exportDesc);
		}

		ComPtr<IDxcBlobEncoding> DIXLlib;
		D3D12_DXIL_LIBRARY_DESC libDesc = {};
		if (ValidatePathNLoad(m_Desc.RaytracingShaders.FileName, DIXLlib.ReleaseAndGetAddressOf()))
		{
			libDesc.pExports = shaderExports.data();
			libDesc.NumExports = static_cast<uint32_t>(shaderExports.size());
			libDesc.DXILLibrary = {
				DIXLlib->GetBufferPointer(),
				DIXLlib->GetBufferSize()
			};

			D3D12_STATE_SUBOBJECT dxilSubObject = {};
			dxilSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			dxilSubObject.pDesc = &libDesc;
			subObjects.emplace_back(std::move(dxilSubObject));
		};


		//// HIT GROUP SUBOBJECT
		std::vector<D3D12_HIT_GROUP_DESC> vHitGroupDescs;	// They need to be alive on pipeline creation!
		vHitGroupDescs.reserve(m_Desc.RaytracingShaders.HitGroups.size());
		for (const RaytracingHitGroup& group : m_Desc.RaytracingShaders.HitGroups)
		{
			// Describe the hit group (intersection shader, any hit shader, closest hit shader)
			// Note that intersection and any hit shaders are assigned to default (triangle) shaders when undefined (nullptr)
			D3D12_HIT_GROUP_DESC hitGroupDesc = {};
			hitGroupDesc.AnyHitShaderImport = group.AnyHitShaderEntryPoint;
			hitGroupDesc.ClosestHitShaderImport = group.ClosestHitShaderEntryPoint;
			hitGroupDesc.IntersectionShaderImport = group.IntersectionShaderEntryPoint;
			hitGroupDesc.HitGroupExport = group.HitGroupName;
			hitGroupDesc.Type = RaytracingHitGroupTypeToD3D(group.Type);
			vHitGroupDescs.emplace_back(std::move(hitGroupDesc));

			D3D12_STATE_SUBOBJECT hitGroupSubObject = {};
			hitGroupSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
			hitGroupSubObject.pDesc = &vHitGroupDescs.back();
			subObjects.emplace_back(std::move(hitGroupSubObject));
		}

		//// RAYTRACING SHADER CONFIG SUBOBJECT
		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
		shaderConfig.MaxPayloadSizeInBytes = m_Desc.MaxPayloadSize;
		shaderConfig.MaxAttributeSizeInBytes = m_Desc.MaxAttributeSize;

		D3D12_STATE_SUBOBJECT shaderConfigSubObject = {};
		shaderConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		shaderConfigSubObject.pDesc = &shaderConfig;
		subObjects.emplace_back(std::move(shaderConfigSubObject));

		//// LOCAL ROOT SIGNATURE SUBOBJECTS Writing to them would be a massiven criminellen painen in arsen, ps5 and differ a fair bit, tho if a binder was used as a local root signature....
		/*if (m_LocalRaygenRootSignature.Get())
		{
			D3D12_STATE_SUBOBJECT rayGenLocalRootSignatureSubObject = {};
			rayGenLocalRootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			rayGenLocalRootSignatureSubObject.pDesc = m_LocalRaygenRootSignature.GetAddressOf();
			subObjects.emplace_back(std::move(rayGenLocalRootSignatureSubObject));

			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION localRootSignatureRayGenAssiciation = {};
			localRootSignatureRayGenAssiciation.pSubobjectToAssociate = &subObjects.back();
			localRootSignatureRayGenAssiciation.NumExports = 1;
			localRootSignatureRayGenAssiciation.pExports = &m_Desc.RaytracingShaders.RayGenShaderEntryPoint;

			D3D12_STATE_SUBOBJECT localRootSignatureRayGenAssiciationSubObject = {};
			localRootSignatureRayGenAssiciationSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			localRootSignatureRayGenAssiciationSubObject.pDesc = &localRootSignatureRayGenAssiciation;
			subObjects.emplace_back(std::move(localRootSignatureRayGenAssiciationSubObject));
		}
		existingExports.clear();
		std::vector<const wchar_t*> vHitGroupExports;
		if (m_LocalHitGroupRootSignature.Get())
		{
			D3D12_STATE_SUBOBJECT hitgroupLocalRootSignatureSubObject = {};
			hitgroupLocalRootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			hitgroupLocalRootSignatureSubObject.pDesc = m_LocalHitGroupRootSignature.GetAddressOf();
			subObjects.emplace_back(std::move(hitgroupLocalRootSignatureSubObject));

			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION localRootSignatureHitgroupAssiciation = {};
			localRootSignatureHitgroupAssiciation.pSubobjectToAssociate = &subObjects.back();
			for (const RaytracingHitGroup& group : m_Desc.RaytracingShaders.HitGroups)
			{
				if (group.AnyHitShaderEntryPoint && !existingExports.count(group.AnyHitShaderEntryPoint))
				{
					existingExports.insert(group.AnyHitShaderEntryPoint);
					vHitGroupExports.push_back(group.AnyHitShaderEntryPoint);
				}
				if (group.ClosestHitShaderEntryPoint && !existingExports.count(group.ClosestHitShaderEntryPoint))
				{
					existingExports.insert(group.ClosestHitShaderEntryPoint);
					vHitGroupExports.push_back(group.ClosestHitShaderEntryPoint);
				}
				if (group.IntersectionShaderEntryPoint && !existingExports.count(group.IntersectionShaderEntryPoint))
				{
					existingExports.insert(group.IntersectionShaderEntryPoint);
					vHitGroupExports.push_back(group.IntersectionShaderEntryPoint);
				}
			}
			localRootSignatureHitgroupAssiciation.NumExports = static_cast<uint32_t>(vHitGroupExports.size());
			localRootSignatureHitgroupAssiciation.pExports = vHitGroupExports.data();

			D3D12_STATE_SUBOBJECT localRootSignatureHitgroupAssiciationSubObject = {};
			localRootSignatureHitgroupAssiciationSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			localRootSignatureHitgroupAssiciationSubObject.pDesc = &localRootSignatureHitgroupAssiciation;
			subObjects.emplace_back(std::move(localRootSignatureHitgroupAssiciationSubObject));
		}
		if (m_LocalMissRootSignature.Get())
		{
			D3D12_STATE_SUBOBJECT missLocalRootSignatureSubObject = {};
			missLocalRootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			missLocalRootSignatureSubObject.pDesc = m_LocalMissRootSignature.GetAddressOf();
			subObjects.emplace_back(std::move(missLocalRootSignatureSubObject));

			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION localRootSignatureMissAssiciation = {};
			localRootSignatureMissAssiciation.pSubobjectToAssociate = &subObjects.back();
			localRootSignatureMissAssiciation.NumExports = static_cast<uint32_t>(m_Desc.RaytracingShaders.MissShaderEntryPoints.size());
			localRootSignatureMissAssiciation.pExports = m_Desc.RaytracingShaders.MissShaderEntryPoints.data();

			D3D12_STATE_SUBOBJECT localRootSignatureMissAssiciationSubObject = {};
			localRootSignatureMissAssiciationSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			localRootSignatureMissAssiciationSubObject.pDesc = &localRootSignatureMissAssiciation;
			subObjects.emplace_back(std::move(localRootSignatureMissAssiciationSubObject));
		}
		if (m_LocalCallableRootSignature.Get())
		{
			D3D12_STATE_SUBOBJECT callableLocalRootSignatureSubObject = {};
			callableLocalRootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			callableLocalRootSignatureSubObject.pDesc = m_LocalCallableRootSignature.GetAddressOf();
			subObjects.emplace_back(std::move(callableLocalRootSignatureSubObject));

			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION localRootSignatureCallableAssiciation = {};
			localRootSignatureCallableAssiciation.pSubobjectToAssociate = &subObjects.back();
			localRootSignatureCallableAssiciation.NumExports = static_cast<uint32_t>(m_Desc.RaytracingShaders.CallableShaders.size());
			localRootSignatureCallableAssiciation.pExports = m_Desc.RaytracingShaders.CallableShaders.data();

			D3D12_STATE_SUBOBJECT localRootSignatureCallableAssiciationSubObject = {};
			localRootSignatureCallableAssiciationSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			localRootSignatureCallableAssiciationSubObject.pDesc = &localRootSignatureCallableAssiciation;
			subObjects.emplace_back(std::move(localRootSignatureCallableAssiciationSubObject));
		}*/

		//// GLOBAL ROOT SIGNATURE SUBOBJECT
		D3D12_STATE_SUBOBJECT globalRootSignatureSubObject = {};
		globalRootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		globalRootSignatureSubObject.pDesc = static_cast<D3DShaderInputLayout*>(layout)->m_RootSig.GetAddressOf();
		subObjects.emplace_back(std::move(globalRootSignatureSubObject));

		//// RAYTRACING PIPELINE SUBOBJECT
		D3D12_RAYTRACING_PIPELINE_CONFIG1 pipelineConfig = {};
		pipelineConfig.MaxTraceRecursionDepth = m_Desc.MaxTraceRecursionDepth;
		pipelineConfig.Flags = (D3D12_RAYTRACING_PIPELINE_FLAGS)m_Desc.Flags;

		D3D12_STATE_SUBOBJECT pipelineConfigSubObject = {};
		pipelineConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
		pipelineConfigSubObject.pDesc = &pipelineConfig;
		subObjects.emplace_back(std::move(pipelineConfigSubObject));

		// Sanity check - ?????
		SKTBD_LOG_ASSERT(subObjects.size() <= maxSubobjects,
			"Subobject vector reallocated memory and is now corrupted. I've not tried this before, but it may crash if it didnt copy on associations. Let's see it for science!!");

		//// PIPELINE STATE OBJECT
		// Create the state object.
		D3D12_STATE_OBJECT_DESC raytracingPipeline = {};
		raytracingPipeline.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		raytracingPipeline.NumSubobjects = static_cast<uint32_t>(subObjects.size());
		raytracingPipeline.pSubobjects = subObjects.data();

		ComPtr<ID3D12StateObject> State;
		D3D_CHECK_FAILURE(pDevice->CreateStateObject(&raytracingPipeline, IID_PPV_ARGS(State.ReleaseAndGetAddressOf())));
		m_State = State;

		/*ComPtr<ID3D12StateObjectProperties1> pSOProperties;
		D3D_CHECK_FAILURE(m_State->QueryInterface(IID_PPV_ARGS(&pSOProperties)));
		m_Identifier = pSOProperties->GetProgram(L"myGenericProgram");*/

		// When using callable shaders, we need to ensure that the stack size is set to a sensible value.
		// The default stack size defined internally by D3D may not be sufficient depending on the recursions with callable shaders.
		// See: https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#pipeline-stack
		// Here we'll use the same calulcation they presented but simply replace the recursion depth with our internal value

		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties1> rtpsoProperties;
		D3D_CHECK_FAILURE(State.As(&rtpsoProperties));	// Get the RTPSO properties
		if (!m_Desc.RaytracingShaders.CallableShaders.empty() && m_Desc.MaxCallableShaderRecursionDepth > 2u)
		{
			const UINT64 RGSMax = rtpsoProperties->GetShaderStackSize(m_Desc.RaytracingShaders.RayGenShaderEntryPoint);
			UINT64 ISMax = 0u, AHSMax = 0u, CHSMax = 0u, MSMax = 0u, CSMax = 0u;
			for (const RaytracingHitGroup& hitGroup : m_Desc.RaytracingShaders.HitGroups)
			{
				std::wstring identifier(hitGroup.HitGroupName);
				identifier.append(L"::");
				if (hitGroup.IntersectionShaderEntryPoint)	ISMax = std::max(ISMax, rtpsoProperties->GetShaderStackSize((identifier + std::wstring(L"intersection")).data()));
				if (hitGroup.AnyHitShaderEntryPoint)			AHSMax = std::max(AHSMax, rtpsoProperties->GetShaderStackSize((identifier + std::wstring(L"anyhit")).data()));
				if (hitGroup.ClosestHitShaderEntryPoint)		CHSMax = std::max(CHSMax, rtpsoProperties->GetShaderStackSize((identifier + std::wstring(L"closesthit")).data()));
			}
			for (const wchar_t* missShaderEntryPoint : m_Desc.RaytracingShaders.MissShaderEntryPoints)
				MSMax = std::max(MSMax, rtpsoProperties->GetShaderStackSize(missShaderEntryPoint));
			for (const wchar_t* callableShader : m_Desc.RaytracingShaders.CallableShaders)
				CSMax = std::max(CSMax, rtpsoProperties->GetShaderStackSize(callableShader));

			const UINT64 stackSize = RGSMax
				+ std::max(CHSMax, std::max(MSMax, ISMax + AHSMax)) * std::min(1u, m_Desc.MaxTraceRecursionDepth)
				+ std::max(CHSMax, MSMax) * std::max(m_Desc.MaxTraceRecursionDepth - 1u, 0u)
				+ m_Desc.MaxCallableShaderRecursionDepth * CSMax;
			rtpsoProperties->SetPipelineStackSize(stackSize);
		}
		m_Program = { D3D12_PROGRAM_TYPE_RAYTRACING_PIPELINE, rtpsoProperties->GetProgramIdentifier(L"") };
	}

	D3DSamplerState::D3DSamplerState(const SamplerDesc& desc)
	{
		D3D12_SAMPLER_DESC2 SD{};

		SD.Filter = SamplerFilterToD3D(desc.Filter);
		SD.AddressU = SamplerModeToD3D(desc.ModeU);
		SD.AddressV = SamplerModeToD3D(desc.ModeV);
		SD.AddressW = SamplerModeToD3D(desc.ModeW);
		SD.MipLODBias = desc.MipMapLevelOffset;
		SD.MaxAnisotropy = desc.MaxAnisotropy;
		SD.ComparisonFunc = SamplerComparisonFunctionToD3D(desc.ComparisonFunction);
		SD.Flags = (D3D12_SAMPLER_FLAGS)desc.Flags;

		switch (desc.BorderColour) {
		case SamplerBorderColour_TransparentBlack:
			if(desc.Flags & SamplerFlags_INTEGER_SAMPLING)
			{
				SD.UintBorderColor[0] = 0;
				SD.UintBorderColor[1] = 0;
				SD.UintBorderColor[2] = 0;
				SD.UintBorderColor[3] = 0;
			}
			else
			{
				SD.FloatBorderColor[0] = 0.f;
				SD.FloatBorderColor[1] = 0.f;
				SD.FloatBorderColor[2] = 0.f;
				SD.FloatBorderColor[3] = 0.f;
			}
			break;
		case SamplerBorderColour_White:
			if (desc.Flags & SamplerFlags_INTEGER_SAMPLING)
			{
				SD.UintBorderColor[0] = 1;
				SD.UintBorderColor[1] = 1;
				SD.UintBorderColor[2] = 1;
				SD.UintBorderColor[3] = 1;
			}
			else
			{
				SD.FloatBorderColor[0] = 1.f;
				SD.FloatBorderColor[1] = 1.f;
				SD.FloatBorderColor[2] = 1.f;
				SD.FloatBorderColor[3] = 1.f;
			}
			break;
		case SamplerBorderColour_Black:
			if (desc.Flags & SamplerFlags_INTEGER_SAMPLING)
			{
				SD.UintBorderColor[0] = 0;
				SD.UintBorderColor[1] = 0;
				SD.UintBorderColor[2] = 0;
				SD.UintBorderColor[3] = 1;
			}
			else
			{
				SD.FloatBorderColor[0] = 0.f;
				SD.FloatBorderColor[1] = 0.f;
				SD.FloatBorderColor[2] = 0.f;
				SD.FloatBorderColor[3] = 1.f;
			}
			break;
		}

		SD.MinLOD = desc.MipMapMinSampleLevel;
		SD.MaxLOD = desc.MipMapMaxSampleLevel;

		m_SamplerDescriptor = gD3DContext->GetSamplerDescriptorHeap().Allocate();

		gD3DContext->GetDevice()->CreateSampler2(&SD, m_SamplerDescriptor.GetCPUHandle());
	}

	D3DSamplerState::~D3DSamplerState()
	{
		gD3DContext->GetSamplerDescriptorHeap().Free(m_SamplerDescriptor);
	}
}
