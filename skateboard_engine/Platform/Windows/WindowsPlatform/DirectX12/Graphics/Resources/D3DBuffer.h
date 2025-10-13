#pragma once
#include "Skateboard/Graphics/Resources/Buffer.h"
#include "D3D.h"
#include "D3D12MemoryAllocator/include/D3D12MemAlloc.h"

namespace Skateboard
{
	class D3DBuffer : public Buffer
	{
	public:
		DISABLE_COPY_AND_MOVE(D3DBuffer);

		D3D12MA::Allocation* m_BufferResource;

		D3D12_GPU_VIRTUAL_ADDRESS GetResourceGPUAddress() const { return m_BufferResource->GetResource()->GetGPUVirtualAddress(); }
		ID3D12Resource* GetResource() const { return m_BufferResource->GetResource(); }

		D3DBuffer(const BufferDesc& desc);
		D3DBuffer(const BufferDesc& desc, D3D12MA::Allocation* Existing_Allocation) : Buffer(desc) { m_BufferResource = Existing_Allocation; }

#ifndef SKTBD_SHIP
		void SetDebugName(const std::wstring& debug_name) override {
			GPUResource::SetDebugName(debug_name);
			m_BufferResource->SetName(debug_name.data());
			m_BufferResource->GetResource()->SetName(debug_name.data());
		};
#endif

		~D3DBuffer() override;
	};

	class D3DTextureBuffer : public TextureBuffer
	{
	public:
		DISABLE_COPY_AND_MOVE(D3DTextureBuffer);

		D3D12MA::Allocation* m_TextureResource;

		D3DTextureBuffer(const TextureDesc& desc);
		D3DTextureBuffer(const TextureDesc& desc, D3D12MA::Allocation* Existing_Allocation) : TextureBuffer(desc) { m_TextureResource = Existing_Allocation; };

		~D3DTextureBuffer() override;

		D3D12_GPU_VIRTUAL_ADDRESS GetResourceGPUAddress() const { return m_TextureResource->GetResource()->GetGPUVirtualAddress(); }
		ID3D12Resource* GetResource() const { return m_TextureResource->GetResource(); }

#ifndef SKTBD_SHIP
		void SetDebugName(const std::wstring& debug_name) override {
			GPUResource::SetDebugName(debug_name);
			m_TextureResource->SetName(debug_name.data());
			m_TextureResource->GetResource()->SetName(debug_name.data());
		};
#endif
	};


}
