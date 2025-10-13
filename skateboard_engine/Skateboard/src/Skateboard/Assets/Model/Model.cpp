#include "sktbdpch.h"
#include "Model.h"

#include "Skateboard/Renderers/Renderer.h"

//#ifdef SKTBD_PLATFORM_WINDOWS
//#include <Platform/DirectX12/Model/D3DMeshletModel.h>
//#endif // SKTBD_PLATFORM_WINDOWS

#define SKTBD_LOG_COMPONENT "MODEL"
#include "Skateboard/Log.h"

namespace Skateboard
{
	Mesh::Mesh(const wchar_t* filename)
	{

	}

	Mesh::Mesh(const std::vector<Primitive>& meshes)
		:
		m_Primitives(meshes)
	{}

	//void Mesh::SetPrimitive(Primitive& prim, uint32_t element)
	//{
	//	SKTBD_LOG_ASSERT(element < m_Primitives.size(), "element out of range.");
	//	m_Primitives[element] = prim;
	//}

	//void Mesh::SetPrimitive(Primitive& prim, const std::string_view& meshTag)
	//{
	//	SKTBD_LOG_ASSERT(m_Tag2Element.count(meshTag) < 0, "element out of range");
	//	m_Primitives[m_Tag2Element[meshTag]] = prim;
	//}

	//void Model::Release()
	//{
	//	// Uh oh! Need the context class to remove the buffers!
	//	for(auto& mesh : m_Meshes)
	//	{
	//	}
	//}

	/*MeshletModel* MeshletModel::Create(const wchar_t* filename)
	{

#if defined SKTBD_PLATFORM_WINDOWS
			return new D3DModel(filename);
#elif defined SKTBD_PLATFORM_PLAYSTATION
			return nullptr;
#endif
	}*/
	
}
