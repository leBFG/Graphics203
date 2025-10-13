#include "Mesh.h"

MeshInstance::MeshInstance() /*: mIndexBuffer(nullptr), mNumDraws(0u)*/
{
}

MeshInstance::~MeshInstance()
{
	// TODO: Release memory! (Buffers, textures, ..)
}

void MeshInstance::InitAsCube()
{
	constexpr uint32_t vertexCount = 24u; // 4 Vertices per face and 6 faces -> 6 * 4 = 24
	VertexType vertices[vertexCount] = {
		// Top
		{ {-1.f, 1.f, -1.f},	{0.f, 1.f},		{0.f, 1.f, 0.f} },
		{ {1.f, 1.f, -1.f},		{1.f, 1.f},		{0.f, 1.f, 0.f} },
		{ {1.f, 1.f, 1.f},		{1.f, 0.f},		{0.f, 1.f, 0.f} },
		{ {-1.f, 1.f, 1.f},		{0.f, 0.f},		{0.f, 1.f, 0.f} },

		// Bottom
		{ {1.f, -1.f, 1.f},		{0.f, 1.f},		{0.f, -1.f, 0.f} },
		{ {-1.f, -1.f, 1.f},	{1.f, 1.f},		{0.f, -1.f, 0.f} },
		{ {-1.f, -1.f, -1.f},	{1.f, 0.f},		{0.f, -1.f, 0.f} },
		{ {1.f, -1.f, -1.f},	{0.f, 0.f},		{0.f, -1.f, 0.f} },

		// Front
		{ {-1.f, -1.f, -1.f},	{0.f, 1.f},		{0.f, 0.f, -1.f} },
		{ {1.f, -1.f, -1.f},	{1.f, 1.f},		{0.f, 0.f, -1.f} },
		{ {1.f, 1.f, -1.f},		{1.f, 0.f},		{0.f, 0.f, -1.f} },
		{ {-1.f, 1.f, -1.f},	{0.f, 0.f},		{0.f, 0.f, -1.f} },

		// Back
		{ {1.f, -1.f, 1.f},		{0.f, 1.f},		{0.f, 0.f, 1.f} },
		{ {-1.f, -1.f, 1.f},	{1.f, 1.f},		{0.f, 0.f, 1.f} },
		{ {-1.f, 1.f, 1.f},		{1.f, 0.f},		{0.f, 0.f, 1.f} },
		{ {1.f, 1.f, 1.f},		{0.f, 0.f},		{0.f, 0.f, 1.f} },

		// Left
		{ {-1.f, -1.f, 1.f},	{0.f, 1.f},		{-1.f, 0.f, 0.f} },
		{ {-1.f, -1.f, -1.f},	{1.f, 1.f},		{-1.f, 0.f, 0.f} },
		{ {-1.f, 1.f, -1.f},	{1.f, 0.f},		{-1.f, 0.f, 0.f} },
		{ {-1.f, 1.f, 1.f},		{0.f, 0.f},		{-1.f, 0.f, 0.f} },

		// Right
		{ {1.f, -1.f, -1.f},	{0.f, 1.f},		{1.f, 0.f, 0.f} },
		{ {1.f, -1.f, 1.f},		{1.f, 1.f},		{1.f, 0.f, 0.f} },
		{ {1.f, 1.f, 1.f},		{1.f, 0.f},		{1.f, 0.f, 0.f} },
		{ {1.f, 1.f, -1.f},		{0.f, 0.f},		{1.f, 0.f, 0.f} }
	};

	constexpr uint32_t indexCount = 36u;
	uint32_t indices[indexCount] = {
		// Top face
		2, 1, 0,
		3, 2, 0,

		// Bottom face
		4, 5, 6,
		4, 6, 7,

		// Front face
		10, 9, 8,
		11, 10, 8,

		// Back face
		14, 13, 12,
		15, 14, 12,

		// Left face
		18, 17, 16,
		19, 18, 16,

		// Right face
		22, 21, 20,
		23, 22, 20
	};

	//mNumDraws = indexCount / 3;
	Mesh mesh = {};
	mesh.mIndexCount = indexCount;

	// Create the vertex buffer and copy the data to it
	sce::Agc::Core::BufferSpec bufSpec;
	VertexType* _vertexBuffer = (VertexType*)Utils::allocDmem({ vertexCount * sizeof(VertexType), sce::Agc::Alignment::kBuffer });
	memcpy(_vertexBuffer, vertices, vertexCount * sizeof(VertexType));
	bufSpec.initAsRegularBuffer(_vertexBuffer, sizeof(VertexType), vertexCount);

	// Register the vertex buffer
	SceError error = sce::Agc::Core::initialize(&mesh.mVertexBuffer, &bufSpec);
	SCE_AGC_ASSERT(error == SCE_OK);
	sce::Agc::Core::registerResource(&mesh.mVertexBuffer, "Vertices");

	// Create the index buffer and copy the data to it
	mesh.mIndexBuffer = Utils::allocDmem({ indexCount * sizeof(uint32_t), sce::Agc::Alignment::kBuffer });
	memcpy(mesh.mIndexBuffer, indices, indexCount * sizeof(uint32_t));
	
	// Store the mesh
	mMeshes.emplace_back(std::move(mesh));

	// Load a default texture to be applied to the mesh
	mTextures.emplace_back();
	Utils::loadTexture(&mTextures[0u], "/app0/DefaultTexture.gnf");
}

