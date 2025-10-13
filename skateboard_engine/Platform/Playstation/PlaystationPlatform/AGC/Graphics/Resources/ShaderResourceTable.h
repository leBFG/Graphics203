#pragma once
#include "AGC/Graphics/AGCF.h"
#include "Skateboard/Graphics/InternalFormats.h"
#include <vector>

namespace Skateboard
{
	class GPUResource;

	class DescriptorTable;
	struct ShaderResourceDesc;
	struct SamplerDesc;

	struct SignatureParameter
	{
		int32_t MemberOffset;
		int32_t MemberSize;
		ShaderElementType_ ElementType;
		ShaderVisibility_ Visibility;
		//int32_t RegisterSlot;
		//int32_t RegisterSpace;
		//union
		//{
		//	GPUResource* pResource;
		//	DescriptorTable* pDescriptorTable;
		//	sce::Agc::Core::Sampler Sampler;
		//};
	};

	struct SrtSignature
	{
		std::vector<SignatureParameter> vParameters;
		uint32_t SizeInBytes;
	};

	//class ShaderResourceTable
	//{
	//}

	//class ShaderResourceTable
	//{
	//public:
	//	ShaderResourceTable();
	//	~ShaderResourceTable();

	//	SrtSignature& SignatureRef() { return m_ShaderSignature; }
	//	void* GetTable() const { return p_ResourceTable; }
	//	uint32_t GetSizeInDwords() const { return m_ShaderSignature.SizeInBytes / sizeof(uint32_t); }

	//	void AddResource(const ShaderResourceDesc& desc);
	//	void AddSampler(const SamplerDesc& desc);
	//	void VerifyTable() const;
	//	void GenerateTableOnDCB(sce::Agc::DrawCommandBuffer& dcb);
	//	void EmptyTable();

	//	uint32_t GetVertexBufferSlot() const { return m_VertexBufferSlot; }

	//private:
	//	SrtSignature m_ShaderSignature;
	//	void* p_ResourceTable;
	//	uint32_t m_VertexBufferSlot;
	//};

	//struct SRTSIGNATURE;

	//struct SRTPARAM
	//{
	//	union
	//	{
	//		int i = sizeof(sce::Agc::Core::Buffer);// m_descriptor;
	//	};
	//};

	//struct SRTSIGNATURE
	//{
	//	std::vector<SRTPARAM> vParameters;
	//	uint32_t SizeInBytes;
	//}; 
}