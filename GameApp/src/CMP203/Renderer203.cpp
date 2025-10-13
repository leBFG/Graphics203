#include "Renderer203.h"
#include "Skateboard/Assets/AssetManager.h"

#define SKTBD_LOG_COMPONENT "Renderer203"

namespace CMP203
{
	void Renderer203::Init()
	{
		SKTBD_MSG_INFO("Init")

		m_DefaultTextureIDX = AssetManager::GetDefaultTexture(TextureDimension_Texture2D)->GetViewIndex();

		m_DefaultSampler = ResourceFactory::CreateSampler(SamplerDesc::InitAsDefaultTextureSampler(), L"Renderer203Sampler");
		m_DefaultSamplerIDX = m_DefaultSampler->GetSamplerIndex();

		//Layout
		ShaderInputLayoutDesc Layout{};

		//instance index
		Layout.AddRootConstant(1, 0);

		//frame data
		Layout.AddConstantBufferView(1);

		//instance data 
		Layout.AddShaderResourceView(0);

		//light data
		Layout.AddShaderResourceView(1);
		

		Layout.DescriptorsDirctlyAddresssed = true;
		Layout.SamplersDirectlyAddressed = true;
		Layout.CanUseInputAssembler = true;

		RootSig = ResourceFactory::CreateShaderInputLayout(Layout);

		GraphicsPipelineDesc Raster{};
		Raster.Rasterizer = RasterizerConfig::Default();
		Raster.Blend.AlphaToCoverage = false;
		Raster.Blend.IndependentBlendEnable = false;
		Raster.Blend.RTBlendConfigs[0].BlendEnable = false;
		Raster.Blend.RTBlendConfigs[0].RenderTargetWriteMask = 0xF;
		Raster.DepthStencil.DepthEnable = false;
		Raster.DepthStencil.BackFace.StencilDepthFailOp = SKTBD_StencilOp_KEEP;
		Raster.DepthStencil.BackFace.StencilFailOp = SKTBD_StencilOp_KEEP;
		Raster.DepthStencil.BackFace.StencilFunc = SKTBD_CompareOp_ALWAYS;
		Raster.DepthStencil.BackFace.StencilPassOp = SKTBD_StencilOp_KEEP;
		Raster.DepthStencil.DepthFunc = SKTBD_CompareOp_LESS;
		Raster.DepthStencil.DepthWriteAll = true;
		Raster.DepthStencil.FrontFace = Raster.DepthStencil.BackFace;
		Raster.DepthStencil.StencilEnable = false;
		Raster.DepthstencilTargetFormat = DataFormat_DEFAULT_DEPTHSTENCIL;
		Raster.InputPrimitiveType = PrimitiveTopologyType_Triangle;
		Raster.RenderTargetCount = 1;
		Raster.RenderTargetDataFormats[0] = DataFormat_DEFAULT_BACKBUFFER;
		Raster.InputVertexLayout = Vertex::VertexLayout();
		Raster.SampleCount = 1;
		Raster.SampleQuality = 0;
		Raster.SampleMask = 1;

		Raster.SetVertexShader(L"CMP203_VertexShader");
		Raster.SetPixelShader(L"CMP203_PixelShader_Unlit");

		//piepline permutations
		for(uint32_t p = 0; p < PIPELINE_PERMUTATIONS; p++ )
		{
			auto RSPD = Raster;

			if (p & PipelineFlags::LIT)
			{
				RSPD.SetPixelShader(L"CMP203_PixelShader");
			}

			if (p & PipelineFlags::ALPHA_BLEND)
			{
				RSPD.Blend.AlphaToCoverage = true;
				RSPD.Blend.RTBlendConfigs[0].BlendEnable = true;
				RSPD.Blend.RTBlendConfigs[0].BlendOp = SKTBD_BlendOp_ADD;
				RSPD.Blend.RTBlendConfigs[0].SrcBlend = SKTBD_Blend_SRC_ALPHA;
				RSPD.Blend.RTBlendConfigs[0].DestBlend = SKTBD_Blend_INV_SRC_ALPHA;
				RSPD.Blend.RTBlendConfigs[0].BlendOpAlpha = SKTBD_BlendOp_ADD;
				RSPD.Blend.RTBlendConfigs[0].SrcBlendAlpha = SKTBD_Blend_SRC_ALPHA;
				RSPD.Blend.RTBlendConfigs[0].DestBlendAlpha = SKTBD_Blend_INV_SRC_ALPHA;
			}

			if (p & PipelineFlags::WIREFRAME)
			{
				RSPD.Rasterizer.Wireframe = true;
			}

			if (p & PipelineFlags::DEPTH_TEST)
			{
				RSPD.DepthStencil.DepthEnable = true;
			}

			if (p & PipelineFlags::FACE_CULL_NONE)
			{
				Raster.Rasterizer.Cull = Cull_NONE;
			}

			Pipelines[p] = ResourceFactory::CreateGraphicsPipelineState(RSPD, RootSig.get());
		}

		//NormalPipeline 

		Raster.DepthStencil.DepthEnable = true;
		Raster.SetGeometryShader(L"CMP203_NormalGS_GS");
		Raster.SetVertexShader(L"CMP203_NormalGS_VS");
		Raster.SetPixelShader(L"CMP203_NormalGS_PS");


		//L"Renderer203_N_Pipeline", ,
		NormalVisualizer = ResourceFactory::CreateGraphicsPipelineState(Raster, RootSig.get());

		//create Buffers
		BufferDesc bufferdesc{};

		//Triangle Buffer
		bufferdesc.Init(DynamicBufferSize, ResourceAccessFlag_CpuWrite | ResourceAccessFlag_GpuRead);
		TriangleDataBuffer.ForEach([&](BufferRef& ref) {ref = ResourceFactory::CreateBuffer(bufferdesc); });

		//InstanceData Buffer
		bufferdesc.Init(InstanceDataBufferSize, ResourceAccessFlag_CpuWrite | ResourceAccessFlag_GpuRead);
		InstanceDataBuffer.ForEach([&](BufferRef& ref) {ref = ResourceFactory::CreateBuffer(bufferdesc); });

		//LightDataBuffer
		bufferdesc.Init(GraphicsConstants::DEFAULT_RESOURCE_ALIGNMENT, ResourceAccessFlag_CpuWrite | ResourceAccessFlag_GpuRead);
		LightDataBuffer.ForEach([&](BufferRef& ref) {ref = ResourceFactory::CreateBuffer(bufferdesc); });

		//CameraData Buffer
		bufferdesc.Init(ROUND_UP(sizeof(Frame), GraphicsConstants::CONSTANT_BUFFER_ALIGNMENT), ResourceAccessFlag_CpuWrite | ResourceAccessFlag_GpuRead);
		FrameDataBuffer.ForEach([&](BufferRef& ref) {ref = ResourceFactory::CreateBuffer(bufferdesc); });

		//Create Views
		//Camera data
		cbvdesc.InitAsConstantBuffer<Frame>(0);

		//InstanceData
		sbvdesc.InitAsStructuredBuffer<InstanceData>(0, InstanceDataBufferSize / sizeof(InstanceData));

		//lightdata
		lightsbvdesc.InitAsStructuredBuffer<Light>(0, InstanceDataBufferSize / sizeof(Light));

		//compute default camera
		auto aspect = GraphicsContext::Context->GetClientAspectRatio();
		m_Frame.CamMatrices.ProjectionMatrix = glm::perspectiveLH(glm::radians(90.f), aspect, 0.01f, 100.f);
		m_Frame.CamMatrices.ViewMatrix = glm::lookAtLH(glm::vec3(0, 0, -10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		
		m_DefaultInstanceData.TextureIndex = m_DefaultTextureIDX;
		m_DefaultInstanceData.SamplerIndex = m_DefaultSamplerIDX;


		//default Instance
		GraphicsContext::CopyDataToBuffer(InstanceDataBuffer[0].get(), 0, sizeof(InstanceData), &m_DefaultInstanceData);
		GraphicsContext::CopyDataToBuffer(InstanceDataBuffer[1].get(), 0, sizeof(InstanceData), &m_DefaultInstanceData);
		GraphicsContext::CopyDataToBuffer(InstanceDataBuffer[2].get(), 0, sizeof(InstanceData), &m_DefaultInstanceData);

		GraphicsContext::CopyDataToBuffer(FrameDataBuffer[0].get(), 0, sizeof(CameraData), &m_Frame);
		GraphicsContext::CopyDataToBuffer(FrameDataBuffer[1].get(), 0, sizeof(CameraData), &m_Frame);
		GraphicsContext::CopyDataToBuffer(FrameDataBuffer[2].get(), 0, sizeof(CameraData), &m_Frame);
	}

	void Renderer203::Begin()
	{
		//Move Onto Next frame In Buffer
		TriangleDataBuffer.IncrementCounter();

		InstanceDataBuffer.IncrementCounter();

		if (m_FrameDataDirty)
		{
			FrameDataBuffer.IncrementCounter();
		}

		if (m_LightsDirty)
		{
			LightDataBuffer.IncrementCounter();
		}

		//reset the buffer Offset
		m_Offset = 0;

		m_InstanceDataForks = 1;

		RenderCommand::SetInputLayoutGraphics(RootSig.get());
		RenderCommand::SetPipelineState(Pipelines[m_PipelineSelection].get());

		RenderCommand::SetInlineResourceViewGraphics(1, FrameDataBuffer.Get().get(), cbvdesc, ViewAccessType_ConstantBuffer);
		RenderCommand::SetInlineResourceViewGraphics(2, InstanceDataBuffer.Get().get(), sbvdesc, ViewAccessType_GpuRead);
		RenderCommand::SetInlineResourceViewGraphics(3, LightDataBuffer.Get().get(), lightsbvdesc, ViewAccessType_GpuRead);
	}

	void Renderer203::End()
	{
		if (m_FrameDataDirty)
		{
			m_Frame.LightCount = m_Lights.size();
			m_Frame.CameraMatrix = glm::inverse(m_Frame.CamMatrices.ViewMatrix);

			//update the frame data buffer section

			GraphicsContext::CopyDataToBuffer(FrameDataBuffer.Get().get(), 0, sizeof(Frame), &m_Frame);

			m_FrameDataDirty = false;
		}

		if (m_LightsDirty)
		{
			//send lightdata to gpu
			GraphicsContext::CopyDataToBuffer(LightDataBuffer.Get().get(), 0, sizeof(Light) * m_Frame.LightCount, m_Lights.data());
			m_LightsDirty = false;
		}
	}

	void Renderer203::SetCameraData(const CameraData& newCameraData)
	{
		m_FrameDataDirty = true;
		m_Frame.CamMatrices = newCameraData;
	}

	void Renderer203::DrawVBIB(VertexBufferView* vb, IndexBufferView* ib, InstanceData* data)
	{
		if (m_Pipeline_dirty)
		{
			m_Pipeline_dirty = false;
			RenderCommand::SetPipelineState(Pipelines[m_PipelineSelection].get());
		}

		if (data)
		{
			GraphicsContext::CopyDataToBuffer(InstanceDataBuffer.Get().get(), sizeof(InstanceData) * m_InstanceDataForks, sizeof(InstanceData), data);
			RenderCommand::SetInline32bitDataGraphics(0, &m_InstanceDataForks, 1);
			++m_InstanceDataForks;
		}
		else
		{
			//default value
			int d = 0;
			RenderCommand::SetInline32bitDataGraphics(0, &d, 1);
		}



		RenderCommand::SetIndexBuffer(ib);
		RenderCommand::SetVertexBuffer(vb, 1);

		RenderCommand::SetPrimitiveTopology(m_Topology);

		RenderCommand::DrawIndexed(0, 0, ib->m_IndexCount);

		if (m_VisualiseNormals)
		{
			RenderCommand::SetPipelineState(NormalVisualizer.get());
			RenderCommand::DrawIndexed(0, 0, ib->m_IndexCount);
			RenderCommand::SetPipelineState(Pipelines[m_PipelineSelection].get());
		}
	}

	void Renderer203::DrawVertices(Vertex* vertexBuffer, size_t vertexcount, void* indexbuffer, size_t indexcount, InstanceData* data, IndexFormat indextype)
	{
		size_t VBSize = vertexcount * Vertex::VertexLayout().GetStride();
		size_t IBSize = ((indextype == bit32) ? sizeof(uint32_t) : sizeof(uint16_t))  * indexcount;
		
		GraphicsContext::CopyDataToBuffer(TriangleDataBuffer.Get().get(),m_Offset ,VBSize, vertexBuffer);
		GraphicsContext::CopyDataToBuffer(TriangleDataBuffer.Get().get(),m_Offset + ROUND_UP(VBSize, GraphicsConstants::BUFFER_ALIGNMENT), IBSize, indexbuffer);

		Skateboard::VertexBufferView vbv{};
		Skateboard::IndexBufferView ibv{};

		vbv.m_Offset = m_Offset;
		vbv.m_ParentResource = TriangleDataBuffer.Get();
		vbv.m_VertexCount = vertexcount;
		vbv.m_VertexStride = Vertex::VertexLayout().GetStride();

		ibv.m_Offset = m_Offset + VBSize;
		ibv.m_Format = indextype;
		ibv.m_ParentResource = TriangleDataBuffer.Get();
		ibv.m_IndexCount = indexcount;
		
		m_Offset += ROUND_UP(VBSize, GraphicsConstants::BUFFER_ALIGNMENT) + ROUND_UP(IBSize, GraphicsConstants::BUFFER_ALIGNMENT);

		DrawVBIB(&vbv, &ibv, data);
	}
}