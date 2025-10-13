
#include "sktbdpch.h"

#include "D3DView.h"
#include "D3DBuffer.h"

#include "Graphics/RHI/D3DGraphicsContext.h"

namespace  Skateboard
{
	D3DConstantBufferView::D3DConstantBufferView(const BufferViewDesc& viewDesc, BufferRef parent) : ConstantBufferView(viewDesc)
	{
		m_Descriptor = gD3DContext->GetGPUSRVDescriptorHeap().Allocate();

		if (parent.get())
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC CBVdesc{};
			CBVdesc.BufferLocation = static_cast<D3DBuffer*>(parent.get())->m_BufferResource->GetResource()->GetGPUVirtualAddress() + viewDesc.Offset;
			CBVdesc.SizeInBytes = viewDesc.ElementSize;
			gD3DContext->GetDevice()->CreateConstantBufferView(&CBVdesc, m_Descriptor.GetCPUHandle());
		}
		else
		{
			gD3DContext->GetDevice()->CreateConstantBufferView(nullptr, m_Descriptor.GetCPUHandle());
		}

		//SetParent
		m_ParentResource = parent;
	}

	D3DShaderResourceBufferView::D3DShaderResourceBufferView(const BufferViewDesc& viewDesc, BufferRef Parent) : ShaderResourceBufferView(viewDesc)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVdesc{};

		SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVdesc.Buffer.NumElements = viewDesc.ElementCount;
		SRVdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (viewDesc.Type)
		{
		case BufferType_ByteBuffer:
			SRVdesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
			SRVdesc.Buffer.FirstElement = viewDesc.Offset;
			SRVdesc.Buffer.StructureByteStride = 1;
			SRVdesc.Format = DXGI_FORMAT_UNKNOWN;
			break;
		case BufferType_StructureBuffer:
			SRVdesc.Buffer.FirstElement = viewDesc.Offset;
			SRVdesc.Buffer.StructureByteStride = viewDesc.ElementSize;
			SRVdesc.Format = DXGI_FORMAT_UNKNOWN;
			break;
		case BufferType_FormattedBuffer:
			SRVdesc.Buffer.FirstElement = viewDesc.Offset;
			SRVdesc.Buffer.StructureByteStride = 0;
			SRVdesc.Format = SkateboardBufferFormatToD3D(viewDesc.Format);
			break;
		default:
			break;
		}

		m_Descriptor = gD3DContext->GetGPUSRVDescriptorHeap().Allocate();

		if(Parent.get())
			gD3DContext->GetDevice()->CreateShaderResourceView(static_cast<D3DBuffer*>(Parent.get())->m_BufferResource->GetResource(), &SRVdesc, m_Descriptor.GetCPUHandle());
		else
			gD3DContext->GetDevice()->CreateShaderResourceView(nullptr, &SRVdesc, m_Descriptor.GetCPUHandle());


		//SetParent
		m_ParentResource = Parent;
			
	}

	D3DUnorderedAccessBufferView::D3DUnorderedAccessBufferView(const BufferViewDesc& viewDesc, BufferRef Parent, BufferRef CounterResource) : UnorderedAccessBufferView(viewDesc)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVdesc{};

		switch (viewDesc.Type)
		{
		case BufferType_ByteBuffer:
			UAVdesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
			UAVdesc.Buffer.FirstElement = viewDesc.Offset;
			UAVdesc.Buffer.StructureByteStride = 1;
			UAVdesc.Format = DXGI_FORMAT_UNKNOWN;
			break;

		case BufferType_StructureBuffer:
			UAVdesc.Buffer.FirstElement = viewDesc.Offset;
			UAVdesc.Buffer.StructureByteStride = viewDesc.ElementSize;
			UAVdesc.Format = DXGI_FORMAT_UNKNOWN;
			break;

		case BufferType_FormattedBuffer:
			UAVdesc.Buffer.FirstElement = viewDesc.Offset;
			UAVdesc.Buffer.StructureByteStride = 0;
			UAVdesc.Format = SkateboardBufferFormatToD3D(viewDesc.Format);
			break;

		case BufferType_ConstantBuffer:
			SKTBD_LOG_ASSERT(false, "Cant Create a Writable Constant Buffer, use Structured Buffer View to Inject a value into CBV")
		};

		UAVdesc.Buffer.CounterOffsetInBytes = 0;

		m_Descriptor = gD3DContext->GetGPUSRVDescriptorHeap().Allocate();

		auto cntResource = (CounterResource.get()) ? static_cast<D3DBuffer*>(CounterResource.get())->m_BufferResource->GetResource() : nullptr;
		auto Resource = (Parent.get()) ? static_cast<D3DBuffer*>(Parent.get())->m_BufferResource->GetResource() : nullptr;

		gD3DContext->GetDevice()->CreateUnorderedAccessView(Resource, cntResource, &UAVdesc, m_Descriptor.GetCPUHandle());

		//SetParent
		m_ParentResource = Parent;
	}

	D3DShaderResourceTextureView::D3DShaderResourceTextureView(const TextureViewDesc& viewDesc, TextureBufferRef Parent) : ShaderResourceTextureView(viewDesc)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVdesc{};
		
		SRVdesc.Format = SkateboardBufferFormatToD3D(viewDesc.Format);
		SRVdesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);

		switch (viewDesc.Dimension)
		{
		case TextureDimension_Texture1D:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			SRVdesc.Texture1D.MipLevels = viewDesc.MipLevels;
			SRVdesc.Texture1D.MostDetailedMip = viewDesc.MostDetailedMip;
			SRVdesc.Texture1D.ResourceMinLODClamp = viewDesc.ResourceMinLodClamp;
			break;
		case TextureDimension_Texture1D_ARRAY:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			SRVdesc.Texture1DArray.ArraySize = viewDesc.ArraySize;
			SRVdesc.Texture1DArray.FirstArraySlice = viewDesc.FirstArraySlice;
			SRVdesc.Texture1DArray.MipLevels = viewDesc.MipLevels;
			SRVdesc.Texture1DArray.MostDetailedMip = viewDesc.MostDetailedMip;
			SRVdesc.Texture1DArray.ResourceMinLODClamp = viewDesc.ResourceMinLodClamp;
			break;
		case TextureDimension_Texture2D:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SRVdesc.Texture2D.MipLevels = viewDesc.MipLevels;
			SRVdesc.Texture2D.PlaneSlice = viewDesc.PlaneSlice;
			SRVdesc.Texture2D.MostDetailedMip = viewDesc.MostDetailedMip;
			SRVdesc.Texture2D.ResourceMinLODClamp = viewDesc.ResourceMinLodClamp;
			break;
		case TextureDimension_Texture2D_ARRAY:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			SRVdesc.Texture2DArray.ArraySize = viewDesc.ArraySize;
			SRVdesc.Texture2DArray.FirstArraySlice = viewDesc.FirstArraySlice;
			SRVdesc.Texture2DArray.MipLevels = viewDesc.MipLevels;
			SRVdesc.Texture2DArray.PlaneSlice = viewDesc.PlaneSlice;
			SRVdesc.Texture2DArray.MostDetailedMip = viewDesc.MostDetailedMip;
			SRVdesc.Texture2DArray.ResourceMinLODClamp = viewDesc.ResourceMinLodClamp;

			break;
		case TextureDimension_Texture2D_MS:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			break;
		case TextureDimension_Texture2D_MS_ARRAY:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
			SRVdesc.Texture2DArray.ArraySize = viewDesc.ArraySize;
			SRVdesc.Texture2DArray.FirstArraySlice = viewDesc.FirstArraySlice;
			SRVdesc.Texture2DArray.MipLevels = viewDesc.MipLevels;
			SRVdesc.Texture2DArray.MostDetailedMip = viewDesc.MostDetailedMip;
			SRVdesc.Texture2DArray.PlaneSlice = viewDesc.PlaneSlice;
			SRVdesc.Texture2DArray.ResourceMinLODClamp = viewDesc.ResourceMinLodClamp;
			break;
		case TextureDimension_Texture3D:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
			SRVdesc.Texture3D.MipLevels = viewDesc.MipLevels;
			SRVdesc.Texture3D.MostDetailedMip = viewDesc.MostDetailedMip;
			SRVdesc.Texture3D.ResourceMinLODClamp = viewDesc.ResourceMinLodClamp;
			break;
		case TextureDimension_CubeMap:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			SRVdesc.TextureCube.MipLevels = viewDesc.MipLevels;
			SRVdesc.TextureCube.MostDetailedMip = viewDesc.MostDetailedMip;
			SRVdesc.TextureCube.ResourceMinLODClamp = viewDesc.ResourceMinLodClamp;
			break;
		case TextureDimension_CubeMap_ARRAY:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			SRVdesc.TextureCubeArray.First2DArrayFace = viewDesc.FirstArraySlice;
			SRVdesc.TextureCubeArray.MipLevels = viewDesc.MipLevels;
			SRVdesc.TextureCubeArray.MostDetailedMip = viewDesc.MostDetailedMip;
			SRVdesc.TextureCubeArray.NumCubes = viewDesc.ArraySize;
			SRVdesc.TextureCubeArray.ResourceMinLODClamp = viewDesc.ResourceMinLodClamp;
			break;
		default:
			SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_UNKNOWN;
			break;
		}

		//allocate descriptor
		m_Descriptor = gD3DContext->GetGPUSRVDescriptorHeap().Allocate();

		//SetParent
		m_ParentResource = Parent;

		auto Resource = (Parent.get()) ? static_cast<D3DTextureBuffer*>(Parent.get())->m_TextureResource->GetResource() : nullptr;

		gD3DContext->GetDevice()->CreateShaderResourceView(Resource, &SRVdesc, m_Descriptor.GetCPUHandle());

	}

	D3DUnorderedAccessTextureView::D3DUnorderedAccessTextureView(const TextureViewDesc& viewDesc, TextureBufferRef Parent) : UnorderedAccessTextureView(viewDesc)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVdesc{};

		UAVdesc.Format = SkateboardBufferFormatToD3D(viewDesc.Format);
		switch (viewDesc.Dimension)
		{
		case TextureDimension_Texture1D:
			UAVdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			UAVdesc.Texture1D.MipSlice = viewDesc.MipSlice;
			break;
		case TextureDimension_Texture1D_ARRAY:
			UAVdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			UAVdesc.Texture1DArray.ArraySize = viewDesc.ArraySize;
			UAVdesc.Texture1DArray.FirstArraySlice = viewDesc.FirstArraySlice;
			UAVdesc.Texture1DArray.MipSlice = viewDesc.MipSlice;
			break;
		case TextureDimension_Texture2D:
			UAVdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			UAVdesc.Texture2D.MipSlice = viewDesc.MipSlice;
			UAVdesc.Texture2D.PlaneSlice = viewDesc.PlaneSlice;
			break;
		case TextureDimension_Texture2D_ARRAY:
			UAVdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			UAVdesc.Texture2DArray.ArraySize = viewDesc.ArraySize;
			UAVdesc.Texture2DArray.FirstArraySlice = viewDesc.FirstArraySlice;
			UAVdesc.Texture2DArray.MipSlice = viewDesc.MipSlice;
			UAVdesc.Texture2DArray.PlaneSlice = viewDesc.PlaneSlice;
			break;
		case TextureDimension_Texture2D_MS:
			UAVdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMS;
			break;
		case TextureDimension_Texture2D_MS_ARRAY:
			UAVdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY;
			UAVdesc.Texture2DArray.ArraySize = viewDesc.ArraySize;
			UAVdesc.Texture2DArray.FirstArraySlice = viewDesc.FirstArraySlice;
			UAVdesc.Texture2DArray.MipSlice = viewDesc.MipSlice;
			UAVdesc.Texture2DArray.PlaneSlice = viewDesc.PlaneSlice;
			break;
		case TextureDimension_Texture3D:
			UAVdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY;
			UAVdesc.Texture3D.FirstWSlice = viewDesc.FirstArraySlice;
			UAVdesc.Texture3D.MipSlice = viewDesc.MipSlice;
			UAVdesc.Texture3D.WSize = viewDesc.ArraySize;
			break;
		case TextureDimension_CubeMap:
			SKTBD_LOG_ASSERT(false, "Creating a UAV TextureCubeArray is illegal, create A Texture Array (6) instead")
				break;
		case TextureDimension_CubeMap_ARRAY:
			SKTBD_LOG_ASSERT(false, "Creating a UAV TextureCubeArray is illegal, create A Texture Array (6)[n] instead")
				break;
		default:
			UAVdesc.ViewDimension = D3D12_UAV_DIMENSION_UNKNOWN;
			break;
		}

		//allocate descriptor
		m_Descriptor = gD3DContext->GetGPUSRVDescriptorHeap().Allocate();

		//SetParent
		m_ParentResource = Parent;

		auto Resource = (Parent.get()) ? static_cast<D3DTextureBuffer*>(Parent.get())->m_TextureResource->GetResource() : nullptr;

		gD3DContext->GetDevice()->CreateUnorderedAccessView(Resource, nullptr, &UAVdesc, m_Descriptor.GetCPUHandle());
	}

	D3DRenderTargetView::D3DRenderTargetView(const RenderTargetDesc* viewDesc, TextureBufferRef Parent) : RenderTargetView(viewDesc)
	{
		//allocate descriptor
		m_Descriptor = gD3DContext->GetRTVDescriptorHeap().Allocate();

		//SetParent
		m_ParentResource = Parent;

		if (viewDesc)
		{
			D3D12_RENDER_TARGET_VIEW_DESC RTdesc{};

			RTdesc.Format = SkateboardBufferFormatToD3D(viewDesc->Format);

			switch (viewDesc->Dimension)
			{
			case TextureDimension_Texture1D:
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
				RTdesc.Texture1D.MipSlice = viewDesc->MipSlice;
				break;
			case TextureDimension_Texture1D_ARRAY:
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
				RTdesc.Texture1DArray.ArraySize = viewDesc->ArraySize;
				RTdesc.Texture1DArray.FirstArraySlice = viewDesc->FirstArraySlice;
				RTdesc.Texture1DArray.MipSlice = viewDesc->MipSlice;
				break;
			case TextureDimension_Texture2D:
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				RTdesc.Texture2D.MipSlice = viewDesc->MipSlice;
				RTdesc.Texture2D.PlaneSlice = viewDesc->PlaneSlice;
				break;
			case TextureDimension_Texture2D_ARRAY:
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				RTdesc.Texture2DArray.ArraySize = viewDesc->ArraySize;
				RTdesc.Texture2DArray.FirstArraySlice = viewDesc->FirstArraySlice;
				RTdesc.Texture2DArray.MipSlice = viewDesc->MipSlice;
				RTdesc.Texture2DArray.PlaneSlice = viewDesc->PlaneSlice;
				break;
			case TextureDimension_Texture2D_MS:
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
				break;
			case TextureDimension_Texture2D_MS_ARRAY:
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
				RTdesc.Texture2DArray.ArraySize = viewDesc->ArraySize;
				RTdesc.Texture2DArray.FirstArraySlice = viewDesc->FirstArraySlice;
				break;
			case TextureDimension_Texture3D:
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
				RTdesc.Texture3D.FirstWSlice = viewDesc->FirstWSlice;
				RTdesc.Texture3D.WSize = viewDesc->Width;
				RTdesc.Texture3D.MipSlice = viewDesc->MipSlice;

				break;
			case TextureDimension_CubeMap:
				SKTBD_MSG_ERROR("Cube Map RTVs not supported");
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN;
				break;

			default:
				RTdesc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN;
				break;
			}

			auto Resource = (Parent.get()) ? static_cast<D3DTextureBuffer*>(Parent.get())->m_TextureResource->GetResource() : nullptr;
			gD3DContext->GetDevice()->CreateRenderTargetView(Resource, &RTdesc, m_Descriptor.GetCPUHandle());

		}
		else
		{
			auto Resource = (Parent.get()) ? static_cast<D3DTextureBuffer*>(Parent.get())->m_TextureResource->GetResource() : nullptr;
			gD3DContext->GetDevice()->CreateRenderTargetView(Resource,nullptr, m_Descriptor.GetCPUHandle());
		}

	}

	D3DDepthStencilView::D3DDepthStencilView(const DepthStencilDesc* viewDesc, TextureBufferRef Parent) : DepthStencilView(viewDesc)
	{
		//allocate descriptor
		m_Descriptor = gD3DContext->GetDSVDescriptorHeap().Allocate();

		//SetParent
		m_ParentResource = Parent;

		if (viewDesc)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC DSTdesc = {};

			DSTdesc.Format = SkateboardBufferFormatToD3D(viewDesc->Format);

			switch (viewDesc->Dimension)
			{
			case TextureDimension_Texture1D:
				DSTdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
				DSTdesc.Texture1D.MipSlice = viewDesc->MipSlice;
				break;
			case TextureDimension_Texture1D_ARRAY:
				DSTdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
				DSTdesc.Texture1DArray.ArraySize = viewDesc->ArraySize;
				DSTdesc.Texture1DArray.FirstArraySlice = viewDesc->FirstArraySlice;
				DSTdesc.Texture1DArray.MipSlice = viewDesc->MipSlice;
				break;
			case TextureDimension_Texture2D:
				DSTdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				DSTdesc.Texture2D.MipSlice = viewDesc->MipSlice;
				break;
			case TextureDimension_Texture2D_ARRAY:
				DSTdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				DSTdesc.Texture2DArray.ArraySize = viewDesc->ArraySize;
				DSTdesc.Texture2DArray.FirstArraySlice = viewDesc->FirstArraySlice;
				DSTdesc.Texture2DArray.MipSlice = viewDesc->MipSlice;
				break;
			case TextureDimension_Texture2D_MS:
				DSTdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
				break;
			case TextureDimension_Texture2D_MS_ARRAY:
				DSTdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
				DSTdesc.Texture2DArray.ArraySize = viewDesc->ArraySize;
				DSTdesc.Texture2DArray.FirstArraySlice = viewDesc->FirstArraySlice;
				break;
			case TextureDimension_Texture3D:
				SKTBD_LOG_ASSERT(false, "3D Depth Render Targets are illegal");
				break;
			case TextureDimension_CubeMap:
				DSTdesc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN;
				break;

			default:
				DSTdesc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN;
				break;
			}

			DSTdesc.Flags |= (viewDesc->Flags & DRT_Flags_::DRT_Flags_ReadOnly_Depth) ? D3D12_DSV_FLAG_READ_ONLY_DEPTH : D3D12_DSV_FLAG_NONE;
			DSTdesc.Flags |= (viewDesc->Flags & DRT_Flags_::DRT_Flags_ReadOnly_Stencil) ? D3D12_DSV_FLAG_READ_ONLY_STENCIL : D3D12_DSV_FLAG_NONE;

			auto Resource = (Parent.get()) ? static_cast<D3DTextureBuffer*>(Parent.get())->m_TextureResource->GetResource() : nullptr;
			gD3DContext->GetDevice()->CreateDepthStencilView(Resource, &DSTdesc, m_Descriptor.GetCPUHandle());
		}
		else
		{
			auto Resource = (Parent.get()) ? static_cast<D3DTextureBuffer*>(Parent.get())->m_TextureResource->GetResource() : nullptr;
			gD3DContext->GetDevice()->CreateDepthStencilView(Resource, nullptr, m_Descriptor.GetCPUHandle());
		}
	}

	D3DAccelerationStructView::D3DAccelerationStructView(AccelerationStructureData AS_handle) : AccelerationStructureView(std::move(AS_handle))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		desc.RaytracingAccelerationStructure.Location = AS_handle.m_Address;

		m_Descriptor = gD3DContext->GetGPUSRVDescriptorHeap().Allocate();

		gD3DContext->GetDevice()->CreateShaderResourceView(nullptr, &desc, m_Descriptor.GetCPUHandle());
	}
}
