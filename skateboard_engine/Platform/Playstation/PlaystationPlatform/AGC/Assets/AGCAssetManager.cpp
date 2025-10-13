#include "sktbdpch.h"
#include "AGCAssetManager.h"
#include "AGC/Graphics/RHI/AGCGraphicsContext.h"

#include "Skateboard/Utilities/StringConverters.h"
#include "AGC/Graphics/AGCTypes.h"

// Include helper files for unloading pack files
#include "vendor/model-loading/pack_file.h"
#include "vendor/model-loading/error_codes.h"
#include "vendor/model-loading/array.h"
#include "vendor/model-loading/material_models.h"

#include <edgeanim.h>
//#include <edge/libedgeanimtool_skeleton.h> // Include these to parse the EdgeAnimSkeleton header
//#include <edge/libedgeanimtool_animation.h>// and EdgeAnimAnimation respectively. 

//#pragma comment(lib, "libSceEdgeAnimTool.a")
//#include  "Skateboard/Renderer/Pipeline.h"

#include "AGC/ImGui/imgui_impl_ps.h"

#define SKTBD_LOG_COMPONENT "ASSET_MANAGER_IMPL"

#ifdef SKTBD_PLATFORM_PLAYSTATION
	//Playstation doesnt have environmental variables hence we jinx it
char* getenv(const char*) { return nullptr; }
#endif

#include "AGC/Graphics/AGCF.h"
#include "AGC/Graphics/Resources/AGCBuffer.h"
#include "AGC/Graphics/Resources/AGCView.h"
#include "Animation/AGCAnimation.h"
#include "Model/Utils.h"
#include "Skateboard/Log.h"

//FIXME :: ALL OF THIS SHIT

namespace Skateboard
{
	AGCAssetManager::AGCAssetManager()
	{
		//NO DEFAULT TEXTURES INSTEAD DEFAULT DESCRIPTORS THAT ARE JUST EMPTY, IE NULL DESCRIPTORS
	}

	//Texture* AGCAssetManager::CreateTextureFromDataImpl(const std::string_view& textureTag, const uint32_t& width, const uint32_t& height, void* data)
	//{
	//	// Need to find an ideal way of creating simple textures by using raw data pointers through AGC

	//	//std::wstring filename = L"PixelTexture";
	//	//
	//	//// To get a Core::Texture out of the binary blob, we can simply use a translate.
	//	//sce::Agc::Core::Texture tex;
	//	//uint64_t* data2 = new uint64_t(0xffffffffffffffff);
	//	//tex.setDataAddress(data2);
	//	//tex.setBorderColorTableSwizzle(sce::Agc::Core::Texture::BorderColorSwizzle::kRGBA);
	//	//tex.setFormat(sce::Agc::Core::TypedFormat::k8_8_8_8Srgb);
	//	//tex.setPrtDefaultColor(sce::Agc::Core::Texture::PrtDefaultColor::k1111);
	//	//tex.setWidth(width);
	//	//tex.setHeight(height);
	//	//tex.setType(sce::Agc::Core::Texture::Type::k2d);
	//	//tex.setMetadataAddress(data2);


	//	//// Create a new texture based on the data extrapolated from the file.
	//	//TextureDesc desc = {};
	//	//desc.Width = tex.getWidth();
	//	//desc.Height = tex.getHeight();
	//	//desc.Type = AGCTextureTypeToSkateboard(tex.getType());
	//	//std::unique_ptr<AGCTexture> texture(new AGCTexture(filename, desc, tex));

	//	//// Store the texture id for later use.
	//	//uint32_t id = m_AvailableIndices.back();
	//	//m_IdToTag[id] = textureTag;
	//	//m_TagToId[textureTag] = id;
	//	//m_AvailableIndices.pop_back();

	//	//// Move texture into map for memory management.
	//	//m_Textures.emplace(textureTag, std::move(texture));
	//	//return m_Textures[textureTag].get();
	//	return nullptr;
	//}

	//ImFont* AGCAssetManager::LoadFontImpl(const wchar_t* filename,  uint32_t sizeInPixels, const std::string_view& modelTag)
	//{
	//	std::filesystem::path path(SanitizeFilePath(filename));
	//	SKTBD_LOG_ASSERT(std::filesystem::exists(path), "Font file does not exist!");

	//	std::string string = path.string();
	//	auto font = ImGui_PS::addFontOTF(string.c_str(), sizeInPixels);

	//	m_Fonts.emplace(modelTag,font);

	//	return font;
	//}

