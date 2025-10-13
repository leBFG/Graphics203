#include "AGCView.h"

#include "AGCBuffer.h"
#include "AGC/Graphics/AGCTypes.h"

namespace Skateboard
{
	AGCConstantBufferView::AGCConstantBufferView(const BufferViewDesc& viewDesc, BufferRef Parent) : ConstantBufferView(viewDesc)
	{

		m_Descriptor = gAGCContext->GetDescriptorHeap().Allocate();
		if(Parent)
		{
			auto agcbuffer = static_cast<AGCBuffer*>(Parent.get());

			sce::Agc::Core::Buffer buffer;

			SCE_AGC_ASSERT(sce::Agc::Core::initializeConstantBuffer(&buffer, (uint8_t*)agcbuffer->BufferMemory.m_Data + viewDesc.Offset, viewDesc.ElementSize) == SCE_OK);

			m_Descriptor.m_DescriptorPtr->set(buffer);
			m_ParentResource = Parent;
		}
		else
		{
			//initialise an empty descriptor; PS5 doesn't throw errors when null views are created
			m_Descriptor.m_DescriptorPtr->init();
		}
	}

	AGCShaderResourceBufferView::AGCShaderResourceBufferView(const BufferViewDesc& viewDesc, BufferRef Parent) : ShaderResourceBufferView(viewDesc)
	{
		m_Descriptor = gAGCContext->GetDescriptorHeap().Allocate();
		if (Parent)
		{
			auto agcbuffer = static_cast<AGCBuffer*>(Parent.get());

			sce::Agc::Core::Buffer buffer;
			//auto dataAddr = (uint8_t*)agcbuffer->BufferMemory.m_Data + (viewDesc.Offset * viewDesc.ElementSize);
			SceError err;

			switch(viewDesc.Type)
			{
			case BufferType_ByteBuffer:
				err = sce::Agc::Core::initializeByteBuffer(&buffer, (uint8_t*)agcbuffer->BufferMemory.m_Data + viewDesc.Offset, viewDesc.ElementCount);
				break;
			case BufferType_StructureBuffer:
				err = sce::Agc::Core::initializeRegularBuffer(&buffer, (uint8_t*)agcbuffer->BufferMemory.m_Data + (viewDesc.Offset * viewDesc.ElementSize), viewDesc.ElementSize, viewDesc.ElementCount);
				break;
			case BufferType_FormattedBuffer:
				err = sce::Agc::Core::initializeDataBuffer(&buffer, (uint8_t*)agcbuffer->BufferMemory.m_Data + viewDesc.Offset, SkateboardDataFormatToAGC(viewDesc.Format), viewDesc.ElementCount);
				break;
			}

			SCE_AGC_ASSERT(err == SCE_OK);
			//ASSERT(dataAddr == buffer.getDataAddress());

			m_Descriptor.m_DescriptorPtr->set(buffer);
			m_ParentResource = Parent;
		}
		else
		{
			//initialise an empty descriptor; PS5 doesn't throw errors when null views are created
			m_Descriptor.m_DescriptorPtr->init();
		}
	}

	AGCUnorderedAccessBufferView::AGCUnorderedAccessBufferView(const BufferViewDesc& viewDesc, BufferRef Parent,
		BufferRef counterResource) :UnorderedAccessBufferView(viewDesc)
	{
		m_Descriptor = gAGCContext->GetDescriptorHeap().Allocate();
		if (Parent)
		{
			auto agcbuffer = static_cast<AGCBuffer*>(Parent.get());

			sce::Agc::Core::BufferSpec spec;

			switch (viewDesc.Type)
			{
			case BufferType_ByteBuffer:
				spec.initAsByteBuffer((uint8_t*)agcbuffer->BufferMemory.m_Data + viewDesc.Offset, viewDesc.ElementCount);
				break;
			case BufferType_StructureBuffer:
				spec.initAsRegularBuffer((uint8_t*)agcbuffer->BufferMemory.m_Data + viewDesc.Offset, viewDesc.ElementSize, viewDesc.ElementCount);
				break;
			case BufferType_FormattedBuffer:
				spec.initAsDataBuffer((uint8_t*)agcbuffer->BufferMemory.m_Data + viewDesc.Offset, SkateboardDataFormatToAGC(viewDesc.Format), viewDesc.ElementCount);
				break;
			}

			SCE_AGC_ASSERT(sce::Agc::Core::initialize(m_Descriptor.m_DescriptorPtr, &spec) == SCE_OK);
			m_ParentResource = Parent;
		}
		else
		{
			//initialise an empty descriptor; PS5 doesn't throw errors when null views are created
			m_Descriptor.m_DescriptorPtr->init();
		}
	}