void MeshInstance::InitFromPack(const PackFile::Package& packFile)
{
	// Ensure that this package is valid
	SCE_AGC_ASSERT(packFile.gpu_data_size != 0);

	// The header gives the information about how many meshes and textures there will be in total.
	// Resize the vectors so that it is easier to reference (this is lazy, for a fully fledged game avoid).
	mMeshes.resize(packFile.header.meshes_count);
	mTextures.resize(packFile.header.texture_count);

	/* ----- 1. Load all textures ----- */ 
	// If no texture was assigned to this model, assign a default texture
	if (mTextures.size())
	{
		for (uint32_t i = 0u; i < packFile.textures.size(); ++i)
		{
			const PackFile::TextureRef& tex = packFile.textures[i];
			std::string path = "/app0/";
			path.append(tex.path.data());
			Utils::loadTexture(&mTextures[i], path.c_str());
		}
	}
	else
	{
		mTextures.emplace_back();
		Utils::loadTexture(&mTextures[0u], "/app0/DefaultTexture.gnf");
	}

	// The nodes represent a collection of meshes transformed in space
	// The goal will be to loop through all nodes and find all their meshes
	for (const PackFile::Node& node : packFile.nodes)
	{
		/* ----- 2. Get the texture index for the mesh ----- */
		uint32_t nodeTextureIndex = 0u;
		// Only look for a texture index if any texture is provided
		if (packFile.header.texture_count)
		{
			// Multiple materials may be applied to the same mesh(es).
			// But we are only interested in the one that contains our texture index!
			for (size_t i = 0u, albedoFound = false; i < node.material_index.size() && !albedoFound; ++i)
			{
				const uint16_t& matIndex = node.material_index[i];
				const PackFile::Material& mat = packFile.materials[matIndex];
				for (const PackFile::MaterialProperty& prop : mat.properties)
				{
					// Check for albedo. If PBR, you can get the normal map, etc. using the corresponding MaterialPropertiesSemantic
					if (prop.semantic == PackFile::MaterialPropertiesSemantic::e_mps_albedo &&
						PackFile::MaterialPropertyType::get_data_type(prop.type) == PackFile::MaterialPropertyType::e_mpt_texture)
					{
						const PackFile::TextureProperties* texProp = (PackFile::TextureProperties*)prop.value_ptr;
						nodeTextureIndex = texProp->tx_idx;

						// Break here because we are not looking for anything else than albedo
						albedoFound = true;
						break;
					}
				}
			}
		}

		for (const uint16_t& meshIndex : node.meshes)
		{
			const PackFile::Mesh& packMesh = packFile.meshes[meshIndex];
			Mesh& currentMesh = mMeshes[meshIndex];

			// Assign the texture index
			currentMesh.mTextureIndex = nodeTextureIndex;

			const uint32_t vertexCount = packMesh.vertex_count;
			const uint32_t indexCount = packMesh.index_count;
			std::vector<VertexType> vertices(vertexCount);
			std::vector<uint32_t> indices(indexCount);

			/* ----- 3. Get the indices of the mesh ----- */
			// The indices may be supplied in 8, 16 or 32 bits formats.
			// We need to handle proper conversion to 32 bits indices to stay consistent with our pipeline
			const PackFile::Buffer iBuff = packFile.buffers[packMesh.index_buffer];
			char* iBufferPtr = (char*)packFile.gpu_data + iBuff.offset;
			SCE_AGC_ASSERT(packMesh.index_elem_size <= 32);	// This is in bits. We will use the buffer stride for equivalent in bytes.
			for (uint32_t i = 0u; i < iBuff.elem_count; ++i)
			{
				memcpy(&indices[i], iBufferPtr, iBuff.stride); //Implicit type conversion
				iBufferPtr += iBuff.stride;
			}

			/* ----- 4. Get the vertices of the mesh ----- */
			// The vertex attributes define the vertex type. Like the indices, the packfile converter
			// may shrink the data down to occupy less space, so we need to handle proper conversion here
			// as well. Note that our implementation does not cover all types, but should be good enough
			// for a wide variety of models using the converter tool. Feel free to improve our code.
			for (const PackFile::VertexAttribute& attribute : packMesh.attributes)
			{
				// Retrieve the memory location of the first attribute of this type in the buffer
				const PackFile::Buffer bufferDesc = packFile.buffers[packMesh.vertex_buffers[attribute.vertex_buffer_index]];
				char* bufferPtr = (char*)packFile.gpu_data + bufferDesc.offset + attribute.offset;

				switch (attribute.semantic)
				{
				case PackFile::VertexSemantic::e_vx_position:
					SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_float32_3);
					// No conversion required
					for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
					{
						memcpy(&vertices[i].position, bufferPtr, sizeof(float3));
						if(node.parent_node_index >= 0)
							Utils::MatrixMul(vertices[i].position, packFile.nodes[node.parent_node_index].matrix);
						Utils::MatrixMul(vertices[i].position, node.matrix);
						bufferPtr += bufferDesc.stride;
					}
					break;
				case PackFile::VertexSemantic::e_vx_uv_channel:
					SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_float16_2);
					// Convert the 16 bits float type to a 32 bit float
					for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
					{
						usint2 data;
						memcpy(&data, bufferPtr, sizeof(usint2));
						vertices[i].uv = { Utils::halfToFloat(data.x), Utils::halfToFloat(data.y) };
						bufferPtr += bufferDesc.stride;
					}
					break;
				case PackFile::VertexSemantic::e_vx_normal:
					SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
					// Convert the 16 bits signed integer type to a 32 bit float
					for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
					{
						sint3 data;
						memcpy(&data, bufferPtr, sizeof(sint3));
						vertices[i].normal = { Utils::snorm16ToFloat(data.x), Utils::snorm16ToFloat(data.y), Utils::snorm16ToFloat(data.z) };
						if (node.parent_node_index >= 0)
							Utils::MatrixMul3x3(vertices[i].normal, packFile.nodes[node.parent_node_index].matrix);
						Utils::MatrixMul3x3(vertices[i].normal, node.matrix);
						Utils::Normalize(vertices[i].normal);
						bufferPtr += bufferDesc.stride;
					}
					break;
				default:
					continue;
				}
			}

			/* ----- 5. Create the vertex and index buffers on the PlayStation 5 memory ----- */
			// This works exactly the same as with our Cube!
			currentMesh.mIndexCount = indices.size();
			const uint32_t vertexBufferSize = vertices.size() * sizeof(VertexType);
			const uint32_t indexBufferSize = currentMesh.mIndexCount * sizeof(uint32_t);
			VertexType* _vb = (VertexType*)Utils::allocDmem({ vertexBufferSize, sce::Agc::Alignment::kBuffer });
			currentMesh.mIndexBuffer = Utils::allocDmem({ indexBufferSize, sce::Agc::Alignment::kBuffer });

			// Init and register the vertex buffer
			sce::Agc::Core::BufferSpec bufSpec;
			memcpy(_vb, vertices.data(), vertexBufferSize);
			bufSpec.initAsRegularBuffer(_vb, sizeof(VertexType), vertices.size());
			SceError error = sce::Agc::Core::initialize(&currentMesh.mVertexBuffer, &bufSpec);
			SCE_AGC_ASSERT(error == SCE_OK);
			sce::Agc::Core::registerResource(&currentMesh.mVertexBuffer, "Vertices");

			// Init index buffer
			memcpy(currentMesh.mIndexBuffer, indices.data(), indexBufferSize);
		}
	}
}

