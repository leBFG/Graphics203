#pragma once
#include "Skateboard/Log.h"
#include <memory>
#include <array>
//#include <unordered_map>
//#include "Skateboard/Graphics/Resources/CommonResources.h"

namespace Skateboard
{

	/*constexpr uint32_t MAX_RASTERIZATION_PIPELINES = 512u;


	class PipelineMap
	{
		friend class Scene;
	public:
		PipelineMap();
		DISABLE_COPY_AND_MOVE(PipelineMap);
		~PipelineMap();

		void Clean();

		RasterizationPipeline* AddRasterizationPipeline(std::string_view name, RasterizationPipeline* pipeline);
		bool RemoveRasterizationPipeline(std::string_view name);

		RasterizationPipeline* GetPipeline(std::string_view name);
		RasterizationPipeline* GetPipeline(int32_t id);


		int32_t GetPipelineId(const std::string_view& name);

	private:
		std::unordered_map<std::string_view, int32_t> m_TagToId;

		std::array<std::unique_ptr<RasterizationPipeline>, MAX_RASTERIZATION_PIPELINES> m_Pipelines;
		std::vector<int32_t> m_AvailableIndices;

		uint32_t m_PipelineCount;
		uint32_t m_PipelineAvailable;
	};*/

}
