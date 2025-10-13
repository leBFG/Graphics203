#include "sktbdpch.h"
#include "MeshEngine.h"
#include "Skateboard/Assets/Model/Model.h"

namespace Skateboard
{
	MeshletMap::MeshletMap()
	{
	}
	MeshletModel* MeshletMap::AddMeshletModel(const std::string_view& name, MeshletModel* model)
	{

		if(m_MeshletModels.count(name))
		{
			SKTBD_APP_WARN("A model with that name already exists!");
			return m_MeshletModels[name].get();
		}

		std::shared_ptr<MeshletModel> pModel(model);

		m_MeshletModels.emplace(name, std::move(model));
		
		return m_MeshletModels[name].get();
	}

	MeshletModel* MeshletMap::AddMeshletModel(const std::string_view& name, std::unique_ptr<MeshletModel>& model)
	{
		if (m_MeshletModels.count(name))
		{
			SKTBD_APP_WARN("A model with that name already exists!");
			return m_MeshletModels[name].get();
		}

		m_MeshletModels.emplace(name, std::move(model));

		return m_MeshletModels[name].get();
	}

	MeshletModel* MeshletMap::Get(const std::string_view& name)
	{
		if (!m_MeshletModels.count(name))
		{
			SKTBD_APP_WARN("A model with that name does not exist!");
			return nullptr;
		}

		return m_MeshletModels[name].get();
	}

	bool MeshletMap::Contains(const std::string_view& name)
	{
		return m_MeshletModels.count(name);
	}

	bool MeshletMap::ReleaseModel(const std::string_view& name)
	{
		if (!m_MeshletModels.count(name))
		{
			SKTBD_APP_WARN("A model with that name does not exist!");
			return false;
		}

		auto* pModel = m_MeshletModels[name].get();
		//pModel->Release();
		
		m_MeshletModels.erase(name);

		return true;
	}

	ModelMap::ModelMap()
	{
		m_TagToIndex.emplace("Default", 0);

		m_AvailableIndices.reserve(MAX_MODELS);
		for (int32_t i = 0; i < MAX_MODELS; ++i) {
			m_AvailableIndices.push_back(i);
		}

	}

	Mesh* ModelMap::AddModel(const std::string_view& name, Mesh* model)
	{
		std::string tag(name);
		if (m_Models[m_TagToIndex[tag]] != nullptr)
		{
			SKTBD_APP_WARN("A model with that name already exists!");
			return m_Models[m_TagToIndex[tag]].get();
		}

		int64_t id = m_AvailableIndices[0];	 
		m_TagToIndex[tag] = id;

		std::unique_ptr<Mesh> pModel(model);
		m_Models[m_TagToIndex[tag]] = std::move(pModel);

		// shift all indices from in range [index, end] to the left by 1
		int64_t back = m_AvailableIndices.size() - 1;

		// swap the current index with the last available index
		std::swap(m_AvailableIndices[0], m_AvailableIndices[back]);
		m_AvailableIndices.pop_back();	// then remove it from the list.
		

		return m_Models[m_TagToIndex[tag]].get();
	}

	Mesh* ModelMap::AddModel(int64_t index, Mesh* model)
	{
		if (m_Models[index] != nullptr)
		{
			SKTBD_APP_WARN("A model with that name already exists!");
			return m_Models[index].get();
		}

		std::unique_ptr<Mesh> pModel(model);
		m_Models[index] = std::move(pModel);

		// shift all indices from in range [index, end] to the left by 1
		int64_t back = m_AvailableIndices.size() - 1;

		// swap the current index with the last available index
		std::swap(m_AvailableIndices[index], m_AvailableIndices[back]);
		m_AvailableIndices.pop_back();	// then remove it from the list.

		return m_Models[index].get();
	}


	Mesh* ModelMap::GetModel(const std::string_view& name)
	{
		std::string tag(name);
		if (m_TagToIndex.count(tag))
		{
			return GetModel(m_TagToIndex[tag]);
		}
		return nullptr;
	}

	Mesh* ModelMap::GetModel(int32_t index)
	{
		if (m_Models[index] == nullptr)
		{
			SKTBD_APP_WARN("A model with that name does not exist!");
			return nullptr;
		}
		return m_Models[index].get();
	}

	int64_t ModelMap::GetModelId(const std::string_view& name)
	{
		std::string tag(name);
		if (m_Models[m_TagToIndex[tag]] == nullptr)
		{
			SKTBD_APP_WARN("A model with that name does not exist!");
			return -1;
		}

		return m_TagToIndex[tag];
	}

	bool ModelMap::Contains(const std::string_view& name)
	{
		std::string tag(name);
		if (m_TagToIndex.count(tag))
		{
			return true;
		}
		return false;
	}

	bool ModelMap::Contains(int64_t index)
	{
		return false;
	}

	bool ModelMap::ReleaseModel(const std::string_view& name)
	{
		std::string tag(name);
		if (m_TagToIndex.count(tag))
		{
			ReleaseModel(m_TagToIndex[tag]);
			m_TagToIndex.erase(tag);
			return true;
		}
		return false;
	}

	bool ModelMap::ReleaseModel(int64_t index)
	{
		// Let the internal API handle deleting model resources from the GPU.
		// This is done by calling the Release() function on the model.
		auto* pModel = m_Models[index].get();
	//	pModel->Release();
		m_Models[index].release();
		return true;
	}
}