	const Texture AGCAssetManager::LoadTextureImpl(const wchar_t* filename, const std::string& textureTag, TextureDimension_ resType)
	{
		if (m_Textures.find(textureTag) != m_Textures.end())
			return m_Textures[textureTag];
		// The filename is converted to chars and we verify that the extension is '.gnf' since
		// the playstation textures are in that format.
		std::filesystem::path path(SanitizeFilePath(filename));
		path.replace_extension(".gnf");


		if (!std::filesystem::exists(path))
		{
			SKTBD_LOG_ASSERT(false, "Texture file does not exist!");
			return GetDefaultTextureImpl(resType);
		}

		std::string pathStr = path.string();

		// For simplicity, this is done with regular kernel file I/O and not with APR.
		SceKernelStat s = {};
		int32_t error, fileID;
		fileID = sceKernelOpen(path.c_str(), SCE_KERNEL_O_RDONLY, 0);
		SCE_AGC_ASSERT_MSG(fileID > 0, "Unable to open texture %s", pathStr.c_str());
		error = sceKernelFstat(fileID, &s);
		SCE_AGC_ASSERT_MSG(error == SCE_OK, "Unable to stat texture %s", pathStr.c_str());

		// Memory is allocated based on the maximum alignment for tiled resources, to allow this code to load all textures.
		// In actual application, determining the correct alignment in advance can save memory.
		// GNF will be aligned according to their own rules as a gnf object can contain multiple textures with their respective mip chains and potential metadata each aligned and ready for use
		//here we are loding the full GNF, which is wastefull but works for now.
		const sce::Agc::SizeAlign sAlign = { (size_t)s.st_size, sce::Agc::Alignment::kMaxTiledAlignment };

		auto memoryHandle = gAGCContext->GetMemAllocator()->AllocateRaw(sAlign, SCE_KERNEL_PROT_GPU_READ | SCE_KERNEL_PROT_CPU_WRITE);
		void* pPoolData = memoryHandle.m_Data;
		sce::Gnf::GnfFileV5* gnf = (sce::Gnf::GnfFileV5*)pPoolData;

		// Read the file into the allocated memory and close the file once completed
		size_t bytes = sceKernelRead(fileID, gnf, s.st_size);
		SCE_AGC_ASSERT_MSG(bytes == s.st_size, "Unable to read texture %s", pathStr.c_str());
		sceKernelClose(fileID);

		// To get a Core::Texture out of the binary blob, we can simply use a translate.
		auto descriptor_handle = gAGCContext->GetDescriptorHeap().Allocate(1);
		auto descriptor = (sce::Agc::Core::Texture*)descriptor_handle.m_DescriptorPtr;

		error = sce::Agc::Core::translate(descriptor, gnf);
		SCE_AGC_ASSERT_MSG(error == SCE_OK, "Unable to decode texture %s", pathStr.c_str());

#ifndef SKTBD_SHIP
		sce::Agc::Core::registerResource(descriptor, "%s", pathStr.c_str());
#endif // !SKTBD_SHIP

		TextureDesc desc = {};

		desc.AccessFlags = ResourceAccessFlag_GpuRead;
		desc.Depth = descriptor->getDepth();
		desc.Dimension = resType;
		desc.Format = AGCBufferFormatToSkateboard(descriptor->getDataFormat());
		desc.Height = descriptor->getHeight();
		desc.Mips = descriptor->getNumMipLevels();
		desc.Type = TextureType_Default;
		desc.Width = descriptor->getWidth();

		//create skateboard wrapper resource
		auto data = std::make_shared<AGCTextureBuffer>(desc, memoryHandle);
		//	*static_cast<void**>(data->GetResourcePtr()) = allocation;

		TextureViewDesc view{};
		view.ArraySize = descriptor->getDepth();
		view.Dimension = resType;
		view.Format = DataFormat_R32G32B32A32_FLOAT;
		view.MipLevels = descriptor->getNumMips();
		view.MostDetailedMip = descriptor->getBaseMipLevel();

		//create skateboard view
		//TextureViewRef textureView;
		m_Textures[textureTag] = std::make_shared<AGCShaderResourceTextureView>(view, descriptor_handle, data);

		// Store the texture id for later use.
		uint32_t id = m_AvailableIndices.back();
		m_IdToTag[id] = textureTag;
		m_TagToId[textureTag] = id;
		m_AvailableIndices.pop_back();

		return m_Textures[textureTag];
	}

	PackFile::VertexSemantic TranslateSemantic(const VertexSemantic& semantic)
	{
		switch(semantic)
		{
		case POSITION:		return PackFile::VertexSemantic::e_vx_position;
		case NORMAL:		return PackFile::VertexSemantic::e_vx_normal;
		case TANGENT:		return PackFile::VertexSemantic::e_vx_tangent;
		case COLOUR:		return PackFile::VertexSemantic::e_vx_color;
		case TEXCOORD:		return PackFile::VertexSemantic::e_vx_uv_channel;
		case BONE_INDEX:	return PackFile::VertexSemantic::e_vx_bone_indices;
		case BONE_WEIGHTS:	return PackFile::VertexSemantic::e_vx_bone_weights;
		case CUSTOM:
		default:			return PackFile::VertexSemantic::e_vx_color;
		}
	}

	std::string TranslateSemanticName(const VertexSemantic& semantic, uint index)
	{
		switch (semantic)
		{
		default:
		case POSITION:		return"POSITION_"	 +		std::to_string(index);
		case NORMAL:		return"NORMAL_"		 +		std::to_string(index);
		case TANGENT:		return"TANGENT_"	 +		std::to_string(index);
		case COLOUR:		return"COLOUR_"		 +		std::to_string(index);
		case TEXCOORD:		return"TEXCOORD_"	 +		std::to_string(index);
		case BONE_INDEX:	return"BONE_INDEX_"	 +		std::to_string(index);
		case BONE_WEIGHTS:	return"BONE_WEIGHTS_"+		std::to_string(index);
		case CUSTOM:		return"CUSTOM_"		 +		std::to_string(index);
					
		}
	}


