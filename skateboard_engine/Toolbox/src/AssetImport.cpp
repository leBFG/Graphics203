#include "AssetImport.h"

AssetImport::AssetImport() : Skateboard::Scene("Asset Import Scene"),
importer()
{
}

void AssetImport::OnHandleInput(Skateboard::TimeManager* time)
{
}

void AssetImport::OnUpdate(Skateboard::TimeManager* time)
{
}

void AssetImport::OnRender()
{
}

void AssetImport::OnImGuiRender()
{
	ImGui::Begin("Asset Importer");
	ImGui::Text("This is the Asset Importer scene.");
	ImGui::End();
}
