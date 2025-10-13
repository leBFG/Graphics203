#pragma once
#include <map>
#include "Span.h"
#include "Skateboard/Graphics/Resources/CommonResources.h"
#include "Skateboard/Renderers//Materials/Material.h"
#include "Skateboard/Mathematics.h"
#include "Skateboard/Memory/VirtualAllocator.h"
#include "Skateboard/Scene/AABB.h"

using namespace Skateboard::MemoryUtils;

namespace Skateboard
{
    struct MaterialData
    {
        float4 Albedo;
        int AlbedoMapIndex;
        int3 padding;
        float3 Fresnel;
        float Metallic;
        float3 Specular;
        float Roughness;
    };

	class PerspectiveCamera;

    #define CULL_FLAG 0x1
    #define MESHLET_FLAG 0x2

    struct Attribute
    {
        enum EType : uint32_t
        {
            Position,
            Normal,
            TexCoord,
            Tangent,
            Bitangent,
            Count
        };

        EType    Type;
        uint32_t Offset;
    };

    struct Subset
    {
        uint32_t Offset;
        uint32_t Count;
    };

    __declspec(align(256u))
    struct MeshInfo
    {
        uint32_t IndexSize;
        uint32_t MeshletCount;

        uint32_t LastMeshletVertCount;
        uint32_t LastMeshletPrimCount;
    };

    struct Meshlet
    {
        uint32_t VertCount;
        uint32_t VertOffset;
        uint32_t PrimCount;
        uint32_t PrimOffset;
    };

    struct PackedTriangle
    {
        uint32_t i0 : 10;
        uint32_t i1 : 10;
        uint32_t i2 : 10;
    };

    struct CullData
    {
        float4  BoundingSphere;   // xyz = center, w = radius
        uint8_t NormalCone[4];    // xyz = axis, w = -cos(a + 90)
        float   ApexOffset;       // apex = center - axis * offset
    };

    __declspec(align(256u))
	struct MeshletCullPass
    {
        float4x4 View;
        float4x4 ViewProj;
        float4 Planes[6];

        float3 ViewPosition;
        uint32_t HighlightedIndex;

        float3 CullViewPosition;
        uint32_t SelectedIndex;

        uint32_t DrawMeshlets;
    };

    struct MeshInstance
    {
        float4x4 WorldTransform;
        float4x4 WorldInvTransform;
        float Scale;
        uint32_t Flags;
    };

    struct PrimitiveSuballocationSingleBuffer
    {
        BlockAllocator* ParentAllocator;
        VirtualAllocation PrimitiveData;

        //Release the Allocations on Deletion of the Primitive
		  ~PrimitiveSuballocationSingleBuffer()
        {
            if (ParentAllocator)
            {
                ParentAllocator->FreeAllocation(PrimitiveData);
            }
        }
    };

    struct PrimitiveBuffers
    {
        BufferRef VertexBuffer;
        BufferRef IndexBuffer;
    };


    struct Primitive
    {
        Primitive()
	        :
	        Layout(BufferLayout::GetDefaultLayout())
	        , Allocations()
	        , IndexBuffer(), BoundBox()
        {
        }

        Primitive(const Primitive& rhs)
	        :
	        Layout(rhs.Layout)
	        , Allocations(rhs.Allocations)
	        , VertexBuffers(rhs.VertexBuffers)
	        , IndexBuffer(rhs.IndexBuffer)
    		, BoundBox(rhs.BoundBox)
        {
        }

        auto operator=(const Primitive& rhs) noexcept -> Primitive&{
            Layout=rhs.Layout;
            VertexBuffers =rhs.VertexBuffers;
            IndexBuffer =rhs.IndexBuffer;
            Allocations = rhs.Allocations;
            BoundBox = rhs.BoundBox;
            return *this;
        }