	Mesh* AGCAssetManager::LoadModelImpl(const wchar_t* filename, const std::string& modelTag, const BufferLayout& RequiredComponents, const std::string& obj_name, bool loadIndexbuffer, bool flipNormals)
	{
		//safeguard against same tags
		if (m_Meshes.contains(modelTag)) return m_Meshes[modelTag].get();

		PackFile::Package packFile = LoadPackage(filename);
		// Ensure that this package is valid
		SCE_AGC_ASSERT_MSG(packFile.gpu_data_size != 0, "EMTY PAKWAGE");

		// The nodes represent a collection of meshes transformed in space
		// The goal will be to loop through all nodes and find all their meshes

		auto FindAttribute = [](const PackFile::Array<PackFile::VertexAttribute>& attributes, const VertexSemantic semantic, uint8_t index) -> int32_t
			{
				auto idx = 0;
				for(auto i : attributes)
				{
					if (i.index == index && i.semantic == TranslateSemantic(semantic)) return idx;
					++idx;
				}
				return -1;
			};

		//		
		auto LoadToMemory = [&](const PackFile::Node& node)-> Mesh*
		{
			std::vector<Primitive> primitives(node.meshes.getCount());

			for (const uint16_t& meshIndex : node.meshes)
			{
				const PackFile::Mesh& packMesh = packFile.meshes[meshIndex];
				Primitive& prim = primitives[meshIndex];

				const uint32_t vertexCount = packMesh.vertex_count;
				const uint32_t indexCount = packMesh.index_count;

				//Check Vertex and index sizes

				if (vertexCount== 0 || indexCount == 0)
				{
					SKTBD_MSG_INFO("LoadModel::{3} Loading mesh {0} within model failed, index or vertex buffer contain no vertices, indices {1}, vertices {2}", meshIndex, indexCount, vertexCount, modelTag)
						continue;
				}
				else
				{
					SKTBD_MSG_WARN("LoadModel::{0} Loading mesh {1}, indices {2}, vertices {3}", modelTag, meshIndex, indexCount, vertexCount)
				}

				//IndexSize
				size_t indexSize = 0;
				if (packMesh.index_elem_size == 32)
				{
					indexSize = sizeof(uint32_t);
					prim.IndexBuffer.m_Format = IndexFormat::bit32;
				}
				else if(packMesh.index_elem_size == 16)
				{
					indexSize = sizeof(uint16_t);
					prim.IndexBuffer.m_Format = IndexFormat::bit16;
				}
				else
				{
					indexSize = sizeof(uint8_t);
					prim.IndexBuffer.m_Format = IndexFormat::bit8;
				}

				size_t IBSize = ROUND_UP(indexCount * indexSize, GraphicsConstants::BUFFER_ALIGNMENT);
				size_t VBsize = ROUND_UP(vertexCount * RequiredComponents.GetStride(), GraphicsConstants::BUFFER_ALIGNMENT);

				size_t PrimitiveSize = IBSize + VBsize;

				MemoryUtils::VIRTUAL_ALLOCATION_DESC desc{};
				desc.Size = PrimitiveSize;
				desc.Alignment = sce::Agc::Alignment::kBuffer; // GraphicsConstants::BUFFER_ALIGNMENT;

				prim.Allocations = std::make_shared<PrimitiveSuballocationSingleBuffer>();
				prim.Allocations->ParentAllocator = m_VertexBufferSuballocator;

				uint64_t Offset;
				m_VertexBufferSuballocator->Allocate(&desc, &prim.Allocations->PrimitiveData, &Offset);

				SKTBD_MSG_INFO("AssetManagerBuffer Offset: {}", Offset);

				prim.IndexBuffer.m_IndexCount = indexCount;
				prim.IndexBuffer.m_Offset = Offset;
				prim.IndexBuffer.m_ParentResource = m_StaticMeshVertexBuffer;

				prim.VertexBuffer.m_Offset = Offset + IBSize;
				prim.VertexBuffer.m_ParentResource = m_StaticMeshVertexBuffer;
				prim.VertexBuffer.m_VertexCount = vertexCount;
				prim.VertexBuffer.m_VertexStride = RequiredComponents.GetStride();

				prim.Layout = RequiredComponents;

				prim.BoundBox.MinX = packMesh.bounding_box.min_corner[0];
				prim.BoundBox.MinY = packMesh.bounding_box.min_corner[1];
				prim.BoundBox.MinZ = packMesh.bounding_box.min_corner[2];

				prim.BoundBox.MaxX = packMesh.bounding_box.max_corner[0];
				prim.BoundBox.MaxY = packMesh.bounding_box.max_corner[1];
				prim.BoundBox.MaxZ = packMesh.bounding_box.max_corner[2];

				SKTBD_MSG_INFO("LoadModel::Primitive::AABB:: MIN X{} Y{} Z{}, MAX X{} Y{} Z{}", prim.BoundBox.MinX, prim.BoundBox.MinY, prim.BoundBox.MinZ, prim.BoundBox.MaxX, prim.BoundBox.MaxY, prim.BoundBox.MaxZ);

				GraphicsContext::CopyDataToBuffer(m_StaticMeshVertexBuffer.get(), Offset, PrimitiveSize, [&](void* dest)
					{
						//HEART STROKE WARNING: following code is a HOT MESS

						//---- - 3. Get the indices of the mesh---- -
							// The indices may be supplied in 8, 16 or 32 bits formats. we write the index buffer format when we load the indices,
							// as index format is specified in index buffer view we dont need to convert it, the loaded one will do

						const PackFile::Buffer iBuff = packFile.buffers[packMesh.index_buffer];
						char* iBufferPtr = (char*)packFile.gpu_data + iBuff.offset;

						//copy indices to destination
						/*switch (prim.IndexBuffer.m_Format) {
						case bit32:
							std::reverse_copy(reinterpret_cast<uint32_t*>(iBufferPtr), reinterpret_cast<uint32_t*>(iBufferPtr + iBuff.size), static_cast<uint32_t*>(dest));
							break;
						case bit16:
							std::reverse_copy(reinterpret_cast<uint16_t*>(iBufferPtr), reinterpret_cast<uint16_t*>(iBufferPtr + iBuff.size), static_cast<uint16_t*>(dest));
							break;
						case bit8:
							std::reverse_copy(reinterpret_cast<uint8_t*>(iBufferPtr), reinterpret_cast<uint8_t*>(iBufferPtr + iBuff.size), static_cast<uint8_t*>(dest));
							break;
						}*/

						memcpy(dest, iBufferPtr, indexCount * indexSize);

						//---- - 4. Get the vertices of the mesh---- -
							// The vertex attributes define the vertex type. Like the indices, the packfile converter
							// may shrink the data down to occupy less space, so we need to handle proper conversion here
							// as well. Note that our implementation does not cover all types, but should be good enough
							// for a wide variety of models using the converter tool. Feel free to improve our code.
						for (auto& element : RequiredComponents)
						{
							auto attr_idx = FindAttribute(packMesh.attributes, element.Semantic, element.SemanticIndex);
							if (attr_idx == -1)
							{
								SKTBD_MSG_WARN("LoadModel::{0} Vertex info does not contain attribute {1}, skipping...", modelTag, TranslateSemanticName(element.Semantic, element.SemanticIndex));
								continue;
							}

							auto& Attribute = packMesh.attributes[attr_idx];

							// Retrieve the memory location of the first attribute of this type in the buffer
							const PackFile::Buffer bufferDesc = packFile.buffers[packMesh.vertex_buffers[Attribute.vertex_buffer_index]];
							char* vBufferPtr = (char*)packFile.gpu_data + bufferDesc.offset + Attribute.offset;

							switch (element.Semantic)
							{
							case POSITION:
								SCE_AGC_ASSERT(Attribute.format == PackFile::VertexAttribFormat::e_float32_3);
								// No conversion required
								for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
								{
									auto data = &((uint8_t*)dest)[IBSize + i * RequiredComponents.GetStride() + element.Offset];
									memcpy(data, vBufferPtr, sizeof(float3));
									//if (node.parent_node_index >= 0)
									//{
									//	MatrixMul(*(float3*)data, packFile.nodes[node.parent_node_index].matrix);
									//}
									//MatrixMul(*(float3*)data, node.matrix);
									vBufferPtr += bufferDesc.stride;
								}
								break;
							case NORMAL:
								SCE_AGC_ASSERT(Attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
								// Convert the 16 bits signed integer type to a 32 bit float
								for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
								{
									glm::int16 intm_data[4];
									memcpy(&intm_data, vBufferPtr, sizeof(glm::int16) * 4);
									auto data = &((uint8_t*)dest)[IBSize + i * RequiredComponents.GetStride() + element.Offset];

									*(float3*)data = { snorm16ToFloat(intm_data[0]), snorm16ToFloat(intm_data[1]), snorm16ToFloat(intm_data[2]) };

									if (node.parent_node_index >= 0)
									{
										MatrixMul3x3(*(float3*)data, packFile.nodes[node.parent_node_index].matrix);
									}
									//MatrixMul3x3(*(float3*)data, node.matrix);
									//Normalize(*(float3*)data);
									//if (flipNormals)
									//	*(float3*)data *= -1.0f;
									vBufferPtr += bufferDesc.stride;
								}
								break;
							case TANGENT:
								SCE_AGC_ASSERT(Attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
								// Convert the 16 bits signed integer type to a 32 bit float
								for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
								{
									glm::int16 intm_data[4];
									memcpy(&intm_data, vBufferPtr, sizeof(glm::int16) * 4);
									auto data = &((uint8_t*)dest)[IBSize + i * RequiredComponents.GetStride() + element.Offset];

									*(float3*)data = { snorm16ToFloat(intm_data[0]), snorm16ToFloat(intm_data[1]), snorm16ToFloat(intm_data[2]) };

									//if (node.parent_node_index >= 0)
									//{
									//	MatrixMul3x3(*(float3*)data, packFile.nodes[node.parent_node_index].matrix);
									//}
									//MatrixMul3x3(*(float3*)data, node.matrix);
									//Normalize(*(float3*)data);
									//if (flipNormals)
									//	*(float3*)data *= -1.0f;
									vBufferPtr += bufferDesc.stride;
								}
								break;
							case COLOUR:
								SCE_AGC_ASSERT(Attribute.format == PackFile::VertexAttribFormat::e_float32_4);
								// No conversion required
								for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
								{
									auto data = &((uint8_t*)dest)[IBSize + i * RequiredComponents.GetStride() + element.Offset];
									memcpy(data, vBufferPtr, sizeof(float4));
									vBufferPtr += bufferDesc.stride;
								}
								break;
							case TEXCOORD:
								SCE_AGC_ASSERT(Attribute.format == PackFile::VertexAttribFormat::e_float16_2);
								// Convert the 16 bits float type to a 32 bit float
								for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
								{
									glm::int16 intm_data[2];
									memcpy(&intm_data, vBufferPtr, sizeof(glm::uint16) * 2);

									auto data = &((uint8_t*)dest)[IBSize + i * RequiredComponents.GetStride() + element.Offset];
									*(float2*)data = {halfToFloat(intm_data[0]), halfToFloat(intm_data[1])};
									vBufferPtr += bufferDesc.stride;
								}
								break;
							case BONE_INDEX:
								SCE_AGC_ASSERT(false); // not supported at the time, write it yourself
								break;
							case BONE_WEIGHTS:
								SCE_AGC_ASSERT(false); // not supported at the time, write it yourself
								break;
							case CUSTOM:
								SCE_AGC_ASSERT(false); // not supported at the time, write it yourself
								break;
							default: ;
							}
						}
					});

				//---- - 5. Create the vertex and index buffers on the PlayStation 5 memory---- -

					// We'll create PS5 resource buffers
				//	std::wstring debugName = L"Mesh " + std::to_wstring(meshIndex) + L" VB";
				//currentMesh.VertexResource = VertexBuffer::Create(debugName, vertices.data(), static_cast<uint32_t>(vertices.size()));

				// Additionally, lets store the vertex data in system memory for CPU access
				//memcpy(currentMesh.Vertices.data(), vertices.data(), vertices.size() * sizeof(VertexType));


				// We'll create PS5 index buffers
				//debugName = L"Mesh " + std::to_wstring(meshIndex) + L" IB";
				//currentMesh.IndexResource = IndexBuffer::Create(debugName, indices.data(), static_cast<uint32_t>(indices.size()));

				// We'll do the same for the indices.
				//memcpy(currentMesh.Indices.data(), indices.data(), indices.size() * sizeof(uint32_t));

				//model->AddPrimitive(currentMesh);
			}

			m_Meshes[modelTag] = std::make_unique<Mesh>(primitives);
			return m_Meshes[modelTag].get();
		};

		//if(!obj_name.empty())
		for (const PackFile::Node& node : packFile.nodes)
		{
			//find a node with a name we are looking for
			if (memcmp(node.name.data(), obj_name.data(), obj_name.size()))
			{
				SKTBD_MSG_WARN("LoadModelObject::{0}", obj_name.c_str());
				return LoadToMemory(node);
			}
		}

		SKTBD_MSG_WARN("LoadModelObject::{0}", packFile.nodes[0].name);
		return LoadToMemory(packFile.nodes[0]);

		//OLD APPROACH, LOADS NODES, NODES ARE PART OF THE SCENE , WE ARE ONLY INTERESTED IN MESHES, WE are also not interested in materials as they depend on renderer
		/*
		
		// ----- 1. Load all textures ----- 

		// std::vector<Texture> textures;
		// textures.resize(packFile.header.texture_count);

		//if (textures.size())
		//{
		//	for (uint32_t i = 0u; i < packFile.textures.size(); ++i)
		//	{
		//		const PackFile::TextureRef& tex = packFile.textures[i];
		//		std::string path = "assets/models/";
		//		path.append(tex.path.data());
		//		auto wpath = ToWString(path);
		//		textures[i] = LoadTextureImpl(wpath.c_str(), packFile.textures.data()->path.data(), TextureDimension_Texture2D);
		//		model->AddTexture(textures[i]);
		//	}
		//}


		// The nodes represent a collection of meshes transformed in space
		// The goal will be to loop through all nodes and find all their meshes
		for (const PackFile::Node& node : packFile.nodes)
		{
			 ----- 2. Get the texture index for the mesh ----- 
			//uint32_t nodeTextureIndex = 0u;

			// Only look for a texture index if any texture is provided
			//if (packFile.header.texture_count)
			//{
			//	// Multiple materials may be applied to the same mesh(es).
			//	// But we are only interested in the one that contains our texture index!
			//	for (size_t i = 0u, albedoFound = false; i < node.material_index.size() && !albedoFound; ++i)
			//	{
			//		const uint16_t& matIndex = node.material_index[i];
			//		const PackFile::Material& mat = packFile.materials[matIndex];
			//		for (const PackFile::MaterialProperty& prop : mat.properties)
			//		{
			//			// Check for albedo. If PBR, you can get the normal map, etc. using the corresponding MaterialPropertiesSemantic
			//			if (prop.semantic == PackFile::MaterialPropertiesSemantic::e_mps_albedo &&
			//				PackFile::MaterialPropertyType::get_data_type(prop.type) == PackFile::MaterialPropertyType::e_mpt_texture)
			//			{
			//				const PackFile::TextureProperties* texProp = (PackFile::TextureProperties*)prop.value_ptr;
			//				nodeTextureIndex = texProp->tx_idx;

			//				// Break here because we are not looking for anything else than albedo
			//				albedoFound = true;
			//				break;
			//			}
			//		}
			//	}
			//}

			for (const uint16_t& meshIndex : node.meshes)
			{
				const PackFile::Mesh& packMesh = packFile.meshes[meshIndex];
				Primitive& currentMesh = primitives[meshIndex];

				// Assign the texture index
				//currentMesh.TextureIndices[meshIndex] = nodeTextureIndex;

				const uint32_t vertexCount = packMesh.vertex_count;
				const uint32_t indexCount = packMesh.index_count;
				std::vector<VertexType> vertices(vertexCount);
				std::vector<uint32_t> indices(indexCount);

				 ----- 3. Get the indices of the mesh ----- 
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

				 ----- 4. Get the vertices of the mesh ----- 
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
							if (node.parent_node_index >= 0)
							{
								MatrixMul(vertices[i].position, packFile.nodes[node.parent_node_index].matrix);
							}
							MatrixMul(vertices[i].position, node.matrix);
							bufferPtr += bufferDesc.stride;
						}
						break;
					case PackFile::VertexSemantic::e_vx_uv_channel:
						SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_float16_2);
						// Convert the 16 bits float type to a 32 bit float
						for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
						{
							glm::uint16 data[2];
							memcpy(&data, bufferPtr, sizeof(glm::uint16)*2);
							vertices[i].uv = { halfToFloat(data[0]), halfToFloat(data[1])};
							bufferPtr += bufferDesc.stride;
						}
						break;
					case PackFile::VertexSemantic::e_vx_normal:
						SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
						// Convert the 16 bits signed integer type to a 32 bit float
						for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
						{
							glm::int16 data[4];
							memcpy(&data, bufferPtr, sizeof(glm::int16)*4);
							vertices[i].normal = { snorm16ToFloat(data[0]), snorm16ToFloat(data[1]), snorm16ToFloat(data[2])};
							if (node.parent_node_index >= 0)
							{
								MatrixMul3x3(vertices[i].normal, packFile.nodes[node.parent_node_index].matrix);
							}
							MatrixMul3x3(vertices[i].normal, node.matrix);
							Normalize(vertices[i].normal);
							if(flipNormals) 
								vertices[i].normal *= -1.0f;
							bufferPtr += bufferDesc.stride;
						}
						break;
					case PackFile::VertexSemantic::e_vx_tangent:
						SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
						// Convert the 16 bits signed integer type to a 32 bit float
						for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
						{
							glm::int16 data[4];
							memcpy(&data, bufferPtr, sizeof(glm::int16) * 4);
							vertices[i].tangent = { snorm16ToFloat(data[0]), snorm16ToFloat(data[1]), snorm16ToFloat(data[2]) };
							if (node.parent_node_index >= 0)
							{
								MatrixMul3x3(vertices[i].tangent, packFile.nodes[node.parent_node_index].matrix);
							}
							MatrixMul3x3(vertices[i].tangent, node.matrix);
							Normalize(vertices[i].tangent);

							// If we have the tangent vector, we can compute the bitangent vector for completeness.
							vertices[i].bitangent = glm::cross(vertices[i].normal, vertices[i].tangent);
							Normalize(vertices[i].bitangent);

							bufferPtr += bufferDesc.stride;
						}
						break;
					default:
						continue;
					}
				}

				 ----- 5. Create the vertex and index buffers on the PlayStation 5 memory ----- 

				// We'll create PS5 resource buffers
				std::wstring debugName = L"Mesh " + std::to_wstring(meshIndex) + L" VB";
				//currentMesh.VertexResource = VertexBuffer::Create(debugName, vertices.data(), static_cast<uint32_t>(vertices.size()));

				// Additionally, lets store the vertex data in system memory for CPU access
				//memcpy(currentMesh.Vertices.data(), vertices.data(), vertices.size() * sizeof(VertexType));


				// We'll create PS5 index buffers
				debugName = L"Mesh " + std::to_wstring(meshIndex) + L" IB";
				//currentMesh.IndexResource = IndexBuffer::Create(debugName, indices.data(), static_cast<uint32_t>(indices.size()));

				// We'll do the same for the indices.
				//memcpy(currentMesh.Indices.data(), indices.data(), indices.size() * sizeof(uint32_t));

				model->AddPrimitive(currentMesh);

			}
		}
		*/

		//}
	}

	PackFile::Package AGCAssetManager::LoadPackage(const wchar_t* filename)
	{
		// Obtain a PackFile::Package from the pack file so we can parse the data
		PackFile::LoaderOptions options = {};
		PackFile::Package pack = {};

		std::filesystem::path path(SanitizeFilePath(filename));
		path.replace_extension(".pack");


		//std::string path = ToString();
		uint32_t err = PackFile::load_package(path.c_str(), pack, options);
		
		if (err != SCE_OK)
		{
			SKTBD_ASSERT(!err, "Invalid filepath");
		}

		return pack;
	}

	Animation* AGCAssetManager::LoadAnimationImpl(const wchar_t* filename, const std::string& animationTag)
	{
		return nullptr;
	}

	SkinnedMesh* AGCAssetManager::LoadSkeletalMeshImpl(const wchar_t* filename, const std::string& meshTag,
		bool flipNormals)
	{
		return nullptr;
	}

	Skeleton* AGCAssetManager::LoadSkeletonImpl(const wchar_t* filename, const std::string& skeletonTag)
	{
		return nullptr;
	}

	Texture AGCAssetManager::CreateTextureFromDataImpl(const std::string& textureTag, const TextureDesc& desc, void* data)
	{
		return nullptr;
	}

	ImFont* AGCAssetManager::LoadFontImpl(const wchar_t* filename, uint32_t sizeInPixels, const std::string& modelTag)
	{
		if (m_Fonts.contains(modelTag)) return m_Fonts[modelTag];
		else
		{
			m_Fonts[modelTag] = ImGui_PS::addFontOTF(SanitizeFilePath(filename).c_str(), sizeInPixels);
			return m_Fonts[modelTag];
		}
	}

	Texture AGCAssetManager::GetDefaultTextureImpl(const TextureDimension_ Dimension)
	{
		//if we already have this view return it
		if (m_DefaultTextures.contains(TextureDimension_Texture2D)) return m_DefaultTextures[TextureDimension_Texture2D];

		//otherwise create it
		TextureViewDesc view{};
		view.ArraySize = 1;
		view.Dimension = TextureDimension_Texture2D;
		view.Format = DataFormat_R32G32B32A32_FLOAT;
		view.MipLevels = 1;
		view.MipSlice = 0;
		view.ResourceMinLodClamp = 0.f;
		view.MostDetailedMip = 0;

		m_DefaultTextures[TextureDimension_Texture2D] = ResourceFactory::CreateTextureShaderResourceView(view);

		return m_DefaultTextures[TextureDimension_Texture2D];
	}

	//SkinnedMesh* AGCAssetManager::LoadSkeletalMeshImpl(const wchar_t* filename, const std::string_view& meshTag, bool flipNormals)
	//{
	//	//PackFile::Package packFile = LoadPackage(filename);
	//	//// Ensure that this package is valid
	//	//SCE_AGC_ASSERT_MSG(packFile.gpu_data_size != 0, "");

	//	//std::unique_ptr<Mesh> model = std::make_unique<Mesh>();

	//	//// The header gives the information about how many meshes and textures there will be in total.
	//	//// Resize the vectors so that it is easier to reference (this is lazy, for a fully fledged game avoid).
	//	//std::vector<SkinnedMesh> meshes;
	//	//meshes.resize(packFile.header.meshes_count);

	//	//std::vector<Texture*> textures;
	//	//textures.resize(packFile.header.texture_count);


	//	//// The nodes represent a collection of meshes transformed in space
	//	//// The goal will be to loop through all nodes and find all their meshes
	//	//for (const PackFile::Node& node : packFile.nodes)
	//	//{
	//	//	/* ----- 2. Get the texture index for the mesh ----- */
	//	//	uint32_t nodeTextureIndex = 0u;
	//	//	// Only look for a texture index if any texture is provided
	//	//	if (packFile.header.texture_count)
	//	//	{
	//	//		// Multiple materials may be applied to the same mesh(es).
	//	//		// But we are only interested in the one that contains our texture index!
	//	//		for (size_t i = 0u, albedoFound = false; i < node.material_index.size() && !albedoFound; ++i)
	//	//		{
	//	//			const uint16_t& matIndex = node.material_index[i];
	//	//			const PackFile::Material& mat = packFile.materials[matIndex];
	//	//			for (const PackFile::MaterialProperty& prop : mat.properties)
	//	//			{
	//	//				// Check for albedo. If PBR, you can get the normal map, etc. using the corresponding MaterialPropertiesSemantic
	//	//				if (prop.semantic == PackFile::MaterialPropertiesSemantic::e_mps_albedo &&
	//	//					PackFile::MaterialPropertyType::get_data_type(prop.type) == PackFile::MaterialPropertyType::e_mpt_texture)
	//	//				{
	//	//					const PackFile::TextureProperties* texProp = (PackFile::TextureProperties*)prop.value_ptr;
	//	//					nodeTextureIndex = texProp->tx_idx;

	//	//					// Break here because we are not looking for anything else than albedo
	//	//					albedoFound = true;
	//	//					break;
	//	//				}
	//	//			}
	//	//		}
	//	//	}

	//	//	// Go through all the mesh nodes
	//	//	for (const uint16_t& meshIndex : node.meshes)
	//	//	{
	//	//		const PackFile::Mesh& packMesh = packFile.meshes[meshIndex];
	//	//		SkinnedMesh& currentMesh = meshes[meshIndex];

	//	//		// Assign the texture index
	//	//		currentMesh.TextureIndex = nodeTextureIndex;

	//	//		const uint32_t vertexCount = packMesh.vertex_count;
	//	//		const uint32_t indexCount = packMesh.index_count;
	//	//		std::vector<SkinnedVertexType> vertices(vertexCount);
	//	//		std::vector<uint32_t> indices(indexCount);

	//	//		/* ----- 3. Get the indices of the mesh ----- */
	//	//		const PackFile::Buffer iBuff = packFile.buffers[packMesh.index_buffer];
	//	//		char* iBufferPtr = (char*)packFile.gpu_data + iBuff.offset;

	//	//		SCE_AGC_ASSERT(packMesh.index_elem_size <= 32);	// This is in bits. We will use the buffer stride for equivalent in bytes.
	//	//		for (uint32_t i = 0u; i < iBuff.elem_count; ++i)
	//	//		{
	//	//			memcpy(&indices[i], iBufferPtr, iBuff.stride); //Implicit type conversion
	//	//			iBufferPtr += iBuff.stride;
	//	//		}
	//	//		 
	//	//		/* ----- 4. Get the vertices of the mesh ----- */
	//	//		for (const PackFile::VertexAttribute& attribute : packMesh.attributes)
	//	//		{

	//	//			// Retrieve the memory location of the first attribute of this type in the buffer
	//	//			const PackFile::Buffer bufferDesc = packFile.buffers[packMesh.vertex_buffers[attribute.vertex_buffer_index]];
	//	//			char* bufferPtr = (char*)packFile.gpu_data + bufferDesc.offset + attribute.offset;

	//	//			switch (attribute.semantic)
	//	//			{
	//	//			case PackFile::VertexSemantic::e_vx_position:
	//	//				SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_float32_3);
	//	//				// No conversion required
	//	//				for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
	//	//				{
	//	//					memcpy(&vertices[i].position, bufferPtr, sizeof(float3));
	//	//					if (node.parent_node_index >= 0)
	//	//					{
	//	//						MatrixMul(vertices[i].position, packFile.nodes[node.parent_node_index].matrix);
	//	//					}
	//	//					MatrixMul(vertices[i].position, node.matrix);
	//	//					bufferPtr += bufferDesc.stride;
	//	//				}
	//	//				break;
	//	//			case PackFile::VertexSemantic::e_vx_uv_channel:
	//	//				SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_float16_2);
	//	//				// Convert the 16 bits float type to a 32 bit float
	//	//				for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
	//	//				{
	//	//					glm::uint16 data[2];
	//	//					memcpy(&data, bufferPtr, sizeof(glm::uint16)*2);
	//	//					vertices[i].uv = { halfToFloat(data[0]), halfToFloat(data[1])};
	//	//					bufferPtr += bufferDesc.stride;
	//	//				}
	//	//				break;
	//	//			case PackFile::VertexSemantic::e_vx_normal:
	//	//				SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
	//	//				// Convert the 16 bits signed integer type to a 32 bit float
	//	//				for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
	//	//				{
	//	//					glm::int16 data[4];
	//	//					memcpy(&data, bufferPtr, sizeof(glm::int16)*4);
	//	//					vertices[i].normal = { snorm16ToFloat(data[0]), snorm16ToFloat(data[1]), snorm16ToFloat(data[2])};
	//	//					if (node.parent_node_index >= 0)
	//	//					{
	//	//						MatrixMul3x3(vertices[i].normal, packFile.nodes[node.parent_node_index].matrix);
	//	//					}
	//	//					MatrixMul3x3(vertices[i].normal, node.matrix);
	//	//					Normalize(vertices[i].normal);
	//	//					if (flipNormals)
	//	//						vertices[i].normal *= -1.0f;
	//	//					bufferPtr += bufferDesc.stride;
	//	//				}
	//	//				break;
	//	//			case PackFile::VertexSemantic::e_vx_tangent:
	//	//				SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
	//	//				// Convert the 16 bits signed integer type to a 32 bit float
	//	//				for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
	//	//				{
	//	//					glm::int16 data[4];
	//	//					memcpy(&data, bufferPtr, sizeof(glm::int16)*4);
	//	//					vertices[i].tangent = { snorm16ToFloat(data[0]), snorm16ToFloat(data[1]), snorm16ToFloat(data[2])};
	//	//					if (node.parent_node_index >= 0)
	//	//					{
	//	//						MatrixMul3x3(vertices[i].tangent, packFile.nodes[node.parent_node_index].matrix);
	//	//					}
	//	//					MatrixMul3x3(vertices[i].tangent, node.matrix);
	//	//					Normalize(vertices[i].tangent);

	//	//					// If we have the tangent vector, we can compute the bitangent vector for completeness.
	//	//					vertices[i].bitangent = glm::cross(vertices[i].normal, vertices[i].tangent);
	//	//					Normalize(vertices[i].bitangent);

	//	//					bufferPtr += bufferDesc.stride;
	//	//				}
	//	//				break;
	//	//			case PackFile::VertexSemantic::e_vx_bone_indices:
	//	//				SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
	//	//				// Convert the 16 bits signed integer type to a 32 bit float
	//	//				for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
	//	//				{
	//	//					glm::int16 data[4];
	//	//					memcpy(&data[0], bufferPtr, sizeof(int16_t) * 4);

	//	//					vertices[i].blendIndices = { data[0], data[1], data[2], data[3]};

	//	//					bufferPtr += bufferDesc.stride;
	//	//				}
	//	//				break;
	//	//			case PackFile::VertexSemantic::e_vx_bone_weights:
	//	//				SCE_AGC_ASSERT(attribute.format == PackFile::VertexAttribFormat::e_sn_int16_4);
	//	//				// Convert the 16 bits signed integer type to a 32 bit float
	//	//				for (uint32_t i = 0; i < bufferDesc.elem_count; ++i)
	//	//				{
	//	//					glm::int16 data[4];
	//	//					memcpy(&data, bufferPtr, sizeof(glm::int16) * 4);

	//	//					vertices[i].blendWeights = { snorm16ToFloat(data[0]), snorm16ToFloat(data[1]), snorm16ToFloat(data[2]), snorm16ToFloat(data[3])};

	//	//					bufferPtr += bufferDesc.stride;
	//	//				}
	//	//				break;
	//	//			default:
	//	//				continue;
	//	//			}
	//	//		}

	//	//		// Create VBs and IBs

	//	//		// We'll create PS5 resource buffers
	//	//		std::wstring debugName = L"Mesh " + std::to_wstring(meshIndex) + L" VB";
	//	//		//currentMesh.VertexResource = VertexBuffer::Create(debugName, vertices.data(), static_cast<uint32_t>(vertices.size()), BufferLayout::GetDefaultSkinnedMeshLayout());

	//	//		// We'll create PS5 index buffers
	//	//		debugName = L"Mesh " + std::to_wstring(meshIndex) + L" IB";
	//	//		//currentMesh.IndexResource = IndexBuffer::Create(debugName, indices.data(), static_cast<uint32_t>(indices.size()));

	//	//		//Skeleton skeleton;

	//	//		///* ----- 5. Get the skeleton of the mesh ----- */
	//	//		//for (const PackFile::Skeleton& packSkeleton : packFile.skeletons)
	//	//		//{
	//	//		//	// Store the name of the skeleton
	//	//		//	skeleton.SetName(packSkeleton.name.data());

	//	//		//	for (auto pose : packSkeleton.bind_pose)
	//	//		//	{
	//	//		//		// convert the joint matrix to glm::mat4x4
	//	//		//		Joint joint;
	//	//		//		joint.InverseBindPose = glm::make_mat4x4(pose.values);
	//	//		//		skeleton.AddJoint(joint);	
	//	//		//	}
	//
	//	//		//}

	//	//		std::unique_ptr<SkinnedMesh> skinnedMesh;
	//	//		skinnedMesh = std::make_unique<SkinnedMesh>(currentMesh);
	//	//		m_SkinnedMesh[meshTag] = std::move(skinnedMesh);
	//	//	}
	//	//}

	//	return m_SkinnedMesh[meshTag].get();
	//}

	//Skeleton* AGCAssetManager::LoadSkeletonImpl(const wchar_t* filename, const std::string_view& skeletonTag)
	//{
	////	std::filesystem::path path(SanitizeFilePath(filename));

	////	SKTBD_LOG_ASSERT(std::filesystem::exists(path), "Skeleton file does not exist!");
	////	std::string pathStr = path.string();

	////	// For simplicity, this is done with regular kernel file I/O and not with APR.
	////	SceKernelStat s = {};
	////	int32_t error, fileID;
	////	fileID = sceKernelOpen(path.c_str(), SCE_KERNEL_O_RDONLY, 0);
	////	SCE_AGC_ASSERT_MSG(fileID > 0, "Unable to open skeleton %s", pathStr.c_str());
	////	error = sceKernelFstat(fileID, &s);
	////	SCE_AGC_ASSERT_MSG(error == SCE_OK, "Unable to stat skeleton %s", pathStr.c_str());

	////	// EdgeAnimSkeleton is 16-byte aligned
	////	const sce::Agc::SizeAlign sAlign = { (uint64_t)s.st_size, sizeof(int16_t) };
	////	auto handle = gAGCContext->GetMemAllocator()->TypedAlignedAllocate<EdgeAnimSkeleton, sizeof(int16_t)>(s.st_size);

	////	// Read the file into the allocated memory and close the file once completed
	////	size_t bytes = sceKernelRead(fileID, handle.data, s.st_size);
	////	SCE_AGC_ASSERT_MSG(bytes == s.st_size, "Unable to read skeleton %s", pathStr.c_str());
	////	sceKernelClose(fileID);

	////	// TODO: Make our own file format for skeletons, mesh and animations.

	////	// Parse the runtime skeleton 
	////	EdgeAnimSkeleton* edgeAnimSkeleton = (EdgeAnimSkeleton*)handle.data;
	////	Skeleton edgeSkeleton;
	//////	ExtractSkeleton(edgeAnimSkeleton, edgeSkeleton);
	////
	////	// Create a new skeleton and store it within the map
	////	std::unique_ptr<Skeleton> skeleton(new Skeleton());

	////	// Store the name of the skeleton
	////	skeleton->SetName(std::string(skeletonTag));

	////	std::vector<Joint>& joints = const_cast<std::vector<Joint>&>(skeleton->GetJoints());

	////	// Extract each joint from the skeleton, and store the name, pose and parent index.
	////	for (int32_t i = 0; i < edgeSkeleton.m_numJoints; ++i) {
	////		Joint joint;
	////		joint.Name = std::to_string(edgeSkeleton.m_jointNameHashes[i]);
	////		joint.ParentId = edgeSkeleton.m_parentIndices[i];
	////		auto basePose = edgeSkeleton.m_basePose[i];

	////		joint.InverseBindPose.Translation.x = basePose.m_translation.m_data[0];
	////		joint.InverseBindPose.Translation.y = basePose.m_translation.m_data[1];
	////		joint.InverseBindPose.Translation.z = basePose.m_translation.m_data[2];

	////		joint.InverseBindPose.Rotation = glm::quat(basePose.m_rotation.m_data[0], basePose.m_rotation.m_data[1],
	////			basePose.m_rotation.m_data[2], basePose.m_rotation.m_data[3]);

	////		joint.InverseBindPose.Scale.x = basePose.m_scale.m_data[0];
	////		joint.InverseBindPose.Scale.y = basePose.m_scale.m_data[1];
	////		joint.InverseBindPose.Scale.z = basePose.m_scale.m_data[2];

	////		skeleton->AddJoint(joint);
	////	}
	////	
	////	m_Skeleton[skeletonTag] = std::move(skeleton);
	//	return m_Skeleton[skeletonTag].get();
	//}

	//Animation* AGCAssetManager::LoadAnimationImpl(const wchar_t* filename, const std::string_view& animationTag)
	//{
	//	//std::filesystem::path path(SanitizeFilePath(filename));

	//	//SKTBD_LOG_ASSERT(std::filesystem::exists(path), "Animation file does not exist!");
	//	//std::string pathStr = path.string();

	//	//// For simplicity, this is done with regular kernel file I/O and not with APR.
	//	//SceKernelStat s = {};
	//	//int32_t error, fileID;
	//	//fileID = sceKernelOpen(path.c_str(), SCE_KERNEL_O_RDONLY, 0);
	//	//SCE_AGC_ASSERT_MSG(fileID > 0, "Unable to open animation %s", pathStr.c_str());
	//	//error = sceKernelFstat(fileID, &s);
	//	//SCE_AGC_ASSERT_MSG(error == SCE_OK, "Unable to stat animation %s", pathStr.c_str());

	//	//// EdgeAnimAnimation is 16-byte aligned
	//	//const sce::Agc::SizeAlign sAlign = { (uint64_t)s.st_size, sizeof(int16_t) };
	//	//auto handle = gAGCContext->GetMemAllocator()->TypedAlignedAllocate<EdgeAnimAnimation,sizeof(int16_t)>(s.st_size);

	//	//// Read the file into the allocated memory and close the file once completed
	//	//size_t bytes = sceKernelRead(fileID, handle.data, s.st_size);
	//	//SCE_AGC_ASSERT_MSG(bytes == s.st_size, "Unable to read animation %s", pathStr.c_str());
	//	//sceKernelClose(fileID);

	//	//std::unique_ptr<Animation> animation(new AGCAnimation(handle));

	//	//m_Animations[animationTag] = std::move(animation);

	//	return nullptr;
	//}


	
}
