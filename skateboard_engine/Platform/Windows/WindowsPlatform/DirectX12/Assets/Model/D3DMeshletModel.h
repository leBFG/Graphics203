#pragma once
#include <vector>
#include <functional>
#include "Skateboard/Renderer/Model/Span.h"
#include "Skateboard/Renderer/Model/Model.h"

#ifdef SKTBD_PLATFORM_WINDOWS
#include "Platform/DirectX12/D3D.h"
#include <DirectXCollision.h>
#endif

namespace Skateboard
{

    struct D3DMesh
    {

        D3DMesh() = default;
        ~D3DMesh()
        {}

#ifdef SKTBD_PLATFORM_WINDOWS
        D3D12_INPUT_ELEMENT_DESC   LayoutElems[Attribute::Count];
        D3D12_INPUT_LAYOUT_DESC    LayoutDesc;
        DirectX::BoundingSphere    BoundingSphere;
#endif // SKTBD_PLATFORM_WINDOWS

      
        BufferLayout Layout;

        std::vector<Span<uint8_t>> Vertices;
        std::vector<uint32_t>      VertexStrides;
        uint32_t                   VertexCount;

        Span<Subset>               IndexSubsets;
        Span<uint8_t>              Indices;
        uint32_t                   IndexSize;
        uint32_t                   IndexCount;

        Span<Subset>               MeshletSubsets;
        Span<Meshlet>              Meshlets;
        Span<uint8_t>              UniqueVertexIndices;
        Span<PackedTriangle>       PrimitiveIndices;
        Span<CullData>             CullingData;

        std::vector<std::shared_ptr<VertexBuffer>> VertexResources;
        std::shared_ptr<IndexBuffer> IndexResource;
        //std::shared_ptr<DefaultBuffer> MeshletResource;
        //std::shared_ptr<ByteAddressBuffer> UniqueVertexIndexResource;
        //std::shared_ptr<DefaultBuffer> PrimitiveIndexResource;
        //std::shared_ptr<DefaultBuffer> CullDataResource;
        //std::shared_ptr<UploadBuffer> MeshInfoResource;

        // Calculates the number of instances of the last meshlet which can be packed into a single threadgroup.
        uint32_t GetLastMeshletPackCount(uint32_t subsetIndex, uint32_t maxGroupVerts, uint32_t maxGroupPrims)
        {
            if (Meshlets.size() == 0)
                return 0;

            Subset& subset = MeshletSubsets[subsetIndex];
            Meshlet& meshlet = Meshlets[subset.Offset + subset.Count - 1];
            
            return std::min(maxGroupVerts / meshlet.VertCount, maxGroupPrims / meshlet.PrimCount);
        }

        void GetPrimitive(uint32_t index, uint32_t& i0, uint32_t& i1, uint32_t& i2) const
        {
            PackedTriangle prim = PrimitiveIndices[index];
            i0 = prim.i0;
            i1 = prim.i1;
            i2 = prim.i2;
        }

        uint32_t GetVertexIndex(uint32_t index) const
        {
            const uint8_t* addr = UniqueVertexIndices.data() + index * IndexSize;
            if (IndexSize == 4)
            {
                return *reinterpret_cast<const uint32_t*>(addr);
            }
            
        	return *reinterpret_cast<const uint16_t*>(addr);
        }
    };

    class D3DModel : public MeshletModel
    {
    public:
        explicit D3DModel(const wchar_t* filename);
        ~D3DModel() override;

        void SetInputLayout(const BufferLayout& inputLayout);
        bool LoadFromFile(const wchar_t* filename);

        void SetTransform(float4x4 transform) override;
        GPUResource* GetTransformBuffer() override;

        // Traditional VB/IB buffers

        VertexBuffer* GetVertexBuffer(const std::wstring_view& meshTag);
        IndexBuffer* GetIndexBuffer(const std::wstring_view& meshTag);

        // Mesh shader buffers

        void SetInputLayout(const BufferLayout& inputLayout, const std::string_view& meshTag) {};

        GPUResource* GetMeshletBuffer(const std::wstring_view& meshTag) override;
        GPUResource* GetPrimitiveIndices(const std::wstring_view& meshTag) override;
        GPUResource* GetUniqueVertexIndices(const std::wstring_view& meshTag) override;

        // <summary>
        // Returns a pointer to the models culling data. This buffer contains data which describes
        // how the 
        // </summayr>
        GPUResource* GetModelCullData(const std::wstring_view& meshTag) override;
        GPUResource* GetMeshInfo(const std::wstring_view& meshTag) override;

        // Scene cull data

        // <summary>
        // Returns the models culling buffer containing the necessary culling data
        // to cull meshlets from the model with respect to the scene camera.
        // </summary>
        GPUResource* GetSceneCullData() override;

        void Cull(PerspectiveCamera* camera, PerspectiveCamera* debugCamera) override;
        void Release();

        const std::vector<Span<uint8_t>>& GetRawVertexData(uint32_t meshIndex = 0)           override { return m_Meshes[meshIndex].Vertices; }
        const Span<Meshlet>&              GetRawMeshletData(uint32_t meshIndex = 0)          override { return m_Meshes[meshIndex].Meshlets; }
        const Span<PackedTriangle>&       GetRawPrimitiveIndices(uint32_t meshIndex = 0)     override { return m_Meshes[meshIndex].PrimitiveIndices; }
        const Span<uint8_t>&              GetRawUniqueVertexIndices(uint32_t meshIndex = 0)  override { return m_Meshes[meshIndex].UniqueVertexIndices; }

        uint32_t GetMeshCount() const { return static_cast<uint32_t>(m_Meshes.size()); }
        const D3DMesh& GetMesh(uint32_t i) const { return m_Meshes[i]; }

#ifdef SKTBD_PLATFORM_WINDOWS
        //_NODISCARD const DirectX::BoundingSphere& GetBoundingSphere() const override { return m_boundingSphere; }
#endif
        // Iterator interface
        auto begin() { return m_Meshes.begin(); }
        auto end() { return m_Meshes.end(); }

    private:
#ifdef SKTBD_PLATFORM_WINDOWS
        HRESULT InitialiseModelBuffers();
#endif
    //    std::unique_ptr<UploadBuffer> m_ModelTransformBuffer{};
    //    std::unique_ptr<UploadBuffer> m_CullBuffer{};
        

        std::vector<D3DMesh>     m_Meshes{};
        //DirectX::BoundingSphere  m_boundingSphere{};
        std::vector<uint8_t>     m_buffer{};
        std::unordered_map<std::wstring_view, uint32_t> m_MeshIndexTable{};

    };

}
