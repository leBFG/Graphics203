#include <sktbdpch.h>
#include "D3DAssetManager.h"
#include "Skateboard/Graphics/RHI/ResourceFactory.h"
#include "Skateboard/Assets/AssetManager.h"
#include "Graphics/Resources/D3DBuffer.h"
#include "Graphics/RHI/D3DGraphicsContext.h"

//D3D12TK helpers
#include "vendor/microsoft/ResourceUploadBatch.h"
#include "vendor/microsoft/DDSTextureLoader.h"

//D3D12 MemAloc
#include "D3D12MemAlloc.h"

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>

#define SKTBD_LOG_COMPONENT "D3DAssetManager"
#include "Skateboard/Log.h"

namespace Skateboard
{
	std::string TranslateSemantic(const VertexSemantic& semantic, uint8_t index)
	{
		switch (semantic) {
		default:
		case POSITION:		return "POSITION";
		case NORMAL:		return "NORMAL";
		case TANGENT:		return "TANGENT";
		case COLOUR:		return "COLOR_" +		std::to_string(index);
		case TEXCOORD:		return "TEXCOORD_" +	std::to_string(index);
		case BONE_INDEX:	return "JOINTS_" +		std::to_string(index);
		case BONE_WEIGHTS:	return "WEIGHTS_" +		std::to_string(index);
		//case CUSTOM:		return "_CUSTOM" +		std::to_string(index);
		}
	}

	D3DAssetManager::D3DAssetManager()
	{
	}

	const Texture D3DAssetManager::LoadTextureImpl(const wchar_t* filename, const std::string& textureTag, TextureDimension_ resType)
	{
		if(m_Textures.contains(textureTag)) return m_Textures[textureTag];

		std::filesystem::path path(filename);
		path.replace_extension(L".dds");

		// Check the file and load the texture using Microsoft's DDS library
		std::ifstream inputFile(path.native().c_str());
		if (inputFile.good())
		{
			if (!path.extension().wstring().compare(L".dds"))
			{

				DirectX::ResourceUploadBatch uploadBatch(gD3DContext->GetDevice());

				uploadBatch.Begin();

				D3D12MA::Allocation* allocation;

				DirectX::CreateDDSTextureFromFile(gD3DContext->GetDevice(),gD3DContext->GetMemoryAllocator(), uploadBatch, path.c_str(), &allocation);

				uploadBatch.End(gD3DContext->CommandQueue()).wait();

				//Plan
				// Update DDSLoader from here https://github.com/microsoft/DirectXTK12/wiki/DDSTextureLoader
				// modify to use the D3D12MA Allocator and Fill its Description
				// store D3D12MA::Allocation* in the void* memhandle as the rest of the resources that could be user created;

#ifndef SKTBD_SHIP
				//Texture result()(Resource) ? reinterpret_cast<D3D12MA::Allocation*>(Resource)->GetResource() : nullptr
				allocation->GetResource()->SetName(path.native().c_str());
#endif

				auto ds = allocation->GetResource()->GetDesc();
				TextureDesc desc = {};

				desc.AccessFlags = ResourceAccessFlag_GpuRead;
				desc.Depth = ds.DepthOrArraySize;
				desc.Dimension = resType;
				desc.Format = D3DBufferFormatToSkateboard(ds.Format);
				desc.Height = ds.Height;
				desc.Mips = ds.MipLevels;
				desc.Type = TextureType_Default;
				desc.Width = ds.Width;

				//create skateboard wrapper resource
				auto data = std::make_shared<D3DTextureBuffer>(desc, allocation);
			//	*static_cast<void**>(data->GetResourcePtr()) = allocation;

				TextureViewDesc view{};
				view.ArraySize = ds.DepthOrArraySize;
				view.Dimension = resType;
				view.Format = D3DBufferFormatToSkateboard(ds.Format);
				view.MipLevels = ds.MipLevels;
				view.MostDetailedMip = 0;

				//create skateboard view
				//TextureViewRef textureView;
				m_Textures[textureTag] = ResourceFactory::CreateTextureShaderResourceView(view, data);
			}
			else
			{
				SKTBD_LOG_ASSERT(false, "Unsopported texture type. For now only DDS files are supported.");
				//delete result, result = nullptr;
				return nullptr;
			}
		}
		else
		{
			SKTBD_LOG_ASSERT(false, "Could not load the {} texture, texture not found.", textureTag);
			//delete result, result = nullptr;
			return nullptr;
		}

		// Success
		++m_TextureMaxIndex;
		return m_Textures[textureTag];
	}

