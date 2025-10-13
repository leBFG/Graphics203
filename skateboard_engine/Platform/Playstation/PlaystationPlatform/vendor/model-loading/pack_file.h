/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 6.00.00.38-00.00.00.0.1
* Copyright (C) 2022 Sony Interactive Entertainment Inc.
* 
*/

//*** Version 0.3.1:
//   Minor fixes and removed warning.

//*** Version 0.3.0:
//   Changed Reader are based on tables,
//     section loader create the serialization based on the version.
//     the read_object deserialize the field based on the table.
//     Now type and field order can be changed by changing the table.
//     Added vertex offset and index offset to the mesh

//*** Version 0.2.7:
//   Added support for BVH
//   Added support for custom stream read
//   Added error code
//
//*** Version 0.2.5:
//   Added Alpha Cutout property
//   Added Gpu and Cpu allocator user data to the LoadOptions
//   Added Skinning support
//   Added Custom loader support
//
//*** Version 0.2.4:
//  Header review:
//    Added section have signature and start on the file
//    Added section in the binary
//
//*** Version 0.2.3:
//  Remove signature from Buffer, as they all the same size
//
//*** Version 0.2.2
//  Mesh review:
//    Separated vertex/index buffer data from mesh descriptor.
//       Mesh data are now a blob at the end of the file.
//       Introduced buffer type that specify an offset in the file.
//*** Version 0.2.1
//  Material review:
//    added illumination model that can be used to indicate to the engine how to shade the material
//    instead of a list of texture, the material has a list of custom properties.
//       properties has type and semantic
//       data are stored in a data blob
//*** Version 0.2.0
//  Mesh review.
//    added vertex stride
//    added vertex buffer size
//    added vertex stride
//    added explitic index size.
//    added primitive type
//    added index buffer size
//    removed last attribute vx_end that was containing the vertex stride.

//*** Version 0.1.6
//  Added material name
//
//*** Version 0.1.5
//  Added 32-bit index support
//
//*** Version 0.1.4
//  Added node support
//
//*** Version 0.1.3
//  Fixed mesh size
//
//*** Version 0.1.2
//  Added bounding box
//
//*** Version 0.1.1
// Basic node support (Place holder)
//
//*** Version 0.1.0
// Basic mesh export
//  Support for materia
//  Texture
//  Mesh

#ifndef PACK_FILE_H
#define PACK_FILE_H

#include "array.h"
#include "material_models.h"

#include <cstdint>
#include <cstdio>

#include "error_codes.h"

static const uint32_t  PACK_VERSION_MAJ = 0;
static const uint32_t  PACK_VERSION_MIN = 3;
static const uint32_t  PACK_VERSION_BUILD = 1;

namespace PackFile
{

    constexpr uint32_t MAKE_VERSION(uint32_t maj, uint32_t min, uint32_t build)
    {
        return (maj<<24) | (min<<16) | build;
    }

