#pragma once

#include <Skateboard.h>

namespace Toolbox
{
	class AssetEditorLayer final : public Skateboard::Layer
	{
	public:
		AssetEditorLayer();
		virtual ~AssetEditorLayer() final override;

		virtual bool OnHandleInput(Skateboard::TimeManager* time) final override;
		virtual bool OnUpdate(Skateboard::TimeManager* time) final override;
		virtual void OnRender() final override;
		virtual void OnImGuiRender() final override;


	private:
		std::unique_ptr<Skateboard::Scene> p_AssetImportScene;
	};
}