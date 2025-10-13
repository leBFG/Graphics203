#pragma once

#include "sktbdpch.h"

#include "AGC/Graphics/AGCTypes.h"
#include "AGC/Memory/AGCMemoryAllocator.h"
#include "Skateboard/Graphics/Resources/Buffer.h"

namespace Skateboard
{
	class AGCTextureBuffer final : public TextureBuffer
	{
	public:
		explicit AGCTextureBuffer(const TextureDesc& desc);

		explicit AGCTextureBuffer(const TextureDesc& desc, const RawMemoryHandle& Mem);
		void SetDebugName(const std::wstring& debug_name) override;

		RawMemoryHandle TextureMemory;

		union
		{
			sce::Agc::Core::Texture TextureDescriptor;
			sce::Agc::CxRenderTarget RenderTarget;
			sce::Agc::CxDepthRenderTarget DepthRenderTarget;
		};
	};

	class AGCBuffer final : public Buffer
	{
	public:
		explicit AGCBuffer(const BufferDesc& desc);
		explicit AGCBuffer(const BufferDesc& desc, const RawMemoryHandle& Mem);
		void SetDebugName(const std::wstring& debug_name) override;

		RawMemoryHandle BufferMemory;
		sce::Agc::Core::Buffer BufferDescriptor;

	private:
		float4 pad;
	};

	//class AGCIndirectArgumentBuffer final : public IndirectArgumentBuffer
	//{
	//public:
	//	AGCIndirectArgumentBuffer(const std::wstring& debugName, const IndirectArgumentBufferDesc& desc);

	//	void* GetIndirectArgumentBuffer() final override;

	//	//void Release() final override;
	//	//const void* GetDefaultDescriptor() final override { return m_IndexIndirectArgs; };
	//	//const void* GetResourceMemory() final override { return m_ArgumentBufferHandle.data; };

	//private:
	//	MemoryHandle<sce::Agc::DrawIndexIndirectArgs,sce::Agc::Alignment::kIndirectArgs> m_ArgumentBufferHandle;
	//	sce::Agc::DrawIndexIndirectArgs* m_IndexIndirectArgs;
	//};


	//class AGCTexture final : public Texture
	//{
	//public:
	//	AGCTexture(const std::wstring& debugName, const TextureDesc& desc);
	//	AGCTexture(const std::wstring& debugName, const TextureDesc& desc, sce::Agc::Core::Texture texture);
	//	AGCTexture(const std::wstring& debugName, const TextureDesc& desc, sce::Agc::Core::Texture texture, MemoryHandle<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment> texturememory);
	//	virtual ~AGCTexture() final override;

	//	const void* GetDefaultDescriptor() final override { return &m_Texture; }
	//	const void* GetResourceMemory() final override { return m_ResourceHandle.data; }

	//	void* GetTexture() const override;

	//	void Release() override;

	//private:
	//	sce::Agc::Core::Texture m_Texture;
	//	MemoryHandle<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment> m_ResourceHandle;
	//};

	//class AGCIndexBuffer final : public IndexBuffer
	//{
	//public:
	//	AGCIndexBuffer(const std::wstring& debugName, uint32_t* indices, uint32_t count);
	//	virtual ~AGCIndexBuffer() final override;

	//	virtual void Bind() const final override;
	//	virtual void Unbind() const final override;

	//	virtual Buffer* GetBuffer() const final override { return nullptr; };

	//	void* GetIndexBuffer() { return m_IndexBuffer.data; }

	//	void Release() override;

	//private:
	//	MemoryHandle<uint32_t, sce::Agc::Alignment::kBuffer> m_IndexBuffer;
	//};

	//class AGCVertexBuffer final : public VertexBuffer
	//{
	//public:
	//	AGCVertexBuffer(const std::wstring& debugName, void* vertices, uint32_t count, const BufferLayout& layout);
	//	virtual ~AGCVertexBuffer() final override;

	//	virtual void Bind() const final override;
	//	virtual void Unbind() const final override;

