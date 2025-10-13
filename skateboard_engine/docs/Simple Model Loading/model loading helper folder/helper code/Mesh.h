#pragma once
//namespace sce { namespace Agc {
//	namespace Core
//	{
//		struct Buffer;
//	}
//	enum class IndexSize;
//}}

#include "libScePackParser/pack_file.h"
#include "Utils.h"

struct Mesh
{
	uint32_t mTextureIndex;
	uint32_t mIndexCount;
	sce::Agc::Core::Buffer mVertexBuffer;
	void* mIndexBuffer;
};

class MeshInstance
{
public:
	MeshInstance();
	~MeshInstance();

	// The cube mesh created in the PS5 Hello World Cube demo ported to our mesh instance class
	void InitAsCube();

	// Refer to Appendix 1 & 2 to break down the code into easier steps
	void InitFromPack(const PackFile::Package& packFile);
	
	// This render function will essentially replace the stageBinder function
	void Render(sce::Agc::DrawCommandBuffer* dcb,
		sce::Agc::Core::StateBuffer* sb,
		sce::Agc::Shader* gs,
		sce::Agc::Shader* ps,
		sce::Agc::Core::Buffer* passbuffer,
		sce::Agc::Core::Buffer* objbuffer,
		uint32_t objBufferIndex,
		sce::Agc::Core::Sampler* sampler);

private:
	std::vector<Mesh> mMeshes;
	std::vector<sce::Agc::Core::Texture> mTextures;
};