#pragma once
#include "Skateboard/Log.h"
#include "Skateboard/Mathematics.h"
#include "Skateboard/Assets/Model/Model.h"

namespace Skateboard
{

 

	//	<summary>
	//	The meshlet engine serves mainly to generate meshlet buffers from geometry which has been configured for the
	//  traditional pipeline. Additionally, the class contains features to flatten meshlet buffers into traditional vertex
	//	and index buffers.
	//	</summary>
	class MeshEngine
	{
	public:
		MeshEngine() = default;
		DISABLE_COPY_AND_MOVE(MeshEngine);
		virtual ~MeshEngine(){}

		virtual void GenerateMeshlets
		(
			uint32_t maxVertices, uint32_t maxPrimitives,
			uint32_t* indices, uint32_t iCount,
			float3* positions, uint32_t vCount,
			std::vector<Subset>& subsets,
            std::vector<Meshlet>& meshlets,
            std::vector<uint8_t>& uniqueVertexIndices,
            std::vector<PackedTriangle>& primitiveIndices
		) {}

        virtual void GenerateMeshlets
        (
            uint32_t maxVertices, uint32_t maxPrimitives,
            uint16_t* indices, uint32_t iCount,
            float3* positions, uint32_t vCount,
            std::vector<Subset>& subsets,
            std::vector<Meshlet>& meshlets,
            std::vector<uint8_t>& uniqueVertexIndices,
            std::vector<PackedTriangle>& primitiveIndices
        ) {}

		virtual void GenerateIBuffers
		(
            uint32_t maxVertices, uint32_t maxPrimitives,
            float3* positions, uint32_t vCount,
            std::vector<Subset>& subsets,
            std::vector<Meshlet>& meshlets,
            std::vector<uint8_t>& uniqueVertexIndices,
            std::vector<PackedTriangle>& primitiveIndices,
            std::vector<uint32_t> outIBuffer
        ) {}

        virtual void GenerateIBuffers
        (
            uint32_t maxVertices, uint32_t maxPrimitives,
            float3* positions, uint32_t vCount,
            std::vector<Subset>& subsets,
            std::vector<Meshlet>& meshlets,
            std::vector<uint8_t>& uniqueVertexIndices,
            std::vector<PackedTriangle>& primitiveIndices,
            std::vector<uint16_t> outIBuffer
        ) {}

	protected:

	};

    constexpr uint32_t MAX_MODELS = 128u;

    class MeshletMap
    {
    public:
        MeshletMap();
        DISABLE_COPY_AND_MOVE(MeshletMap);
        ~MeshletMap() = default;

        MeshletModel* AddMeshletModel(const std::string_view& name, MeshletModel* model);
        MeshletModel* AddMeshletModel(const std::string_view& name, std::unique_ptr<MeshletModel>& model);

        MeshletModel* Get(const std::string_view& name);

        bool Contains(const std::string_view& name);
        bool ReleaseModel(const std::string_view& name);

    private:
        std::unordered_map<std::string_view, std::unique_ptr<MeshletModel>> m_MeshletModels;
    };

    class ModelMap
    {
    public:
        ModelMap();
        DISABLE_COPY_AND_MOVE(ModelMap);
        ~ModelMap() = default;

        Mesh* AddModel(const std::string_view& name, Mesh* model);
        Mesh* AddModel(const std::string_view& name, std::unique_ptr<Mesh>& model);

        Mesh* AddModel(int64_t index, Mesh* model);
        Mesh* AddModel(int64_t index, std::unique_ptr<Mesh>& model);

        Mesh* GetModel(const std::string_view& name);
        Mesh* GetModel(int32_t index);

        int64_t GetModelId(const std::string_view& name);

        bool Contains(const std::string_view& name);
        bool ReleaseModel(const std::string_view& name);

        bool Contains(int64_t index);
        bool ReleaseModel(int64_t index);

    private:
        std::unordered_map<std::string, int64_t> m_TagToIndex;
        std::array<std::unique_ptr<Mesh>, MAX_MODELS> m_Models;
        std::vector<int64_t> m_AvailableIndices;

    };

    class MeshMap {
    public:
        MeshMap()=default;
        DISABLE_COPY_AND_MOVE(MeshMap);
        ~MeshMap() = default;

    };
}
