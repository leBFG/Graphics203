#include "sktbdpch.h"
#include "D3DBuffer.h"
#include "Graphics/RHI/D3DGraphicsContext.h"

namespace Skateboard
{
	D3DBuffer::D3DBuffer(const BufferDesc& desc) : Buffer(desc)
	{
		D3D12_RESOURCE_DESC1 buffdesc = {};
		buffdesc.Alignment = 0;//(desc.Alignment + SKTBD_DESKTOP_PLATFORM_MIN_BUFFER_ALIGNMENT) & ~SKTBD_DESKTOP_PLATFORM_MIN_BUFFER_ALIGNMENT;
		buffdesc.DepthOrArraySize = 1;
		buffdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		buffdesc.Flags |= (desc.Flags & BufferFlags_RaytacingStructures) ? D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE : D3D12_RESOURCE_FLAG_NONE;
		buffdesc.Flags |= (desc.AccessFlags & ResourceAccessFlag_GpuRead) ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		buffdesc.Flags |= (desc.AccessFlags & ResourceAccessFlag_GpuWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		buffdesc.Format = DXGI_FORMAT_UNKNOWN;
		buffdesc.Height = 1;
		buffdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		buffdesc.MipLevels = 1;
		buffdesc.SampleDesc.Count = 1;
		buffdesc.SampleDesc.Quality = 0;
		buffdesc.Width = desc.Size;

		D3D12MA::ALLOCATION_DESC allocdesc = {};
		allocdesc.HeapType = [desc]() ->D3D12_HEAP_TYPE
			{
				if ((desc.AccessFlags & ResourceAccessFlag_CpuWrite) && !(desc.AccessFlags & ResourceAccessFlag_DesktopPlatformPrimaryResidenceGPU))
					return D3D12_HEAP_TYPE_UPLOAD;
				else if (desc.AccessFlags == ResourceAccessFlag_CpuRead)
					return D3D12_HEAP_TYPE_READBACK;
				else
					return D3D12_HEAP_TYPE_DEFAULT;
			}();

		gD3DContext->GetMemoryAllocator()->CreateResource3(
				&allocdesc,
				&buffdesc,
				D3D12_BARRIER_LAYOUT_UNDEFINED,
				0,
				0,
				NULL,
				&m_BufferResource,
				IID_NULL,
				nullptr
		);
	};

	D3DBuffer::~D3DBuffer()
	{
		m_BufferResource->Release();
		m_BufferResource = nullptr;
	}

	D3DTextureBuffer::D3DTextureBuffer(const TextureDesc& desc) : TextureBuffer(desc)
	{
		D3D12_RESOURCE_DESC1 texturedesc = {};
		texturedesc.Alignment = 0;
		texturedesc.DepthOrArraySize = desc.Depth;
		texturedesc.Dimension = SkateboardTextureDimensionToD3D(desc.Dimension);

		texturedesc.Flags |= (desc.AccessFlags & ResourceAccessFlag_GpuRead) ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		//Depth Stencil is not compatible with this flag
		if (desc.Type != TextureType_DepthStencil)
			texturedesc.Flags |= (desc.AccessFlags & ResourceAccessFlag_GpuWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

		texturedesc.Flags |= (desc.Type == TextureType_RenderTarget) ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;
		texturedesc.Flags |= (desc.Type == TextureType_DepthStencil) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;
		texturedesc.Format = SkateboardBufferFormatToD3D(desc.Format);
		texturedesc.Width = desc.Width;
		texturedesc.Height = desc.Height;
		texturedesc.MipLevels = desc.Mips;
		texturedesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texturedesc.SampleDesc.Count = 1;
		texturedesc.SampleDesc.Quality = 0;

	//	SKTBD_LOG_ASSERT((desc.AccessFlags & (ResourceAccessFlag_GpuRead | ResourceAccessFlag_GpuWrite)) && (desc.Type != TextureType_Default), "RenderTargets Must be GPU Read/Write Access")

		D3D12MA::ALLOCATION_DESC allocdesc = {};
		allocdesc.HeapType = [desc]()->D3D12_HEAP_TYPE
		{
			if (desc.Type != TextureType_Default) return D3D12_HEAP_TYPE_DEFAULT;

			if (desc.AccessFlags & ResourceAccessFlag_GpuWrite | ResourceAccessFlag_DesktopPlatformPrimaryResidenceGPU)
				return D3D12_HEAP_TYPE_DEFAULT;
			else if (desc.AccessFlags == ResourceAccessFlag_CpuRead)
				return D3D12_HEAP_TYPE_READBACK;
			else
				return D3D12_HEAP_TYPE_UPLOAD;
		}();

		auto layout = D3D12_BARRIER_LAYOUT_COMMON;
		D3D12_CLEAR_VALUE clear{};

		clear.Format = SkateboardBufferFormatToD3D(desc.Format);

		if (desc.Type == TextureType_DepthStencil)
		{
			layout = D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
			clear.DepthStencil.Depth = desc.Clear.Depth;
			clear.DepthStencil.Stencil = desc.Clear.Stencil;
		}
		if (desc.Type == TextureType_RenderTarget)
		{
			layout = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
			memcpy(&clear.Color, &desc.Clear.RenderClearValue, sizeof(glm::float4));
		}

		auto allocator = gD3DContext->GetMemoryAllocator();

			
		allocator->CreateResource3(
			&allocdesc,
			&texturedesc,
			layout,
			(Type != TextureType_Default ) ?  &clear : nullptr,
			0,
			NULL,
			&m_TextureResource,
			IID_NULL,
			nullptr
			);
	
	}

	D3DTextureBuffer::~D3DTextureBuffer()
	{
		m_TextureResource->Release();
		m_TextureResource = nullptr;
	}
}