	//	virtual Buffer* GetBuffer() const final override { return nullptr; }
	//	const sce::Agc::Core::Buffer& GetResource() const { return m_Resource; }

	//	void Release() final override;

	//private:
	//	MemoryHandle<uint32_t, sce::Agc::Alignment::kBuffer> m_ResourceHandle;
	//	sce::Agc::Core::Buffer m_Resource;
	//};

	//class AGCFrameBuffer final : public FrameBuffer
	//{
	//public:
	//	AGCFrameBuffer(const std::wstring& debugName, const FrameBufferDesc& desc);
	//	virtual ~AGCFrameBuffer() final override;

	//	virtual void Bind(uint32_t renderTargetIndex) const final override;
	//	virtual void Unbind() const final override;

	//	virtual void Resize(uint32_t newWidth, uint32_t newHeight) final override;
	//	virtual float GetAspectRatio() const final override { return 0; };

	//	virtual void* GetRenderTargetAsImGuiTextureID(uint32_t renderTargetIndex) const final override;
	//	virtual void* GetDepthStencilTargetAsImGuiTextureID(uint32_t renderTargetIndex) const final override;

	//	//void ReleaseMemory() final override;
	//	//const void* GetDefaultDescriptor() final override;
	//	const void* GetResourceMemory() final override;

	//	

	//private:
	//	void CreateRenderTarget(TemplatedGPUMemoryPoolAllocator* allocator);
	//	void CreateDepthRenderTarget(TemplatedGPUMemoryPoolAllocator* allocator);

	//private:
	//	std::vector<MemoryHandle<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment>> v_RenderTargetResourceHandles;
	//	MemoryHandle<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment> m_DepthResourceHandle;
	//	MemoryHandle<uint8_t, sce::Agc::Alignment::kMaxTiledAlignment> m_DepthHTileHandle; // HTILE baseAddress must be aligned to 0 bytes

	//	sce::Agc::CxViewport m_Viewport;
	//	std::vector<sce::Agc::CxRenderTarget> v_RenderTargets;

	//	mutable std::vector<sce::Agc::Core::Texture> v_RenderTargetAsTextures;
	//	mutable sce::Agc::Core::Sampler m_RenderTextureSampler;

	//	sce::Agc::CxDepthRenderTarget m_DepthStencilTarget;
	//	sce::Agc::CxRenderTargetMask m_RenderTargetMask;
	//	sce::Agc::CxDepthStencilControl m_DepthStencilControl;
	//	mutable sce::Agc::Core::Texture m_DepthStencilTargetAsTexture;
	//};

	//class AGCBottomLevelAccelerationStructure final : public BottomLevelAccelerationStructure
	//{
	//public:
	//	AGCBottomLevelAccelerationStructure(const std::wstring& debugName, const BottomLevelAccelerationStructureDesc& desc);
	//	virtual ~AGCBottomLevelAccelerationStructure() final override;

	//	//void Release() final override;
	//	//const void* GetDefaultDescriptor() final override { return &m_Resource; };
	//	const void* GetResourceMemory() final override { return m_ResourceHandle.data; };

	//private:
	//	sce::Psr::BottomLevelBvhDescriptor m_Resource;
	//	MemoryHandle<uint8_t, sce::Psr::kRequiredAlignment> m_ResourceHandle;
	//};

	//class AGCTopLevelAccelerationStructure final : public TopLevelAccelerationStructure
	//{
	//public:
	//	AGCTopLevelAccelerationStructure(const std::wstring& debugName, const TopLevelAccelerationStructureDesc& desc);
	//	virtual ~AGCTopLevelAccelerationStructure() final override;

	//	virtual void PerformUpdate(uint32_t instanceID, const float4x4& transform) final override;

	////  void Release() final override;
	//	const void* GetDefaultDescriptor() final override { return &m_Resource; };
	//	const void* GetResourceMemory() final override { return m_ResourceHandle.data; };

	//private:
	//	sce::Psr::TopLevelBvhDescriptor m_Resource;
	//	MemoryHandle<uint8_t, sce::Psr::kRequiredAlignment> m_ResourceHandle;
	//};
}