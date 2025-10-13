#include "AssetEditorLayer.h"
#include "Skateboard/Assets/AssetManager.h"
#include "Skateboard/Scene/SceneBuilder.h"

#include "AssetImport.h"

namespace Toolbox
{
	AssetEditorLayer::AssetEditorLayer()
	{
		// Initialise the scene
		p_AssetImportScene = std::make_unique<AssetImport>();
	}

	AssetEditorLayer::~AssetEditorLayer()
	{
	}

	bool AssetEditorLayer::OnHandleInput(Skateboard::TimeManager* time)
	{
		p_AssetImportScene->OnHandleInput(time);

		return true;
	}

	bool AssetEditorLayer::OnUpdate(Skateboard::TimeManager* time)
	{
		// Prepare for rendering

		p_AssetImportScene->OnUpdate(time);

		return true;
	}

	void AssetEditorLayer::OnRender()
	{
		// Render the scene
		p_AssetImportScene->OnRender();

	}

	void AssetEditorLayer::OnImGuiRender()
	{
		p_AssetImportScene->OnImGuiRender();
	}
}