#pragma once

//#include "Skateboard/Renderer/GraphicsContext.h"
#include "Skateboard/Graphics/RHI/ResourceFactory.h"
#include "Skateboard/Graphics/RHI/RenderCommand.h"
#include "Skateboard/Renderers/Renderer.h"

using namespace Skateboard;

namespace CMP203
{
	//buffer layouts

	struct Vertex
	{
		float3 Position;
		float3 Colour;
		float2 UV;
		float3 Normal;

		//default colour is white
		Vertex() : Position(0, 0, 0), Colour(0, 0, 0), UV(0, 0), Normal(0,0,0) { }
		Vertex(float3 pos) : Vertex() { Position = pos; }
		Vertex(float3 pos, float3 col) : Vertex(pos) { Colour = col; }
		Vertex(float3 pos, float3 col, float2 uv) : Vertex(pos, col) { UV = uv; }
		Vertex(float3 pos, float3 col, float2 uv, float3 normal) : Vertex(pos, col, uv) {  Normal = normal; }

		static BufferLayout VertexLayout()
		{
			return {
				{ POSITION, ShaderDataType_::Float3 },
				{ COLOUR,	  ShaderDataType_::Float3 },
				{ TEXCOORD, ShaderDataType_::Float2 },
				{ NORMAL,   ShaderDataType_::Float3 },
			};
		}
	};

	enum LightType : uint32_t
	{
		LightDirectional = 0,
		LightPoint = 1,
		LightSpot = 2,
	};

	struct InstanceData
	{
		matrix World;
		float4 InstanceColour;

		int TextureIndex;
		int SamplerIndex;

		float SpecularPower;
		float SpecularStrength;

		InstanceData() { World = glm::identity<glm::mat4x4>(); InstanceColour = float4(0, 0, 0, 0);  TextureIndex = 1; SamplerIndex = 0; SpecularPower = 1.f; SpecularStrength = 1; }
	};

	struct Light
	{
	public:
		float4 DiffuseColour;
		float ConstantAttenuation;
		float LinearAttenuation;
		float SquareAttenuation;
		float CutOffDistance;

		float3 LightPosition;
		float InnerCone;

		float3 LightDirection;
		float OuterCone;

	private:
		float2 Padding;
	public:

		float FalloffPower;
		LightType Type;

		Light(LightType type = LightSpot)
		{
			switch (type)
			{
			case LightDirectional:
				Type = LightDirectional;
				LightDirection = float3(1, -1, 1);
				DiffuseColour = float4(0.5, 0.5, 0.5, 1);
				break;
			case LightPoint:
				Type = LightPoint;
				DiffuseColour = float4(0.5, 0.5, 0.5, 1);
				LightPosition = float3(0, 0, 0);
				ConstantAttenuation = 0.05f;
				LinearAttenuation = 0.01f;
				SquareAttenuation = 0.001f;
				CutOffDistance = 100.f;
				break;
			case LightSpot:
				Type = LightSpot;
				LightDirection = float3(0, -1, 0);
				DiffuseColour = float4(0.5, 0.5, 0.5, 1);
				LightPosition = float3(0, 0, 0);
				ConstantAttenuation = 0.05f;
				LinearAttenuation = 0.01f;
				SquareAttenuation = 0.001f;
				CutOffDistance = 100.f;
				InnerCone = glm::radians(30.f);
				OuterCone = glm::radians(90.f);
				FalloffPower = 2;
				break;
			}

		}
	};

	struct  CameraData
	{
		glm::mat4x4 ViewMatrix;
		glm::mat4x4 ProjectionMatrix;
	};

	struct Frame
	{
		CameraData CamMatrices;

		matrix CameraMatrix;
		uint32_t LightCount;
		float3 AmbientLight;
	};

	static SamplerDesc AnisotropicSampler(uint8_t anisotropy, SamplerMode_ U, SamplerMode_ V)
	{
		return
		{
			.Filter = SamplerFilter_::SamplerFilter_Anisotropic,
			.ModeU = U,
			.ModeV = V,
			.ModeW = V,
			.MipMapLevelOffset = 0.f,
			.MipMapMinSampleLevel = 0.f,
			.MipMapMaxSampleLevel = 10.f,
			.MaxAnisotropy = anisotropy,	// Valid range 1 - 16 -> uint32_t cause padding anyways
			.ComparisonFunction = SamplerComparisonFunction_Never,
			.BorderColour = SamplerBorderColour_TransparentBlack,
			.Flags = SamplerFlags_::SamplerFlags_NONE,
		};
	}

	static SamplerDesc LinearSampler(SamplerMode_ U, SamplerMode_ V)
	{
		auto sampler = SamplerDesc::InitAsDefaultTextureSampler();
		sampler.ModeU = U;
		sampler.ModeV = V;
		return sampler;
	}

	static SamplerDesc PointSampler(SamplerMode_ U, SamplerMode_ V)
	{
		auto sampler = SamplerDesc::InitAsDefaultTextureSampler();
		sampler.Filter = SamplerFilter_::SamplerFilter_Comaprison_Min_Mag_Mip_Point;
		sampler.ModeU = U;
		sampler.ModeV = V;
		return sampler;
	}

	enum PipelineFlags : uint8_t
	{
		DEPTH_TEST = 1 << 0,
		ALPHA_BLEND = 1 << 1,
		WIREFRAME = 1 << 2,
		LIT = 1 << 3,
		FACE_CULL_NONE = 1<<4
	};
	ENUM_FLAG_OPERATORS(PipelineFlags);

