#pragma once

#include "Skateboard/Scene/Entity.h"
#include "AABB.h"
#include "Skateboard/SizedPtr.h"
#include "Skateboard/Graphics/Resources/Buffer.h"
#include "Skateboard/Graphics/Resources/View.h"

namespace Skateboard
{
	class Scene;

	//get element of a layout from a byte array
	template<typename T >
	T& GetElement(uint32_t index, const BufferLayout& Components, const BufferElement& Component, void* dest)
	{
		return *(T*)(&((uint8_t*)dest)[index * Components.GetStride() + Component.Offset]);
	}

	// gets struct from byte array equivalent to layout
	template<typename T >
	T& GetStructChecked(uint32_t index, const BufferLayout& Components, void* dest)
	{
		ASSERT(Components.GetStride() == sizeof(T))
		return (T)((uint8_t*)dest)[index * Components.GetStride()];
	}

	inline SizedPtr GetOpaqueStruct(uint32_t index, const BufferLayout& Components, void* dest)
	{
		return { &((uint8_t*)dest)[index * Components.GetStride()], Components.GetStride() };
	};

	// casts byte array into array of type
	template<typename T >
	T& GetTypedElement(uint32_t index, void* dest)
	{
		return ((T*)dest)[index];
	}

	struct MeshData
	{
		std::string Name;

		AABB BoundingBox;

		uint32_t VertexCount;
		BufferLayout VertexLayout;
		std::vector<std::byte> VertexData;

		uint32_t IndexCount;
		IndexFormat  IndexLayout;
		std::vector<std::byte> Indices;

		template<typename T>
		T* GetVertexData()
		{
			return reinterpret_cast<T*>(VertexData.data());
		}

		template<typename T>
		T* GetIndexData()
		{
			return reinterpret_cast<T*>(Indices.data());
		}

		template<typename T>
		T& IndexType(uint32_t index)
		{
			return GetTypedElement<T>(index, Indices.data());
		}

		template<typename T>
		T& VertexStruct(uint32_t index)
		{
			return GetStructChecked<T>(index, VertexLayout, VertexData.data());
		}

		template<typename T>
		T& VertexElement(uint32_t index, const BufferElement& element)
		{
			return GetElement<T>(index, VertexLayout, element, VertexData.data());
		}

		template<typename VertexType>
		VertexType& VertexTypeElement(uint32_t index)
		{
			return GetTypedElement<VertexType>(index, VertexData.data());
		}

		SizedPtr OpaqueVertex(uint32_t index)
		{
			return GetOpaqueStruct(index, VertexLayout, VertexData.data());
		}
	};

	class SceneBuilder
	{
		friend Scene;
	protected:
		// would only generate these types in that layout
		/*struct VertexType
		{
			float3 position;
			float2 texCoord;
			float3 normal;
			float3 tangent;
		};

		const BufferLayout m_Layout = {
			{ POSITION, Skateboard::ShaderDataType_::Float3 },
			{ TEXCOORD, Skateboard::ShaderDataType_::Float2 },
			{ NORMAL, Skateboard::ShaderDataType_::Float3 },
			{ TANGENT, Skateboard::ShaderDataType_::Float3 },
		};*/

	protected:
		GENERATE_DEFAULT_CLASS(SceneBuilder)
	public:
		static MeshData BuildCone(BufferLayout Layout, uint32_t resolution = 20u, bool capMesh = false);
		static MeshData BuildCube(BufferLayout Layout, float size = 1);
		static MeshData BuildCubeSphere(BufferLayout Layout, float radius = 1.f, uint32_t meshResolution = 20u);
		static MeshData BuildCylinder(BufferLayout Layout, uint32_t resolution = 20, uint32_t stackCount = 4, bool capMesh = false);
		static MeshData BuildSphere(BufferLayout Layout, float size = 1.0f, uint32_t resolution = 20u);
		static MeshData BuildTerrain(BufferLayout Layout, uint32_t resolution = 20u);

	private:
	};
}