    constexpr uint32_t MAKE_MAGIC(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    {
        return (uint32_t) ((d<<24) | (c<<16) | (b<<8) | a);
    }

    static const uint32_t k_signature_0 = MAKE_MAGIC('S','I','E', '-');
    static const uint32_t k_signature_1 = MAKE_MAGIC('P', 'A', 'C', 'K');

    static const uint32_t k_string_header   = MAKE_MAGIC('S', 'T', 'R', '_');
    static const uint32_t k_buffer_header   = MAKE_MAGIC('B', 'U', 'F', 'F');
    static const uint32_t k_mesh_header     = MAKE_MAGIC('M', 'E', 'S', 'H');
    static const uint32_t k_material_header = MAKE_MAGIC('M', 'A', 'T', 'E');
    static const uint32_t k_node_header     = MAKE_MAGIC('N', 'O', 'D', 'E');
    static const uint32_t k_texture_header  = MAKE_MAGIC('T', 'E', 'X', '_');
    static const uint32_t k_skeleton_header = MAKE_MAGIC('S', 'K', 'E', 'L');

    static const uint32_t k_version = MAKE_VERSION(PACK_VERSION_MAJ, PACK_VERSION_MIN, PACK_VERSION_BUILD);


    // The enumerated type identifying a vertex or mesh attribute.
    enum VertexSemantic : uint8_t
    {
        //! Vertex attributes (per vertex).
        e_vx_position,                   // The vertex position.
        e_vx_normal,                     // The vertex normal.
        e_vx_tangent,                    // The vertex tangent.
        e_vx_color,                      // The vertex color.
        e_vx_uv_channel,                 // The vertex texture coordinates.
        e_vx_bone_indices,               // Skinning bone index
        e_vx_bone_weights                // Skinning bone weight
    };

    // The enumerated type identifying the data format for a vertex attribute.
    enum VertexAttribFormat : uint8_t
    {
        e_float16_2,                     // The data is 2d half float format.
        e_float16_4,                     // The data is 4d half float format.
        e_float32,                       // The data is float format.
        e_float32_2,                     // The data is 2d float format.
        e_float32_3,                     // The data is 3d float format.
        e_float32_4,                     // The data is 4d half float format.
        e_s_int8_4,                      // The data is 4d 8-bit signed integer format.
        e_s_int16_2,                     // The data is 2d 16-bit signed integer format.
        e_s_int16_4,                     // The data is 4d 16-bit signed integer format.
        e_s_int32,                       // The data is 32-bit signed integer format.
        e_s_int32_2,                     // The data is 2d 32-bit signed integer format.
        e_s_int32_3,                     // The data is 3d 32-bit signed integer format.
        e_s_int32_4,                     // The data is 4d 32-bit signed integer format.
        e_u_int8_4,                      // The data is 4d 8-bit unsigned integer format.
        e_u_int16_2,                     // The data is 2d 16-bit unsigned integer format.
        e_u_int16_4,                     // The data is 4d 16-bit unsigned integer format.
        e_u_int32,                       // The data is 32-bit unsigned integer format.
        e_u_int32_2,                     // The data is 2d 32-bit unsigned integer format.
        e_u_int32_3,                     // The data is 3d 32-bit unsigned integer format.
        e_u_int32_4,                     // The data is 4d 32-bit unsigned integer format.
        e_sn_int8_4,                     // The data is 4d 8-bit signed normalized integer format.
        e_sn_int16_2,                    // The data is 2d 16-bit signed normalized integer format.
        e_sn_int16_4,                    // The data is 4d 16-bit signed normalized integer format.
        e_un_int8_4,                     // The data is 4d 8-bit unsigned normalized integer format.
        e_un_int16_2,                    // The data is 2d 16-bit unsigned normalized integer format.
        e_un_int16_4,                    // The data is 4d 16-bit unsigned normalized integer format.

        e_float11_11_10,                 // The data is 3d float 11 bit RG 10 bit B format.
        e_sn_int11_11_10,                // The data is 3d signed normalized 11 bit RG 10 bit B integer format.
        e_un_int11_11_10,                // The data is 3d unsigned normalized 11 bit RG 10 bit B integer format.
        e_sn_int10_10_10_2,              // The data is 4d signed normalized 10 bit RGB 2 bit alpha integer format.
        e_un_int10_10_10_2,              // The data is 4d unsigned normalized 10 bit RGB 2 bit alpha integer format.
        e_none = 0xFF                    // The data has no format.
    };

    enum MaterialFlags : uint16_t
    {
        e_mf_alpha_clip  = 1 << 0,
        e_mf_alpha_blend = 1 << 1,
        e_mf_background  = 1 << 2,
        e_mf_two_side    = 1 << 3,
    };

    struct MaterialPropertyType
    {
        enum DataType : uint8_t // 4 bits
        {
            e_mpt_char    = 0x00,
            e_mpt_short   = 0x01,
            e_mpt_int     = 0x02,
            e_mpt_uchar   = 0x03,
            e_mpt_ushort  = 0x04,
            e_mpt_uint    = 0x05,
            e_mpt_half    = 0x06,
            e_mpt_float   = 0x07,
            e_mpt_texture = 0x0F,  // Texture
        };

        enum Multiplicity : uint8_t // 4 bit
        {
            e_mpt_scalar  = 0x00,
            e_mpt_vec2    = 0x01,
            e_mpt_vec3    = 0x02,
            e_mpt_vec4    = 0x03,
            e_mpt_mat2x2  = 0x04,
            e_mpt_mat2x3  = 0x05,
            e_mpt_mat2x4  = 0x06,
            e_mpt_mat3x2  = 0x07,
            e_mpt_mat3x3  = 0x08,
            e_mpt_mat3x4  = 0x09,
            e_mpt_mat4x2  = 0x0A,
            e_mpt_mat4x3  = 0x0B,
            e_mpt_mat4x4  = 0x0C
        };

        enum TextureChannel : uint8_t // 4 bit (overlapping with the others)
        {
            e_mpt_textureR    = 0x01,
            e_mpt_textureG    = 0x02,
            e_mpt_textureB    = 0x04,
            e_mpt_textureA    = 0x08,
            e_mpt_textureRG   = e_mpt_textureR | e_mpt_textureG,
            e_mpt_textureRB   = e_mpt_textureR | e_mpt_textureB,
            e_mpt_textureRA   = e_mpt_textureR | e_mpt_textureA,
            e_mpt_textureGA   = e_mpt_textureG | e_mpt_textureA,
            e_mpt_textureGB   = e_mpt_textureR | e_mpt_textureB,
            e_mpt_textureBA   = e_mpt_textureB | e_mpt_textureA,
            e_mpt_textureRGB  = e_mpt_textureR | e_mpt_textureG | e_mpt_textureB,
            e_mpt_textureRBA  = e_mpt_textureR | e_mpt_textureB | e_mpt_textureA,
            e_mpt_textureRGA  = e_mpt_textureR | e_mpt_textureG | e_mpt_textureA,
            e_mpt_textureGBA  = e_mpt_textureG | e_mpt_textureB | e_mpt_textureA,
            e_mpt_textureRGBA = e_mpt_textureR | e_mpt_textureG | e_mpt_textureB | e_mpt_textureA
        };

        uint16_t rep; // for alignment reason

        static MaterialPropertyType make_value_property(DataType t, Multiplicity m)
        {
            return { (uint16_t)(m | (t << 4)) };
        }
        static MaterialPropertyType make_texture_property(TextureChannel c)
        {
            return { (uint16_t)((e_mpt_texture << 4) | c) };
        }
        static DataType get_data_type(MaterialPropertyType mpt) { return (DataType)(0x0F & (mpt.rep >> 4)); }
        static Multiplicity get_multiplicity(MaterialPropertyType mpt) { return (Multiplicity)(0x0F & mpt.rep); }
        static TextureChannel get_texture_channel(MaterialPropertyType mpt) { return (TextureChannel)(0x0F & mpt.rep); }
    };

    // The description of a single vertex attribute.
    struct VertexAttribute
    {
        VertexAttribFormat format;          // The data format of the attribute
        VertexSemantic semantic;            // The target shader semantic
        uint8_t index;                      // Target shader index
        uint8_t vertex_buffer_index;        // Buffer index
        uint16_t offset;                    // Offset in vertex buffer
    };

    // The file header for the geometry file.
    struct Header
    {
        uint32_t version;                   // Version number for the geometry file.

        uint32_t buffer_count;              // Number of buffers referenced in the geometry file.
        uint32_t meshes_count;              // Number of meshes in the geometry file.
        uint32_t materials_count;           // Number of materials in the geometry file.
        uint32_t nodes_count;               // Number of nodes in the geometry file.
        uint32_t texture_count;             // Number of textures referenced in the geometry file.
        // From version 0.2.5 the size is taken from the array length
    };

    // An axis aligned bounding box in the geometry file.
    struct BoundingBox
    {
        float min_corner[3];             // The minimum extents of the bounding box.
        float max_corner[3];             // The maximum extents of the bounding box.
    };

    struct Buffer
    {
        uint64_t offset = 0; // offset in the gpu data
        uint64_t size = 0;
        uint32_t elem_count = 0;
        uint32_t stride = 0;
    };

    enum class PrimitiveType : uint8_t
    {
        e_none                 = 0x00,
        e_point_list           = 0x01,
        e_line_list            = 0x02,
        e_line_strip           = 0x03,
        e_tri_list             = 0x04,
        e_tri_fan              = 0x05,
        e_tri_strip            = 0x06,
        e_patch                = 0x09,
        e_line_list_adjacency  = 0x0a,
        e_line_strip_adjacency = 0x0b,
        e_tri_list_adjacency   = 0x0c,
        e_tri_strip_adjacency  = 0x0d,
        e_rect_list            = 0x11,
        e_line_loop            = 0x12,
        e_polygon              = 0x15,
    };

    // The mesh from a geometry file.
    struct Mesh
    {
        BoundingBox bounding_box;              // The bounding box for the mesh.

        Array<VertexAttribute> attributes;     // The vertex attributes for the mesh.
        Array<uint32_t> vertex_buffers;
        uint32_t index_buffer;

        uint32_t vertex_count = 0;              // The number of Vertices
        uint32_t index_count = 0;               // The number of index in the mesh.
        uint32_t vertex_offset = 0;             // Offset from the base offset
        uint32_t index_offset = 0;              // Offset from the base index

        uint8_t index_elem_size = 0;            // 8, 16, or 32
        PrimitiveType primitive_type = PrimitiveType::e_tri_list;
        uint16_t skeleton_index = 0xFFFF;

        String name;
    };

    struct MaterialProperty
    {
        MaterialPropertiesSemantic semantic;
        MaterialPropertyType type;
        void const* value_ptr;
    };

    struct TextureProperties
    {
        uint32_t tx_idx;     // Index of the texture to use
        uint32_t uv_idx;     // Uv channel the texture should use
        float uv_matrix[6];  // 2x3 matrix, representing UV transformation
    };

    // A material from the geometry file.
    struct Material
    {
        enum Flags
        {
            alpha_cutout = 1 << 0,
            alpha_blend = 1 << 1,
            background = 1 << 2,
            double_sided = 1 << 3,
        };
        MaterialModel model;
        uint16_t model_version;
        uint16_t flags; // Reserved
        Array<uint8_t>  data_blob;
        Array<MaterialProperty> properties;
        String name;                     // The name of the material.
    };

    // Column major
    struct Float4x4
    {
       float values[16];
    };

    // A node from the geometry file.
    struct Node
    {
        String name;                     // The name of the node.
        int16_t parent_node_index;       // The index of the parent node, -1 if this node is the root.
        Float4x4 matrix;                 // The local transformation matrix for the node.
        Array<uint16_t> meshes;          // The array of mesh Indices to attach to this node.
        Array<uint16_t> material_index;  // The index of the material for each node.
    };

    struct Skeleton
    {
        String name;
        Array<uint16_t> node_index;
        Array<Float4x4> bind_pose;
    };

    struct TextureRef
    {
        String path;                    // Texture path relative to the pack file
    };

    // A geometry package from the geometry file.
    struct Package
    {
        Header header;
        Array<Buffer>     buffers;
        Array<Mesh>       meshes;
        Array<Material>   materials;
        Array<Node>       nodes;
        Array<TextureRef> textures;
        Array<Skeleton>   skeletons;

        void* gpu_data = nullptr;
        size_t gpu_data_size = 0;
    };

    struct Allocator
    {
        void* alloc(size_t size, size_t alignment) const
        {
            return alloc_cb(size, alignment, user_data, header);
        }
        void dealloc(void* ptr) const
        {
            dealloc_cb(ptr, user_data, header);
        }
        void* (*alloc_cb)(size_t size, size_t alignment, void* user_data, uint32_t header) = nullptr;
        void (*dealloc_cb)(void* ptr, void* user_data, uint32_t header) = nullptr; // used in case of failure
        void* user_data = nullptr;
        mutable uint32_t header = 0x00;
    };

    struct SectionLoadParam
    {
        uint32_t section_header;
        BufferFile& section_data;
        uint32_t elements_count;
        Allocator const& cpu_allocator;
        Allocator const& gpu_allocator;
        Array<uint8_t> const& string_table;
        uint32_t pack_version;
    };

    using SectionLoad = int (*)(void* user_data, SectionLoadParam& param);

    struct SectionLoader
    {
        uint32_t section_header;
        SectionLoad loader = nullptr;
        void* loader_data = nullptr;
    };

    struct LoaderOptions
    {
        Allocator cpu_allocator;
        Allocator gpu_allocator;
        Array<SectionLoader> custom_section_loaders;
        bool resolve_names = true;
        bool report_warning = false;
    };

    struct OffsetSize
    {
        uint64_t offset;
        uint64_t size;
    };

    int peek_gpu_data_offset(void const* buffer, size_t buffer_size, OffsetSize& out_data);
    int load_package(void const* buffer, size_t buffer_size, void* gpu_buffer, size_t gpu_buffer_size, Package& out_pack, LoaderOptions const& options = {});
    int load_package(Stream& file, Package& out_pack, LoaderOptions const& options = {});
    int load_package(FILE *file, Package& out_pack, LoaderOptions const& options = {});
    int load_package(char const* filename, Package& out_pack, LoaderOptions const& options = {});

    namespace Serialization
    {   // Internal structures for serialization only
        struct Preamble
        {
            uint32_t signature[2]; // SIE-PACK
            uint64_t gpu_data_offset;
            uint64_t gpu_data_size;
            uint32_t version;
            uint32_t section_count;
            //Section sections[section_count];
        };

        struct Section
        {
            uint32_t signature;
            uint32_t element_count;
            uint64_t offset;
            uint64_t size;
        };
    }
}

#endif // PACK_FILE_H
