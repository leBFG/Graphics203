#pragma once

#include "Skateboard/Scene/Scene.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>

class AssetImport : public Skateboard::Scene
{
public:
	AssetImport();
	
	virtual void OnHandleInput(Skateboard::TimeManager* time) final override;
	virtual void OnUpdate(Skateboard::TimeManager* time) final override;
	virtual void OnRender() final override;
	virtual void OnImGuiRender() final override;

	Assimp::Importer importer;
};