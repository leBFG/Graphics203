#pragma once

#include "Skateboard/Graphics/InternalFormats.h"
#include "Skateboard/Mathematics.h"
#include "GPUResource.h"
#include "Box2D/include/box2d/b2_types.h"

namespace Skateboard
{
	class Buffer;
	class TextureBuffer;
	class MemoryResource;


	typedef std::shared_ptr<MemoryResource> MemoryResourceRef;

	typedef std::shared_ptr<Buffer> BufferRef;
	typedef std::shared_ptr<TextureBuffer> TextureBufferRef;

	enum VertexAttributeType : uint8_t
	{
		PerVertexData,
		PerInstanceData
	};

	enum VertexSemantic : uint8_t
	{
		//! Vertex attributes (per vertex).
		POSITION,						// The vertex position.
		NORMAL,							// The vertex normal.
		TANGENT,						// The vertex tangent.
		COLOUR,							// The vertex color.
		TEXCOORD,						// The vertex texture coordinates.
		BONE_INDEX,						// Skinning bone index / also known as JOINT
		BONE_WEIGHTS,		            // Skinning bone weight

//		BINORMAL,						// The vertex binormal.
		CUSTOM							// Whatever you wanna shove in here
	};

	struct BufferElement
	{
		VertexSemantic Semantic;			// Name of the semantic used in the shader (careful with different APIs!)
		uint8_t SemanticIndex;			
		ShaderDataType_ DataType;			// Abstraction of the shader data type of this element
		VertexAttributeType AttributeType;	// determines weather the input assembly should fetch this data as a per vertex or per instance type
		uint8_t InputSlot;					// The VB input slot of this element for this semantic
		uint32_t Size;						// The size of the element (obtained automatically)
		uint32_t Offset;					// The offset of the element in the overall layout (obtained automatically)

		BufferElement() = default;
		BufferElement(VertexSemantic semantic, ShaderDataType_ type, uint8_t SemanticIdx = 0, uint8_t inputSlot = 0, VertexAttributeType VAT = PerVertexData) :
			Semantic(semantic), SemanticIndex(SemanticIdx), DataType(type), AttributeType(VAT), InputSlot(inputSlot),
			Size(ShaderDataTypeSizeInBytes(type)), Offset(0)
		{
		}

		/// <summary>
		/// An shader element contains one or multiple components. For instance:
		///		- A float3 contains 3 components (3 floats)
		///		- A int2 contains 2 components (2 ints)
		///		- A float4x4 contains 16 components (4x4 floats)
		/// This function retrieves this information outside of an API context.
		/// </summary>
		/// <returns>Returns the number of components present in this element</returns>
		uint32_t GetComponentCount() const;
	};

	class BufferLayout
	{
	public:
		BufferLayout(){};
		BufferLayout(const std::initializer_list<BufferElement>& elements) :
			v_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		/// <summary>
		/// Function to get the default layout for graphics applicaitons.
		/// Custom layouts can be created using the same pattern used in this function for simpler/more complicated layouts
		/// (for instance when using skinning). For static geometry this layout is perfect as it gives all the necessary
		/// elements to render high quality graphics!
		/// </summary>
		/// <returns>The default BufferLayout for unskinned geometry</returns>
		static BufferLayout GetDefaultLayout()
		{
			return {
				{ POSITION,	ShaderDataType_::Float3 },
				{ TEXCOORD,   ShaderDataType_::Float2 },
				{ NORMAL,     ShaderDataType_::Float3 },
				{ TANGENT,    ShaderDataType_::Float3 },
			};
		}

		/// <summary>
		/// Retrieves the vertex layout input for simple animated skinned mesh. This layout 
		/// can be used across all APIs.
		/// </summary>
		/// <returns>The default BufferLayout for skinned geometry</returns>
		/*static BufferLayout GetDefaultSkinnedMeshLayout()
		{
			return {
				{ "POSITION",		ShaderDataType_::Float3 },
				{ "TEXCOORD_0",		ShaderDataType_::Float2 },
				{ "NORMAL",			ShaderDataType_::Float3 },
				{ "TANGENT",		ShaderDataType_::Float3 },
				{ "BITANGENT",		ShaderDataType_::Float3 },
				{ "BLENDINDICES",	ShaderDataType_::Int4 },
				{ "BLENDWEIGHTS",	ShaderDataType_::Float4 },
			};
		}*/