void MeshInstance::Render(sce::Agc::DrawCommandBuffer* dcb,
	sce::Agc::Core::StateBuffer* sb,
	sce::Agc::Shader* gs,
	sce::Agc::Shader* ps,
	sce::Agc::Core::Buffer* passbuffer,
	sce::Agc::Core::Buffer* objbuffer,
	uint32_t objBufferIndex,	// The index to acces this object's buffer section (world matrix, ...) in the entire object buffer
	sce::Agc::Core::Sampler* sampler)
{
	sce::Agc::Core::StageBinder gsBinder, psBinder;
	gsBinder.init(dcb, dcb).setShader(gs);
	psBinder.init(dcb, dcb).setShader(ps);

	gsBinder.setShader(gs);
	psBinder.setShader(ps);

	// The indices have been specifically converted to 32 bits
	dcb->setIndexSize(sce::Agc::IndexSize::k32);

	/* ----- 6. Render the mesh instance ----- */
	for (const Mesh& mesh : mMeshes)
	{
		const uint32_t numDraws = mesh.mIndexCount / 3u;
		const sce::Agc::Core::Buffer* vertexBuffer = &mesh.mVertexBuffer;
		const sce::Agc::Core::Texture* tex = &mTextures[mesh.mTextureIndex];

		// Send the constant buffer to the vertex shader
		gsBinder.setBuffers(0, 1, vertexBuffer).setConstantBuffers(0, 1, passbuffer).setBuffers(1, 1, objbuffer).setDrawIndex(objBufferIndex);

		// Send the texture and sampler to the pixel shader
		psBinder.setTextures(0, 1, tex).setSamplers(0, 1, sampler);

		// Now that the resources are bound we can issue our draw call.
		dcb->drawIndex(mesh.mIndexCount, mesh.mIndexBuffer, gs->m_specials->m_drawModifier);

		sb->postDraw();
		gsBinder.postBatch();
		psBinder.postBatch();
	}
}