	Texture D3DAssetManager::GetDefaultTextureImpl(TextureDimension_ Dimension)
	{
		//if we already have this view return it
		if (m_DefaultTextures.contains(Dimension)) return m_DefaultTextures[Dimension];
		
		//otherwise create it
		TextureViewDesc view{};
		view.ArraySize = 1;
		view.Dimension = Dimension;
		view.Format = DataFormat_R32G32B32A32_FLOAT;
		view.MipLevels = 1;
		view.MipSlice = 0;
		view.ResourceMinLodClamp = 0.f;
		view.MostDetailedMip = 0;

		m_DefaultTextures[Dimension] = ResourceFactory::CreateTextureShaderResourceView(view);

		return m_DefaultTextures[Dimension];
	}

	Mesh* D3DAssetManager::LoadModelImpl(const wchar_t* filename, const std::string& modelTag, const BufferLayout& Layout, bool loadIndexBuffer)
	{
		if (m_Meshes.contains(modelTag)) return m_Meshes[modelTag].get();
		else
		{
			auto PrimTypeToSkateboard = [](const fastgltf::PrimitiveType& type) -> SKTBD_PRIMITIVE_TOPOLOGY
				{
					switch (type)
					{
					case fastgltf::PrimitiveType::Points:		 return SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_POINTLIST;
					case fastgltf::PrimitiveType::Lines:		 return SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_LINELIST;
					case fastgltf::PrimitiveType::LineLoop:		{ SKTBD_MSG_INFO("CANNY DO LINE LOOPS HUN, ITS 2024 - gotta work with line strips"); return SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_LINESTRIP; }
					case fastgltf::PrimitiveType::LineStrip:	 return SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_LINESTRIP;
					case fastgltf::PrimitiveType::Triangles:	 return SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
					case fastgltf::PrimitiveType::TriangleStrip: return SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
					case fastgltf::PrimitiveType::TriangleFan:	 return SKTBD_PRIMITIVE_TOPOLOGY::SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLEFAN;
					default:
						break;
					}
				};

			auto LoadMeshToMemory = [&](fastgltf::Asset& ass, fastgltf::Mesh& mesh, const BufferLayout& RequiredComponents) -> Mesh*
				{
					std::vector<Primitive> primitives(mesh.primitives.size());
					//std::vector<Texture*> meshTextures(ass.textures.size());
					uint32_t index = 0;

					for (auto& p : mesh.primitives) {

						auto& prim = primitives[index]; ++index;

						size_t VBsize = 0;
						size_t IBSize = 0;
						const fastgltf::Accessor& indexaccessor = ass.accessors[p.indicesAccessor.value()];
						const fastgltf::Accessor& vertaccessor = ass.accessors[p.findAttribute("POSITION")->accessorIndex];

						//If loading index buffer we need to determine its size
						if(loadIndexBuffer)
						{
							//load index buffer
							 

							if (indexaccessor.count == 0)
							{
								SKTBD_MSG_WARN("LoadModel::{0} Loading mesh {1}, No index buffer found", modelTag, index)
							}
							else
							{
								SKTBD_MSG_WARN("LoadModel::{0} Loading mesh {1}, indices {2}", modelTag, index, indexaccessor.count)
							}

							//IndexSize
							size_t indexSize = 0;
							if (indexaccessor.componentType == fastgltf::ComponentType::UnsignedInt)
							{
								indexSize = sizeof(uint32_t);
								prim.IndexBuffer.m_Format = IndexFormat::bit32;
							}
							else
							{
								indexSize = sizeof(uint16_t);
								prim.IndexBuffer.m_Format = IndexFormat::bit16;
							}

							IBSize = ROUND_UP(indexaccessor.count * indexSize, GraphicsConstants::BUFFER_ALIGNMENT);
							prim.IndexBuffer.m_IndexCount = indexaccessor.count;
							prim.IndexBuffer.m_ParentResource = m_StaticMeshVertexBuffer;
						}

						if (vertaccessor.count == 0)
						{
							SKTBD_MSG_WARN("LoadModel::{0} Loading mesh {1}, contains no vertices", modelTag, index)
						}
						else
						{
							SKTBD_MSG_WARN("LoadModel::{0} Loading mesh {1}, indices {2}", modelTag, index, vertaccessor.count)
						}

						VBsize = ROUND_UP(vertaccessor.count * RequiredComponents.GetAbsoluteStride(), GraphicsConstants::BUFFER_ALIGNMENT);
						

						size_t PrimitiveSize = IBSize + VBsize;

						MemoryUtils::VIRTUAL_ALLOCATION_DESC desc{};
						desc.Size = PrimitiveSize;
						desc.Alignment = GraphicsConstants::BUFFER_ALIGNMENT;

						prim.Allocations = std::make_shared<PrimitiveSuballocationSingleBuffer>();
						prim.Allocations->ParentAllocator = m_VertexBufferSuballocator;


						uint64_t Offset{};
						m_VertexBufferSuballocator->Allocate(&desc, &prim.Allocations->PrimitiveData, &Offset);

						prim.IndexBuffer.m_Offset = Offset;

						for (auto [slot, Stride] : RequiredComponents.GetSlotsAndStrides())
						{
							prim.VertexBuffers[slot].m_Offset = Offset + IBSize;
							prim.VertexBuffers[slot].m_ParentResource = m_StaticMeshVertexBuffer;
							prim.VertexBuffers[slot].m_VertexCount = vertaccessor.count;
							prim.VertexBuffers[slot].m_VertexStride = Stride;
						}

						prim.Layout = RequiredComponents;

						GraphicsContext::CopyDataToBuffer(m_StaticMeshVertexBuffer.get(), Offset, PrimitiveSize, [&](void* dest)

							{
								if (loadIndexBuffer)
								{
								//write indices
								if (prim.IndexBuffer.m_Format = IndexFormat::bit16)
									fastgltf::iterateAccessorWithIndex<uint16_t>(ass, indexaccessor,
										[&](std::uint16_t idx, size_t i) {
											((uint16_t*)(dest))[i] = idx;
										});
								else
									fastgltf::iterateAccessorWithIndex<std::uint32_t>(ass, indexaccessor,
										[&](std::uint32_t idx, size_t i) {
											((uint32_t*)(dest))[i] = idx;
										});
								}

								//Processed Slot Offset will offset the writer into the next vertex buffer
								auto ProcessedSlotOffset = 0;

								for (auto [slot, Stride] : RequiredComponents.GetSlotsAndStrides())
								{
									for (auto element : RequiredComponents)
									{
										if (slot == element.InputSlot)
										{
											//load vertex data, interlacing it as needed
											auto semantic = TranslateSemantic(element.Semantic, element.SemanticIndex);

											if (!p.findAttribute(semantic))
											{
												SKTBD_MSG_WARN("LoadModel::{0} Vertex info does not contain attribute {1}, skipping...", modelTag, semantic)
													continue;
											}
											else
											{
												SKTBD_MSG_WARN("LoadModel::{0} Found attribute {1}", modelTag, semantic)
											}

											fastgltf::Accessor& accessor = ass.accessors[p.findAttribute(semantic)->accessorIndex];

											if (element.Semantic == POSITION)
											{
												prim.BoundBox.MinX = std::get<std::pmr::vector<double>>(accessor.min)[0];
												prim.BoundBox.MinY = std::get<std::pmr::vector<double>>(accessor.min)[1];
												prim.BoundBox.MinZ = std::get<std::pmr::vector<double>>(accessor.min)[2];

												prim.BoundBox.MaxX = std::get<std::pmr::vector<double>>(accessor.max)[0];
												prim.BoundBox.MaxY = std::get<std::pmr::vector<double>>(accessor.max)[1];
												prim.BoundBox.MaxZ = std::get<std::pmr::vector<double>>(accessor.max)[2];
											}

											//Handle Bizzaroland that is Loading Tangent Information, Blender Saves bitangent sign when a tangent is saved in Vec4.w 
											if (element.Semantic == TANGENT)
											{
												if (accessor.type == fastgltf::AccessorType::Vec4)
												{
													switch (element.DataType)
													{
													case ShaderDataType_::Float3:
														SKTBD_MSG_INFO("Tangent Info found as float4, W Component Was Discarded")
															fastgltf::iterateAccessorWithIndex<glm::vec4>(ass, accessor,
																[&](glm::vec4 value, size_t index) {
																	*(glm::vec3*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = glm::vec3(value.x, value.y, value.z);
																});
														break;
													case ShaderDataType_::Float4:
														fastgltf::iterateAccessorWithIndex<glm::vec4>(ass, accessor,
															[&](glm::vec4 value, size_t index) {
																*(glm::vec4*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
															});
														break;
													}
												}
												else if (accessor.type == fastgltf::AccessorType::Vec3)
												{
													switch (element.DataType)
													{
													case ShaderDataType_::Float3:
														fastgltf::iterateAccessorWithIndex<glm::vec3>(ass, accessor,
															[&](glm::vec3 value, size_t index) {
																*(glm::vec3*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = glm::vec3(value.x, value.y, value.z);
															});
														break;
													case ShaderDataType_::Float4:
														fastgltf::iterateAccessorWithIndex<glm::vec3>(ass, accessor,
															[&](glm::vec3 value, size_t index) {
																*(glm::vec4*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride() + element.Offset]) = glm::vec4(value.x, value.y, value.z, 0);
															});
														break;
													}
												}

												continue;
											}

											switch (element.DataType)
											{
											case ShaderDataType_::Bool:
												fastgltf::iterateAccessorWithIndex<uint8_t>(ass, accessor,
													[&](uint8_t value, size_t index) {
														*(uint8_t*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Int:
												fastgltf::iterateAccessorWithIndex<int>(ass, accessor,
													[&](int value, size_t index) {
														*(int*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Int2:
												fastgltf::iterateAccessorWithIndex<glm::u32vec2>(ass, accessor,
													[&](glm::u32vec2 value, size_t index) {
														*(glm::u32vec2*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Int3:
												fastgltf::iterateAccessorWithIndex<glm::u32vec3>(ass, accessor,
													[&](glm::u32vec3 value, size_t index) {
														*(glm::u32vec3*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Int4:
												fastgltf::iterateAccessorWithIndex<glm::u32vec4>(ass, accessor,
													[&](glm::u32vec4 value, size_t index) {
														*(glm::u32vec4*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Uint:
												fastgltf::iterateAccessorWithIndex<uint32_t>(ass, accessor,
													[&](uint32_t value, size_t index) {
														*(uint32_t*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Uint2:
												fastgltf::iterateAccessorWithIndex<glm::u32vec2>(ass, accessor,
													[&](glm::u32vec2 value, size_t index) {
														*(glm::u32vec2*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Uint3:
												fastgltf::iterateAccessorWithIndex<glm::u32vec3>(ass, accessor,
													[&](glm::u32vec3 value, size_t index) {
														*(glm::u32vec3*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Uint4:
												fastgltf::iterateAccessorWithIndex<glm::u32vec4>(ass, accessor,
													[&](glm::u32vec4 value, size_t index) {
														*(glm::u32vec4*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Float:
												fastgltf::iterateAccessorWithIndex<float>(ass, accessor,
													[&](float value, size_t index) {
														*(float*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Float2:
												fastgltf::iterateAccessorWithIndex<glm::vec2>(ass, accessor,
													[&](glm::vec2 value, size_t index) {
														*(glm::vec2*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Float3:
												fastgltf::iterateAccessorWithIndex<glm::vec3>(ass, accessor,
													[&](glm::vec3 value, size_t index) {
														*(glm::vec3*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;
											case ShaderDataType_::Float4:
												fastgltf::iterateAccessorWithIndex<glm::vec4>(ass, accessor,
													[&](glm::vec4 value, size_t index) {
														*(glm::vec4*)(&((uint8_t*)dest)[ProcessedSlotOffset + IBSize + index * RequiredComponents.GetStride(slot) + element.Offset]) = value;
													});
												break;

											default:
												break;
											}
										}
									}

									ProcessedSlotOffset += Stride * vertaccessor.count;
								}
							}
						);

						SKTBD_MSG_INFO("LoadModel::Primitive::AABB:: MIN X {} Y {} Z {}, MAX X {} Y {} Z {}", prim.BoundBox.MinX, prim.BoundBox.MinY, prim.BoundBox.MinZ, prim.BoundBox.MaxX, prim.BoundBox.MaxY, prim.BoundBox.MaxZ);
					};

					m_Meshes[modelTag].reset();
					m_Meshes[modelTag] = std::make_unique<Mesh>(primitives);

					return m_Meshes[modelTag].get();
			};

			fastgltf::Parser parser;

			std::filesystem::path path(filename);
			path.replace_extension(L".gltf");

			auto data = fastgltf::GltfDataBuffer::FromPath(path);
			if (data.error() != fastgltf::Error::None) {
				// The file couldn't be loaded, or the buffer could not be allocated.
				SKTBD_MSG_ERROR("LoadModel::{0} Asset not found, abork", modelTag);
				return nullptr;
			}

			auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::LoadExternalBuffers);
			if (auto error = asset.error(); error != fastgltf::Error::None) {
				// Some error occurred while reading the buffer, parsing the JSON, or validating the data.
				return nullptr;
			}

			SKTBD_MSG_WARN("LoadModelObject::{0}", asset.get().meshes[0].name);
			return LoadMeshToMemory(asset.get(), asset.get().meshes[0],Layout);
			
		};
		
	}

	Animation* D3DAssetManager::LoadAnimationImpl(const wchar_t* filename, const std::string& animationTag)
	{
		return nullptr;
	}
	
	Skeleton* D3DAssetManager::LoadSkeletonImpl(const wchar_t* filename, const std::string& skeletonTag)
	{
		return nullptr;
	}
	Texture D3DAssetManager::CreateTextureFromDataImpl(const std::string& textureTag, const TextureDesc& desc, void* data)
	{
		return nullptr;
	}

	ImFont* D3DAssetManager::LoadFontImpl(const wchar_t* filename, uint32_t sizeInPixels, const std::string& modelTag)
	{
		if (!m_Fonts.contains(modelTag))
		{
			m_Fonts[modelTag] = ImGui::GetIO().Fonts->AddFontFromFileTTF(ToString(filename).c_str(), sizeInPixels);
		}

		return m_Fonts[modelTag];
	}

	std::vector<MeshData> D3DAssetManager::LoadModelCPUImpl(const wchar_t* filename, const BufferLayout& Layout, bool loadIndexBuffer)
	{
		std::vector<MeshData> meshes;



		return meshes;
	}
}
