/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 6.00.00.38-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#ifndef PACK_FILE_MATERIAL_MODEL_H
#define PACK_FILE_MATERIAL_MODEL_H

// Diffuse model
// e_mt_diffuse_only
// Semantics
// albedo     default Color(1, 1, 1)
// normal map (optional, default null)
#include <cstdint>

namespace PackFile
{

enum MaterialModel : uint16_t
{
   e_mt_diffuse_only  = 0,
   e_mt_phong         = 1,
   e_mt_pbr_metalness = 5,
   e_mt_pbr_specular  = 6,
   e_mt_unlit         = 8,
   e_mt_toon_shading  = 9
};

#ifndef _MSC_VER
static const MaterialModel k_all_material_models [] = {
   e_mt_diffuse_only,
   e_mt_phong,
   e_mt_pbr_metalness,
   e_mt_pbr_specular,
   e_mt_unlit,
   e_mt_toon_shading,
};
#endif

enum MaterialPropertiesSemantic : uint16_t
{
   e_mps_albedo,
   e_mps_normal,            // Texture only
   e_mps_metalness,
   e_mps_emissive,
   e_mps_roughness,
   e_mps_roughtness  [[deprecated]] = e_mps_roughness,
   e_mps_opacity,
   e_mps_reflectivity,
   e_mps_specular,          // Phong
   e_mps_shininess,         // Phong
   e_mps_ambient,
   e_mps_transparency,
   e_mps_alpha_cutout,      // Value only
   e_mps_normal_strength,   // Value only
   e_mps_ambient_occlusion, // texture only
   e_mps_base_color,
   e_mps_unknown = 0xFFFF,
};

}

#endif