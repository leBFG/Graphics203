#include <sktbdpch.h>
#include "ShaderResourceTable.h"
#include "AGCBuffer.h"
#include "AGCPipeline.h"
#include "AGC/Memory/AGCDescriptorTable.h"

#define SKTBD_LOG_COMPONENT "PS5_SRT_IMPL"
#include "Skateboard/Log.h"

namespace Skateboard
{
	//ShaderResourceTable::ShaderResourceTable() :
	//	m_ShaderSignature{},
	//	p_ResourceTable(nullptr),
	//	m_VertexBufferSlot(0u)
	//{
	//}

	//ShaderResourceTable::~ShaderResourceTable()
	//{
	//}

	//void ShaderResourceTable::AddResource(const ShaderResourceDesc& desc)
	//{
	//	// Find a corresponding parameter in the SRT Signature and assign the input resource to the corresponding index
	//	// Note: If an input resource desc is given but no match is found here it will be discarded. This should mimic
	//	// the behaviour of D3D12 where when a resource is visible to all stages but unused in some of them, nothing bad
	//	// happens and the program runs
	//	const size_t argCount = m_ShaderSignature.vParameters.size();
	//	for (size_t i = 0u; i < argCount; ++i)
	//	{
	//		SignatureParameter& param = m_ShaderSignature.vParameters[i];

	//		const bool isMatch =
	//			param.RegisterSpace == desc.Descriptor.RegisterSpace &&
	//			param.RegisterSlot == desc.Descriptor.ShaderRegister &&
	//			param.ElementType == desc.ShaderElementType;

	//		if (isMatch)
	//		{
	//			if (param.ElementType == ShaderElementType_DescriptorTable)
	//			{
	//				param.pDescriptorTable = desc.Descriptor.;
	//			}
	//			else 
	//			{
	//				param.pResource = desc.pResource;
	//			}

	//			if (param.ElementType == ShaderElementType_ShaderResourceView)
	//				m_VertexBufferSlot++;

	//			return;
	//		}
	//	}
	//}

	//void ShaderResourceTable::AddSampler(const SamplerDesc& desc)
	//{

	//	// Find a corresponding parameter in the SRT Signature and assign the input resource to the corresponding index
	//	// Note: If an input resource desc is given but no match is found here it will be discarded. This should mimic
	//	// the behaviour of D3D12 where when a resource is visible to all stages but unused in some of them, nothing bad
	//	// happens and the program runs
	//	/*const size_t argCount = m_ShaderSignature.vParameters.size();
	//	for (size_t i = 0u; i < argCount; ++i)
	//	{
	//		SignatureParameter& param = m_ShaderSignature.vParameters[i];

	//		const bool isMatch =
	//			param.RegisterSpace == desc. &&
	//			param.RegisterSlot == desc.ShaderRegister &&
	//			param.ElementType == ShaderElementType_Sampler;

	//		if (isMatch)
	//		{
	//			param.Sampler.init().setWrapMode(SamplerModeToAGC(desc.Mode)).setBorderColor(SamplerBorderColourToAGC(desc.BorderColour));
	//			SetFilterMode(param.Sampler, desc.Filter, desc.ComparisonFunction);
	//			return;
	//		}
	//	}*/
	//}

	//void ShaderResourceTable::VerifyTable() const
	//{
	//	const size_t argCount = m_ShaderSignature.vParameters.size();
	//	for (size_t i = 0u; i < argCount; ++i)
	//	{
	//		SKTBD_LOG_ASSERT(m_ShaderSignature.vParameters[i].pResource != nullptr, "The SRT Signature defined more resources than what has been provided.");
	//	}
	//}

	//void ShaderResourceTable::GenerateTableOnDCB(sce::Agc::DrawCommandBuffer& dcb)
	//{
	//	// Ensure this table contains data
	//	SKTBD_LOG_ASSERT(!m_ShaderSignature.vParameters.empty(), "An empty SRT cannot be built on a dcb. Is it possible that the shader expected no resources?");

	//	// Allocate memory for the table on the dcb so that the resource can be supplied
	//	// We will store the allocation in a member variable so it can be retrieved later, but it won't need to be freed!!
	//	p_ResourceTable = dcb.allocateTopDown(m_ShaderSignature.SizeInBytes, alignof(uint8_t));

	//	// Supply the resources
	//	for (size_t i = 0u; i < m_ShaderSignature.vParameters.size(); ++i)
	//	{
	//		const SignatureParameter& param = m_ShaderSignature.vParameters[i];
	//		uint8_t* pCurrentResource = static_cast<uint8_t*>(p_ResourceTable) + param.MemberOffset;

	//		// I have to copy the sce::Agc::Core::Buffers
	//		// For descriptor tables, I have to allocate top down to a pointer for the size of the table and then copy the data
	//		switch (param.ElementType)
	//		{
	//		case ShaderElementType_RootConstant:
	//			break;
	//		case ShaderElementType_ConstantBufferView:
	//			//memcpy(pCurrentResource, param.pResource->GetDefaultDescriptor(), param.MemberSize);
	//			break;
	//		case ShaderElementType_ShaderResourceView:
	//			switch (param.pResource->GetResourceType())
	//			{
	////			case GPUResourceType_DefaultBuffer:
	////				memcpy(pCurrentResource, param.pResource->GetResource(), param.MemberSize);
	////				break;
	////			case GPUResourceType_UploadBuffer:
	////				memcpy(pCurrentResource, param.pResource->GetResource(), param.MemberSize);
	////				break;
	////			case GPUResourceType_UnorderedAccessBuffer:
	////				memcpy(pCurrentResource, param.pResource->GetResource(), param.MemberSize);
	////				break;
	//			//case GPUResourceType_FrameBuffer:
	//			//	memcpy(pCurrentResource, param.pResource->GetDefaultDescriptor(), param.MemberSize);
	//			//	break;
	//			//case GPUResourceType_Texture2D:
	//			//	memcpy(pCurrentResource, param.pResource->GetDefaultDescriptor(), param.MemberSize);
	//			//	break;
	//			//case GPUResourceType_Texture3D:
	//				// TODO
	//				break;
	//			default:
	//				SKTBD_LOG_ASSERT(false, "Impossible binding, shader element type is corrupted or unsupported in this pipeline.");
	//				break;
	//			}
	//			break;
	//		case ShaderElementType_UnorderedAccessView:
	//			//memcpy(pCurrentResource, param.pResource->GetDefaultDescriptor(), param.MemberSize);
	//			break;
	//		case ShaderElementType_DescriptorTable:
	//			{
	//			// A descriptor table is just a pointer
	//			// So we need to allocate top down on the DCB and copy the table
	//			const SizedPtr& table = static_cast<AGCDescriptorTable*>(param.pDescriptorTable)->GetTable();
	//			void* entry = dcb.allocateTopDown(table.Size, alignof(uint8_t));
	//			memcpy(entry, table.Ptr, table.Size);
	//			// Then we need to copy the the pointer itself (not what it points to) to the SRT
	//			memcpy(pCurrentResource, &entry, param.MemberSize);
	//			}
	//			break;
	//		case ShaderElementType_Sampler:
	//			memcpy(pCurrentResource, &param.Sampler, param.MemberSize);
	//			break;
	//		default:
	//			SKTBD_LOG_ASSERT(false, "Impossible binding, shader element type is corrupted.");
	//			break;
	//		}
	//	}
	//}
	/*void ShaderResourceTable::EmptyTable()
	{
		
	}*/
}