	auto TranslateSkateboardViewDescToAGCDescriptor(const AGCTextureBuffer& buffer, sce::Agc::Core::ResourceDescriptor& DestinationDescriptor)
	{
		switch(buffer.GetType())
		{
		case TextureType_Default:
			break;
		case TextureType_RenderTarget:
			break;
		case TextureType_DepthStencil:
			break;
		}
	}

	AGCShaderResourceTextureView::AGCShaderResourceTextureView(const TextureViewDesc& viewDesc, TextureBufferRef parent) : ShaderResourceTextureView(viewDesc)
	{
		m_Descriptor = gAGCContext->GetDescriptorHeap().Allocate();

		if (parent)
		{
			auto agcbuffer = static_cast<AGCTextureBuffer*>(parent.get());

			sce::Agc::Core::TextureSpec spec;

			spec.init();

			spec.setIsWriteable(false);
			spec.setType(SkateboardTextureDimensionToAGC(viewDesc.Dimension));
			spec.setFormat(SkateboardDataFormatToAGC(viewDesc.Format));
			spec.setNumMips(viewDesc.MipLevels);
			spec.setDepth(viewDesc.ArraySize);

			spec.setWidth(parent->GetWidth());
			spec.setHeight(parent->GetHeight());

			spec.setDataAddress(agcbuffer->TextureMemory.m_Data);

			SCE_AGC_ASSERT(sce::Agc::Core::initialize(m_Descriptor.m_DescriptorPtr, &spec) == SCE_OK);

			m_ParentResource = parent;
		}
		else
		{
			//initialise an empty descriptor; PS5 doesn't throw errors when null views are created
			m_Descriptor.m_DescriptorPtr->init();
		}

	}

	AGCShaderResourceTextureView::AGCShaderResourceTextureView(const TextureViewDesc& viewDesc, DescriptorHandle<sce::Agc::Core::ResourceDescriptor> descriptor,
	TextureBufferRef parent) : ShaderResourceTextureView(viewDesc)
	{
		m_ParentResource = parent;
		m_Descriptor = descriptor;
	}

	AGCUnorderedAccessTextureView::AGCUnorderedAccessTextureView(const TextureViewDesc& viewDesc,
		TextureBufferRef parent) : UnorderedAccessTextureView(viewDesc)
	{
		m_Descriptor = gAGCContext->GetDescriptorHeap().Allocate();

		if (parent)
		{
			auto agcbuffer = static_cast<AGCTextureBuffer*>(parent.get());

			switch (agcbuffer->GetType())
			{
			case TextureType_Default:
				sce::Agc::Core::TextureSpec spec;

				spec.init();

				spec.setIsWriteable(true);
				spec.setType(SkateboardTextureDimensionToAGC(viewDesc.Dimension));

				spec.setFormat(SkateboardDataFormatToAGC(viewDesc.Format));

				spec.setNumMips(viewDesc.MipLevels);

				spec.setDepth(viewDesc.ArraySize);

				spec.setWidth(parent->GetWidth());
				spec.setHeight(parent->GetHeight());

				spec.setDataAddress(agcbuffer->TextureMemory.m_Data);

				SCE_AGC_ASSERT(sce::Agc::Core::initialize(m_Descriptor.m_DescriptorPtr, &spec) == SCE_OK);

				break;
			case TextureType_RenderTarget:
				SCE_AGC_ASSERT(sce::Agc::Core::translate(m_Descriptor.m_DescriptorPtr, &agcbuffer->RenderTarget, sce::Agc::Core::RenderTargetComponent::kData) == SCE_OK);
				break;
			case TextureType_DepthStencil:
				SCE_AGC_ASSERT(sce::Agc::Core::translate(m_Descriptor.m_DescriptorPtr, &agcbuffer->DepthRenderTarget, sce::Agc::Core::DepthRenderTargetComponent::kDepth) == SCE_OK);
				break;
			}
			

			

			m_ParentResource = parent;
		}
		else
		{
			//initialise an empty descriptor; PS5 doesn't throw errors when null views are created
			m_Descriptor.m_DescriptorPtr->init();
		}
	}

	AGCRenderTargetView::AGCRenderTargetView(const RenderTargetDesc* viewDesc, TextureBufferRef Parent) : RenderTargetView(viewDesc)
	{
		if(Parent)
		{
			m_ParentResource = Parent;
		}
		else
		{
			
		}
	}

	AGCDepthStencilView::AGCDepthStencilView(const DepthStencilDesc* viewDesc, TextureBufferRef Parent) : DepthStencilView(viewDesc)
	{
		if (Parent)
		{
			m_ParentResource = Parent;
		}
		else
		{

		}
	}
}
