#include "sktbdpch.h"
#include "D3DMeshletModel.h"

#include "Skateboard/Log.h"

#include <fstream>
#include <unordered_set>
#include <d3dx12.h>

#include "Platform/DirectX12/D3DGraphicsContext.h"
#include "Skateboard/Camera.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Skateboard
{

    const D3D12_INPUT_ELEMENT_DESC c_elementDescs[Attribute::Count] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
    };

    const uint32_t c_sizeMap[] =
    {
        12, // Position
        12, // Normal
        8,  // TexCoord
        12, // Tangent
        12, // Bitangent
    };

    const uint32_t c_prolog = 'MSHL';

    enum FileVersion
    {
        FILE_VERSION_INITIAL = 0,
        CURRENT_FILE_VERSION = FILE_VERSION_INITIAL
    };

    struct FileHeader
    {
        uint32_t Prolog;
        uint32_t Version;

        uint32_t MeshCount;
        uint32_t AccessorCount;
        uint32_t BufferViewCount;
        uint32_t BufferSize;
    };

    struct MeshHeader
    {
        uint32_t Indices;
        uint32_t IndexSubsets;
        uint32_t Attributes[Attribute::Count];

        uint32_t Meshlets;
        uint32_t MeshletSubsets;
        uint32_t UniqueVertexIndices;
        uint32_t PrimitiveIndices;
        uint32_t CullData;
    };

    struct BufferView
    {
        uint32_t Offset;
        uint32_t Size;
    };

    struct Accessor
    {
        uint32_t BufferView;
        uint32_t Offset;
        uint32_t Size;
        uint32_t Stride;
        uint32_t Count;
    };

    uint32_t GetFormatSize(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
        case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
        case DXGI_FORMAT_R32G32_FLOAT: return 8;
        case DXGI_FORMAT_R32_FLOAT: return 4;
        default: throw std::exception("Unimplemented type");
        }
    }

    template <typename T, typename U>
    constexpr T DivRoundUp(T num, U denom)
    {
        return (num + denom - 1) / denom;
    }

    template <typename T>
    size_t GetAlignedSize(T size)
    {
        const size_t alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        const size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);
        return alignedSize;
    }


    D3DModel::D3DModel(const wchar_t* filename)
    {
        D3DModel::LoadFromFile(filename);

        //TODO: Remove this, for testing purposes!
        m_MeshIndexTable.emplace(L"Main", 0);

        m_ModelTransformBuffer.reset(UploadBuffer::Create(filename,
            UploadBufferDesc::Init(true, 1, sizeof(MeshInstance))));

        m_CullBuffer.reset(UploadBuffer::Create(L"Meshlet Cull",
            UploadBufferDesc::Init(true, 1, sizeof(MeshletCullPass))));

    }

    D3DModel::~D3DModel()
    {

       
    }

    void D3DModel::SetInputLayout(const BufferLayout& inputLayout)
    {
    }

    bool D3DModel::LoadFromFile(const wchar_t* filename)
    {
        std::ifstream stream(filename, std::ios::binary);
        if (!stream.is_open())
        {
            return E_INVALIDARG;
        }

        std::vector<MeshHeader> meshes;
        std::vector<BufferView> bufferViews;
        std::vector<Accessor> accessors;

        FileHeader header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));

        if (header.Prolog != c_prolog)
        {
            return E_FAIL; // Incorrect file format.
        }

        if (header.Version != CURRENT_FILE_VERSION)
        {
            return E_FAIL; // Version mismatch between export and import serialization code.
        }

        // Read mesh metdata
        meshes.resize(header.MeshCount);
        stream.read(reinterpret_cast<char*>(meshes.data()), meshes.size() * sizeof(meshes[0]));

        accessors.resize(header.AccessorCount);
        stream.read(reinterpret_cast<char*>(accessors.data()), accessors.size() * sizeof(accessors[0]));

        bufferViews.resize(header.BufferViewCount);
        stream.read(reinterpret_cast<char*>(bufferViews.data()), bufferViews.size() * sizeof(bufferViews[0]));

        m_buffer.resize(header.BufferSize);
        stream.read(reinterpret_cast<char*>(m_buffer.data()), header.BufferSize);

        char eofbyte;
        stream.read(&eofbyte, 1); // Read last byte to hit the eof bit

        assert(stream.eof()); // There's a problem if we didn't completely consume the file contents.

        stream.close();

        // Populate mesh data from binary data and metadata.
        m_Meshes.resize(meshes.size());
        for (uint32_t i = 0; i < static_cast<uint32_t>(meshes.size()); ++i)
        {
            auto& meshView = meshes[i];
            auto& mesh = m_Meshes[i];

            // Index data
            {
                Accessor& accessor = accessors[meshView.Indices];
                BufferView& bufferView = bufferViews[accessor.BufferView];

                mesh.IndexSize = accessor.Size;
                mesh.IndexCount = accessor.Count;

                mesh.Indices = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
            }

            // Index Subset data
            {
                Accessor& accessor = accessors[meshView.IndexSubsets];
                BufferView& bufferView = bufferViews[accessor.BufferView];

                mesh.IndexSubsets = MakeSpan(reinterpret_cast<Subset*>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Vertex data & layout metadata

            // Determine the number of unique GPUResource Views associated with the vertex attributes & copy vertex buffers.
            std::vector<uint32_t> vbMap;

            /*mesh.LayoutDesc.pInputElementDescs = mesh.LayoutElems;
            mesh.LayoutDesc.NumElements = 0;*/

            for (uint32_t j = 0; j < Attribute::Count; ++j)
            {
                if (meshView.Attributes[j] == -1)
                    continue;

                Accessor& accessor = accessors[meshView.Attributes[j]];

                auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);
                if (it != vbMap.end())
                {
                    continue; // Already added - continue.
                }

                // New buffer view encountered; add to list and copy vertex data
                vbMap.push_back(accessor.BufferView);
                BufferView& bufferView = bufferViews[accessor.BufferView];

                Span<uint8_t> verts = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);

                mesh.VertexStrides.push_back(accessor.Stride);
                mesh.Vertices.push_back(verts);
                mesh.VertexCount = static_cast<uint32_t>(verts.size()) / accessor.Stride;
            }

            // Populate the vertex buffer metadata from accessors.
            for (uint32_t j = 0; j < Attribute::Count; ++j)
            {
                if (meshView.Attributes[j] == -1)
                    continue;

                Accessor& accessor = accessors[meshView.Attributes[j]];

                // Determine which vertex buffer index holds this attribute's data
                auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);

              /*
				D3D12_INPUT_ELEMENT_DESC desc = c_elementDescs[j];
                desc.InputSlot = static_cast<uint32_t>(std::distance(vbMap.begin(), it));

                mesh.LayoutElems[mesh.LayoutDesc.NumElements++] = desc;
                */
            }

            // Meshlet data
            {
                Accessor& accessor = accessors[meshView.Meshlets];
                BufferView& bufferView = bufferViews[accessor.BufferView];

                mesh.Meshlets = MakeSpan(reinterpret_cast<Meshlet*>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Meshlet Subset data
            {
                Accessor& accessor = accessors[meshView.MeshletSubsets];
                BufferView& bufferView = bufferViews[accessor.BufferView];

                mesh.MeshletSubsets = MakeSpan(reinterpret_cast<Subset*>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Unique Vertex Index data
            {
                Accessor& accessor = accessors[meshView.UniqueVertexIndices];
                BufferView& bufferView = bufferViews[accessor.BufferView];

                mesh.UniqueVertexIndices = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
            }

            // Primitive Index data
            {
                Accessor& accessor = accessors[meshView.PrimitiveIndices];
                BufferView& bufferView = bufferViews[accessor.BufferView];

                mesh.PrimitiveIndices = MakeSpan(reinterpret_cast<PackedTriangle*>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Cull data
            {
                Accessor& accessor = accessors[meshView.CullData];
                BufferView& bufferView = bufferViews[accessor.BufferView];

                mesh.CullingData = MakeSpan(reinterpret_cast<CullData*>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }
        }

        // Build bounding spheres for each mesh
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_Meshes.size()); ++i)
        {
            auto& tm = m_Meshes[i];

            uint32_t vbIndexPos = 0;

            // Find the index of the vertex buffer of the position attribute
            for (uint32_t j = 1; j < tm.LayoutDesc.NumElements; ++j)
            {
                auto& desc = tm.LayoutElems[j];
                if (strcmp(desc.SemanticName, "POSITION") == 0)
                {
                    vbIndexPos = j;
                    break;
                }
            }

            // Find the byte offset of the position attribute with its vertex buffer
            uint32_t positionOffset = 0;

            for (uint32_t j = 0; j < tm.LayoutDesc.NumElements; ++j)
            {
                auto& desc = tm.LayoutElems[j];
                auto& pDesc = tm.Layout;
                

                if (strcmp(desc.SemanticName, "POSITION") == 0)
                {
                    break;
                }

                if (desc.InputSlot == vbIndexPos)
                {
                    positionOffset += GetFormatSize(tm.LayoutElems[j].Format);
                }
            }

            XMFLOAT3* v0 = reinterpret_cast<XMFLOAT3*>(tm.Vertices[vbIndexPos].data() + positionOffset);
            uint32_t stride = tm.VertexStrides[vbIndexPos];

            BoundingSphere::CreateFromPoints(tm.BoundingSphere, tm.VertexCount, v0, stride);

            if (i == 0)
            {
                m_boundingSphere = tm.BoundingSphere;
            }
            else
            {
                BoundingSphere::CreateMerged(m_boundingSphere, m_boundingSphere, tm.BoundingSphere);
            }
        }

        HRESULT hr{ S_OK };
        if(!SUCCEEDED(InitialiseModelBuffers()))
        {
            hr = E_FAIL;
        }

        return (hr == S_OK) ? true : false;
    }

    void D3DModel::SetTransform(float4x4 transform)
    {
        auto frameIndex = gD3DContext->GetCurrentFrameResourceIndex();
        MeshInstance mT{};
        mT.WorldTransform = transform;
        mT.Flags = 1u;
        m_ModelTransformBuffer->CopyData(0, &mT);
    }

    GPUResource* D3DModel::GetTransformBuffer()
    {
        return m_ModelTransformBuffer.get();
    }

    VertexBuffer* D3DModel::GetVertexBuffer(const std::wstring_view& meshTag)
    {
        return m_Meshes[m_MeshIndexTable[meshTag]].VertexResources[0].get();
    }

    IndexBuffer* D3DModel::GetIndexBuffer(const std::wstring_view& meshTag)
    {
        return m_Meshes[m_MeshIndexTable[meshTag]].IndexResource.get();
    }

    GPUResource* D3DModel::GetMeshletBuffer(const std::wstring_view& meshTag)
    {
        return m_Meshes[m_MeshIndexTable[meshTag]].MeshletResource.get();
    }

    GPUResource* D3DModel::GetPrimitiveIndices(const std::wstring_view& meshTag)
    {
        return m_Meshes[m_MeshIndexTable[meshTag]].PrimitiveIndexResource.get();
    }

    GPUResource* D3DModel::GetUniqueVertexIndices(const std::wstring_view& meshTag)
    {
        return m_Meshes[m_MeshIndexTable[meshTag]].UniqueVertexIndexResource.get();
    }

    GPUResource* D3DModel::GetModelCullData(const std::wstring_view& meshTag)
    {
        return m_Meshes[m_MeshIndexTable[meshTag]].CullDataResource.get();
    }

    GPUResource* D3DModel::GetMeshInfo(const std::wstring_view& meshTag)
    {
        return m_Meshes[m_MeshIndexTable[meshTag]].MeshInfoResource.get();
    }

    GPUResource* D3DModel::GetSceneCullData()
    {
        return m_CullBuffer.get();
    }

    void D3DModel::Cull(PerspectiveCamera* camera, PerspectiveCamera* debugCamera)
    {

        XMMATRIX view = camera->GetViewMatrix();
        XMMATRIX proj = camera->GetProjectionMatrix();
        XMMATRIX viewInv = XMMatrixInverse(nullptr, view);

        XMVECTOR scale, rot, viewPosition;
        XMMatrixDecompose(&scale, &rot, &viewPosition, viewInv);

        // Calculate the debug camera's properties to extract plane data.
        XMMATRIX cullWorld = XMMatrixInverse(nullptr, debugCamera->GetViewMatrix());
        XMMATRIX cullView = debugCamera->GetViewMatrix();
        XMMATRIX cullProj = debugCamera->GetProjectionMatrix();

        XMVECTOR cullScale, cullRot, cullPos;
        XMMatrixDecompose(&cullScale, &cullRot, &cullPos, cullWorld);

        // Extract the planes from the debug camera view-projection matrix.
        XMMATRIX vp = XMMatrixTranspose(cullView * cullProj);
        XMVECTOR planes[6] =
        {
            XMPlaneNormalize(vp.r[3] + vp.r[0]), // Left
            XMPlaneNormalize(vp.r[3] - vp.r[0]), // Right
            XMPlaneNormalize(vp.r[3] + vp.r[1]), // Bottom
            XMPlaneNormalize(vp.r[3] - vp.r[1]), // Top
            XMPlaneNormalize(vp.r[2]),           // Near
            XMPlaneNormalize(vp.r[3] - vp.r[2]), // Far
        };

        // Set constant data to be read by the shaders.
        MeshletCullPass constants = {};
        constants.HighlightedIndex = 1;
        constants.SelectedIndex = 1;
        constants.DrawMeshlets = 1;
        constants.View = XMMatrixTranspose(view);
        constants.ViewProj = XMMatrixTranspose(view * proj);
        XMStoreFloat3(&constants.ViewPosition, viewPosition);
        XMStoreFloat3(&constants.CullViewPosition, cullPos);

        for (uint32_t i = 0; i < _countof(planes); ++i)
        {
            XMStoreFloat4(&constants.Planes[i], planes[i]);
        }

        m_CullBuffer->CopyData(0, &constants);

    }

    void D3DModel::Release()
    {
        for(const auto& mesh : m_Meshes)
        {
            mesh.CullDataResource->Release();
            mesh.IndexResource->Release();
            mesh.MeshInfoResource->Release();
            mesh.MeshletResource->Release();

            for(const auto& vBuffer : mesh.VertexResources)
            {
                vBuffer->Release();
            }
        }
    }

    HRESULT D3DModel::InitialiseModelBuffers()
    {

        for (uint32_t i = 0; i < m_Meshes.size(); ++i)
        {
            auto& tm   = m_Meshes[i];

            DefaultBufferDesc desc = {}; 
            tm.VertexResources.resize(m_Meshes.size());
            for (uint32_t j = 0; j < tm.Vertices.size(); ++j)
            {
                tm.Layout =
                {
                    {"POSITION", ShaderDataType_::Float3 },
                    {"NORMAL", ShaderDataType_::Float3 }
                };
                tm.VertexResources[j].reset(VertexBuffer::Create(L"Vertex GPUResource", tm.Vertices[j].data(), tm.Vertices[j].size(), tm.Layout));

            }

            tm.IndexResource.reset(IndexBuffer::Create(L"Index GPUResource", (uint32_t*)tm.Indices.data(), tm.Indices.size()/4));

            auto bdesc = ByteAddressBufferDesc::Init(tm.UniqueVertexIndices.data(), DivRoundUp(tm.UniqueVertexIndices.size(), 4) * 4);
            tm.UniqueVertexIndexResource.reset(ByteAddressBuffer::Create(L"Unique Vertex Indices", bdesc));
            tm.UniqueVertexIndexResource->CopyData(tm.UniqueVertexIndices.data(), 0, tm.UniqueVertexIndices.size());

            desc = DefaultBufferDesc::Init(tm.Meshlets.data(), tm.Meshlets.size(), sizeof(Meshlet));
            desc.OverrideStride = false;
            desc.BufferStride = 0;
        	tm.MeshletResource.reset(DefaultBuffer::Create(L"Meshlet GPUResource", desc));

			desc = DefaultBufferDesc::Init(tm.PrimitiveIndices.data(), tm.PrimitiveIndices.size(), sizeof(tm.PrimitiveIndices[0]));
            desc.OverrideStride = false;
            desc.BufferStride = sizeof(tm.PrimitiveIndices[0]);
        	tm.PrimitiveIndexResource.reset(DefaultBuffer::Create(L"Primitive Index GPUResource", desc));

            desc = DefaultBufferDesc::Init(tm.CullingData.data(), tm.CullingData.size(), sizeof(CullData));
            desc.OverrideStride = true;
            desc.BufferStride = sizeof(CullData);
            tm.CullDataResource.reset(DefaultBuffer::Create(L"Cull Data GPUResource", desc));

            MeshInfo info = {};
            info.IndexSize = tm.IndexSize;
            info.MeshletCount = static_cast<uint32_t>(tm.Meshlets.size());
            info.MeshletCount = 0;
            info.LastMeshletVertCount = tm.Meshlets.back().VertCount;
            info.LastMeshletPrimCount = tm.Meshlets.back().PrimCount;

           /* auto udesc = DefaultBufferDesc::Init(&info, 1,sizeof(MeshInfo));
            tm.MeshInfoResource.reset(DefaultBuffer::Create(L"Mesh Info GPUResource", udesc));*/

            auto udesc = UploadBufferDesc::Init(false, 1, sizeof(MeshInfo));
            tm.MeshInfoResource.reset(UploadBuffer::Create(L"Mesh Info GPUResource", udesc));
            tm.MeshInfoResource->CopyData(0, &info);
       
        }

        return S_OK;
    }
}