		/// <summary>
		/// Retrieve the different elements in this layout as a vector.
		/// See BufferElement for an overview of what elements contain.
		/// </summary>
		/// <returns></returns>
		const std::vector<BufferElement>& GetElements() const { return v_Elements; }
		/// <summary>
		/// Retrieves the stride for a particular input slot.
		/// </summary>
		/// <returns>The overall stride of elements bound to the same input slot, in bytes, by default returns slot 0</returns>
		uint32_t GetStride(uint8_t slot = 0) const;
		/// <summary>
		///	Return stride of the layout, as if all elements are bound to the same input slot.
		/// </summary>
		uint32_t GetAbsoluteStride() const;
		/// <summary>
		///	Return the number of elements in this layout.
		/// </summary>
		size_t GetElementCount() const { return v_Elements.size(); }
		/// <summary>
		///	Return numbers of vertex buffer slots this layout would use when bound to the input assembler.
		/// </summary>
		size_t GetVertexBufferCount() const { return m_SlotsAndStrides.size(); }
		/// </summary>
		///	Return the map of vertex buffer slots and their respective strides.
		/// </summary>
		std::map<uint8_t, uint32_t> GetSlotsAndStrides() const { return m_SlotsAndStrides; }


		// Iterators for nice C++ functionalities
		std::vector<BufferElement>::iterator begin() { return v_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return v_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return v_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return v_Elements.end(); }

	private:
		// This function is called privately to calculate the internal offsets of the different elements
		// as well as the overall stride of the layout. These calculation can only be performed once the
		// layout contains all the elements that define it.
		void CalculateOffsetsAndStride();


	private:
		std::vector<BufferElement> v_Elements;
		std::map<uint8_t, uint32_t> m_SlotsAndStrides;
		//uint32_t m_Stride;
	};

	struct BufferDesc
	{
		ResourceAccessFlag_ AccessFlags;
		BufferFlags_ Flags;
		uint64_t Size;
		
		//void* pInitialDataToTransfer;

		void Init(uint64_t SizeInBytes, ResourceAccessFlag_ accessFlags = ResourceAccessFlag_CpuWrite | ResourceAccessFlag_GpuRead | ResourceAccessFlag_DesktopPlatformPrimaryResidenceGPU, BufferFlags_ bufferFlags = BufferFlags_NONE)
		{
			Size = SizeInBytes;
			AccessFlags = accessFlags;
			Flags = bufferFlags;
			//pInitialDataToTransfer = pDataToTransfer;
		};
	};

	class MemoryResource : public GPUResource
	{
	public :
		uint32_t GetAccessFlags() const { return AccessFlags; };
	protected:
		//Pass-through constructor
		template<typename ... Args>
		MemoryResource(Args&&... args) : GPUResource(std::forward<Args>(args)...) {}

		ResourceAccessFlag_ AccessFlags{};
	};

	class Buffer : public MemoryResource //, public CPUInterface
	{
	public:
		uint64_t Size{};
		uint64_t Alignment{};
		BufferFlags_ Flags{};

		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_Buffer; }

	protected:
		Buffer(const BufferDesc& desc)
		{
			Size = desc.Size;
			Flags = desc.Flags;
			AccessFlags = desc.AccessFlags;
		};
	};

	union ClearValue
	{
		glm::vec4 RenderClearValue;
		struct
		{
			float Depth;
			uint32_t Stencil;
		};
	};

	struct TextureDesc
	{
		DataFormat_ Format;
		ResourceAccessFlag_ AccessFlags;
		TextureType_ Type;
		TextureDimension_ Dimension;

		uint64_t Width;
		uint32_t Height;
		uint32_t Depth;
		uint32_t Mips;

		ClearValue Clear;
	};

	class TextureBuffer : public MemoryResource
	{
	protected:
		TextureBuffer(const TextureDesc& desc) 
		{
			Format = desc.Format;
			AccessFlags = desc.AccessFlags;
			Type = desc.Type;
			Dimension = desc.Dimension;

			Width = desc.Width;
			Height = desc.Height;
			Depth = desc.Depth;
			Mips = desc.Mips;
			Clear = desc.Clear;
		};

		DataFormat_ Format{};
		TextureType_ Type{};
		TextureDimension_ Dimension{};

		ClearValue Clear{};
		
		uint64_t Width{};
		uint32_t Height{};
		uint32_t Depth{};
		uint32_t Mips{};

	public:
		uint64_t GetWidth() const	{ return Width;}
		uint32_t GetHeight() const	{ return Height;}
		uint32_t GetDepth() const	{ return Depth;}
		uint32_t GetMipCount() const { return Mips; }
		ClearValue GetClearValue() const { return Clear; }
		glm::vec4 RTClearValue() const { return Clear.RenderClearValue; }
		float DepthClearValue() const { return Clear.Depth; }
		uint32_t StencilClearValue() const { return Clear.Stencil; }
		TextureType_ GetType() const { return Type; }

		constexpr  GPUResourceType_ GetResourceType() override { return GpuResourceType_Texture; }
	};
}