	constexpr static PipelineFlags DEFAULT_FLAGS = DEPTH_TEST;
	constexpr static uint8_t PIPELINE_PERMUTATIONS = (DEPTH_TEST | ALPHA_BLEND | WIREFRAME | LIT | FACE_CULL_NONE) + 1;

	class Renderer203
	{
	public:
		Renderer203() = default;
		virtual ~Renderer203() = default;
		DISABLE_COPY_AND_MOVE(Renderer203);

	private:
		Skateboard::ShaderInputLayoutRef RootSig;

		//pipeline permutations
		std::array<Skateboard::GraphicsPipelineRef, PIPELINE_PERMUTATIONS> Pipelines;

		//normal rendering
		Skateboard::GraphicsPipelineRef NormalVisualizer;

		size_t DynamicBufferSize = 64 * 1024;
		size_t InstanceDataBufferSize = 64 * 1024;

		Skateboard::MultiResource<Skateboard::BufferRef> TriangleDataBuffer;
		Skateboard::MultiResource<Skateboard::BufferRef> FrameDataBuffer;
		Skateboard::MultiResource<Skateboard::BufferRef> InstanceDataBuffer;
		Skateboard::MultiResource<Skateboard::BufferRef> LightDataBuffer;

		//inline view desc
		BufferViewDesc sbvdesc;
		BufferViewDesc lightsbvdesc;
		BufferViewDesc cbvdesc;
	
		//Sampler keep the reference around to keep the object alive
		SamplerRef m_DefaultSampler;

		//counts offsets for dynamic data uploaded every frame
		size_t m_Offset = 0;//ROUND_UP(sizeof(CameraData), GraphicsConstants::CONSTANT_BUFFER_ALIGNMENT);

		//forked on each call with a unique data pointer/ otherwise default instance data is used
		uint32_t m_InstanceDataForks = 0;

		SKTBD_PRIMITIVE_TOPOLOGY m_Topology = SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		Frame m_Frame{ .AmbientLight = { 0.1, 0.1, 0.1} };

		std::vector<Light> m_Lights;

		InstanceData m_DefaultInstanceData{};

		uint32_t m_DefaultTextureIDX = 0;
		uint32_t m_DefaultSamplerIDX = 0;

		PipelineFlags m_PipelineSelection = DEFAULT_FLAGS;
		bool m_Pipeline_dirty;

		bool m_LightsDirty = false;
		bool m_FrameDataDirty = false;
		bool m_VisualiseNormals = false;

	public:
		virtual void Init();

		virtual void Init(size_t TriangleBuffersizeOverride, size_t InstanceBufferSizeOverride)
		{
			DynamicBufferSize = ROUND_UP(TriangleBuffersizeOverride, GraphicsConstants::DEFAULT_RESOURCE_ALIGNMENT);
			InstanceDataBufferSize = ROUND_UP(InstanceBufferSizeOverride, GraphicsConstants::DEFAULT_RESOURCE_ALIGNMENT);
			Init();
		}

		virtual void Begin();
		virtual void End();

		void SetAmbientLight(float3 light_colour) {
			m_Frame.AmbientLight = light_colour;
			m_FrameDataDirty = true;
		}

		void SetTopology(SKTBD_PRIMITIVE_TOPOLOGY nTopology) { m_Topology = nTopology; }

		void SetLights(std::vector<Light> Lights)
		{
			m_Lights = std::move(Lights);
			m_FrameDataDirty = true;
			m_LightsDirty = true;
		};

		Light& GetLight(uint32_t idx)
		{
			m_LightsDirty = true;
			return m_Lights[idx];
		}

		Light GetLight(uint32_t idx) const
		{
			return m_Lights[idx];
		}

		size_t GetLightCount() const
		{
			return m_Lights.size();
		}

		uint32 GetDefaultTextureIDX() const { return m_DefaultTextureIDX; }
		uint32 GetDefaultSamplerIDX() const { return m_DefaultSamplerIDX; }

		//RendererConfiguration
		ShaderInputLayoutRef GetRootSig() { return RootSig; };
		PipelineFlags GetPipelineFlags() const { return m_PipelineSelection; };

		void SetPipelineFlags(PipelineFlags Flags = DEFAULT_FLAGS)
		{
			m_PipelineSelection |= Flags;
			m_Pipeline_dirty = true;
		}

		void UnsetPipelineFlags(PipelineFlags Flags = DEFAULT_FLAGS)
		{
			m_PipelineSelection &= ~Flags;
			m_Pipeline_dirty = true;
		}

		void ResetPipelineFlags(PipelineFlags Flags = DEFAULT_FLAGS)
		{
			m_PipelineSelection = Flags;
			m_Pipeline_dirty = true;
		}

		//Debug normals
		void SetDrawDebugNormals(bool DrawNormals) { m_VisualiseNormals = DrawNormals; }

		void SetCameraData(const CameraData& newCameraData);

		//Copies and draws Triangle Data from the CPU to the GPU
		void DrawVertices(Vertex* vertxBuffer, size_t vertexcount, void* indexbuffer, size_t indexcount, InstanceData* data = nullptr, IndexFormat indextype = bit32);

		// DOESNT Copy Data Over and just draws SpecifiedVertex and Index Buffers, Use with assets from asset manager
		void DrawVBIB(VertexBufferView* vb, IndexBufferView* ib, InstanceData* data = nullptr);
	};

};