        Primitive(Primitive&& rhs) noexcept
	        :
    			Layout(rhs.Layout)
			, Allocations(std::move(rhs.Allocations))
			, VertexBuffers(std::move(rhs.VertexBuffers))
	        , IndexBuffer(std::move(rhs.IndexBuffer))
    	    , BoundBox(rhs.BoundBox)
        {
        }

        auto operator=(Primitive&& rhs) noexcept -> Primitive& {
            Layout = rhs.Layout;
            VertexBuffers = std::move(rhs.VertexBuffers);
            IndexBuffer = std::move(rhs.IndexBuffer);
            Allocations = std::move(rhs.Allocations);
            BoundBox = rhs.BoundBox;
            return *this;
        }

        BufferLayout Layout;                        // Vertex data layout
		std::shared_ptr<PrimitiveSuballocationSingleBuffer>  Allocations;

        std::map<uint8_t,VertexBufferView> VertexBuffers;
        IndexBufferView IndexBuffer;

        AABB BoundBox;
    };

	class Mesh
	{
	public:
        virtual ~Mesh() = default;
        Mesh() = default;
        explicit Mesh(const wchar_t* filename);
        explicit Mesh(const std::vector<Primitive>& meshes);

        BufferLayout GetPrimitiveLayout(uint32_t primitiveIDX) { return m_Primitives[primitiveIDX].Layout; }

        VertexBufferView* GetVertexBuffer(uint32_t PrimtiveIDX, uint8_t inputSlot = 0)
        {
	        if(m_Primitives[PrimtiveIDX].Layout.GetSlotsAndStrides().contains(inputSlot))
                return &m_Primitives[PrimtiveIDX].VertexBuffers[inputSlot];
			else
				return nullptr;
        };

        IndexBufferView* GetIndexBuffer(uint32_t PrimtiveIDX) { return &m_Primitives[PrimtiveIDX].IndexBuffer; };

        Primitive* GetPrimitive(uint32_t meshElement) { return &m_Primitives[meshElement]; }
        const Primitive* GetPrimitive(uint32_t meshElement) const {return &m_Primitives[meshElement]; }

        uint32_t GetPrimitiveCount() const { return static_cast<uint32_t>(m_Primitives.size()); }
        const std::vector<Primitive>& GetPrimitives() const { return m_Primitives; }

        std::vector<Primitive>::iterator begin() { return m_Primitives.begin(); }
        std::vector<Primitive>::iterator end()   { return m_Primitives.end(); }

    public:
        Transform Offset;
    protected:
        std::vector<Primitive>       m_Primitives;
	};

	class MeshletModel : public Mesh
	{
	public:
        virtual void SetInputLayout(const BufferLayout& inputLayout, const std::string_view& meshTag) = 0;


		static MeshletModel* Create(const wchar_t* filename);

		virtual GPUResource* GetMeshletBuffer(const std::wstring_view& meshTag)         = 0;
		virtual GPUResource* GetUniqueVertexIndices(const std::wstring_view& meshTag)   = 0;
		virtual GPUResource* GetPrimitiveIndices(const std::wstring_view& meshTag)      = 0;
        virtual GPUResource* GetMeshInfo(const std::wstring_view& meshTag)              = 0;

		virtual const std::vector<Span<uint8_t>>&   GetRawVertexData(uint32_t meshIndex = 0)		   = 0;
		virtual const Span<Meshlet>&				GetRawMeshletData(uint32_t meshIndex = 0)	       = 0;
		virtual const Span<PackedTriangle>&		    GetRawPrimitiveIndices(uint32_t meshIndex = 0)	   = 0;
		virtual const Span<uint8_t>&				GetRawUniqueVertexIndices(uint32_t meshIndex = 0)  = 0;

        virtual void SetTransform(float4x4 transform) = 0;
        virtual GPUResource* GetTransformBuffer() = 0;

        virtual GPUResource* GetModelCullData(const std::wstring_view& meshTag) = 0;
        virtual GPUResource* GetSceneCullData() = 0;

        virtual void Cull(PerspectiveCamera* camera, PerspectiveCamera* debugCamera) = 0;

	};

}
