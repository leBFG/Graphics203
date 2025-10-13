#include "sktbdpch.h"
//#include "PipelineMap.h"
//#include "Skateboard/Renderers/MeshletEngine/MeshEngine.h"
//#include "Skateboard/Graphics/Resources/CommonResources.h"

namespace Skateboard
{
	//PipelineMap::PipelineMap()
	//	:
	//		m_PipelineCount(0u)
	//	,	m_PipelineAvailable(MAX_RASTERIZATION_PIPELINES)
	//{
	//	m_AvailableIndices.reserve(MAX_RASTERIZATION_PIPELINES);
	//	for (size_t i = 0; i < MAX_RASTERIZATION_PIPELINES; ++i){
	//		m_AvailableIndices.push_back(static_cast<uint32_t>(i));
	//	}

	//	m_TagToId.reserve(MAX_RASTERIZATION_PIPELINES);
	//}

	//PipelineMap::~PipelineMap()
	//{
	//	Clean();
	//}

	//void PipelineMap::Clean()
	//{
	//}

	//RasterizationPipeline* PipelineMap::AddRasterizationPipeline(std::string_view name, RasterizationPipeline* pipeline)
	//{
	//	if (m_Pipelines[m_TagToId[name]] != nullptr){
	//		SKTBD_APP_WARN("A pipeline with that name already exists!");
	//		return m_Pipelines[m_TagToId[name]].get();
	//	}

	//	int32_t id = m_AvailableIndices.back();
	//	m_TagToId[name] = id;

	//	std::unique_ptr<RasterizationPipeline> pPipeline(pipeline);
	//	m_Pipelines[id] = std::move(pPipeline);

	//	m_PipelineCount += 1u;
	//	m_PipelineAvailable -= 1u;

	//	m_AvailableIndices.pop_back();

	//	return m_Pipelines[id].get();
	//}

	//bool PipelineMap::RemoveRasterizationPipeline(std::string_view name)
	//{
	//	if (m_Pipelines[m_TagToId[name]] == nullptr) {
	//		SKTBD_APP_WARN("A pipeline with that name does not exist!");
	//		return false;
	//	}

	//	uint32_t id = m_TagToId[name];

	//	// Do not try to delete this on this call...
	//	// Let the internal rendering API handle this.
	//	m_Pipelines[id]->Release();

	//	m_PipelineCount -= 1u;
	//	m_PipelineAvailable += 1u;

	//	m_AvailableIndices.push_back(id);
	//	m_TagToId.erase(name);

	//	return true;
	//}

	//RasterizationPipeline* PipelineMap::GetPipeline(std::string_view name)
	//{
	//	int32_t id = m_TagToId[name];
	//	if (m_Pipelines[id] == nullptr) {
	//		SKTBD_APP_WARN("A pipeline with that name does not exist!");
	//		return nullptr;
	//	}

	//	return m_Pipelines[m_TagToId[name]].get();
	//}

	//RasterizationPipeline* PipelineMap::GetPipeline(int32_t id)
	//{
	//	if (m_Pipelines[id] == nullptr) {
	//		SKTBD_APP_WARN("A pipeline with that name does not exist!");
	//		return nullptr;
	//	}

	//	return m_Pipelines[id].get();
	//}

	//int32_t PipelineMap::GetPipelineId(const std::string_view& name)
	//{
	//	if (m_TagToId.count(name) < 0u)
	//	{
	//		return -1;
	//	}

	//	return m_TagToId[name];
	//}



}
