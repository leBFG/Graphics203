#pragma once
#include <unordered_map>
#include "Skateboard/Assets/AssetManager.h"

struct usint2
{
	unsigned short int x, y;
};

namespace PackFile
{
	struct Package;
}

namespace Skateboard
{

	class AGCAssetManager final : public AssetManager
	{
	public:
		AGCAssetManager();
		virtual ~AGCAssetManager() final override {}

		// Abstracted Loaders
		const Texture LoadTextureImpl(const wchar_t* filename, const std::string& textureTag, TextureDimension_ resType) override;
		Mesh* LoadModelImpl(const wchar_t* filename, const std::string& modelTag, const BufferLayout& Layout, const std::string& obj_name = "",bool loadIndexBuffer = true, bool flipNormals = false) override;

	private:
		PackFile::Package LoadPackage(const wchar_t* filename);

	protected:
		Animation* LoadAnimationImpl(const wchar_t* filename, const std::string& animationTag) override;
		SkinnedMesh*
		LoadSkeletalMeshImpl(const wchar_t* filename, const std::string& meshTag, bool flipNormals) override;
		Skeleton* LoadSkeletonImpl(const wchar_t* filename, const std::string& skeletonTag) override;
		Texture CreateTextureFromDataImpl(const std::string& textureTag, const TextureDesc& ,void* data) override;
		ImFont* LoadFontImpl(const wchar_t* filename, uint32_t sizeInPixels, const std::string& modelTag) override;
		Texture GetDefaultTextureImpl(const TextureDimension_ Dimension) override;
		//bool LoadTextures(const PackFile::Package& package, std::vector<>);

	};
}