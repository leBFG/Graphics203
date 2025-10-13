// Header with utilitary includes, data types and functions
#pragma once
#include <vector>
#include <agc.h>

#include "Skateboard/Mathematics.h"

namespace PackFile
{
	struct Float4x4;
};

struct VertexType
{
	glm::float3 position;
	glm::float2 uv;
	glm::float3 normal;
	glm::float3 tangent;
	glm::float3 bitangent;

};

struct SkinnedVertexType
{
	glm::float3 position;
	glm::float2 uv;
	glm::float3 normal;
	glm::float3 tangent;
	glm::float3 bitangent;
	glm::int4	blendIndices;
	glm::float4 blendWeights;
};

namespace Skateboard
{
	// A function to transform a 3D vector by a 4x4 column major matrix (CLM)
	void MatrixMul(glm::float3& inout_vec, const PackFile::Float4x4& in_matrixCLM);

	// A function to transform a 3D vector by the first 3x3 items of a 4x4 column major matrix
	void MatrixMul3x3(glm::float3& inout_vec, const PackFile::Float4x4& in_matrixCLM);

	// A function to normalize a 3D vector. Note that this and all functions above may already exist in the PS5 SDK!
	void Normalize(glm::float3& normal);

	// https://github.com/microsoft/DirectXMath/blob/main/Inc/DirectXPackedVector.inl
	float halfToFloat(uint16_t float16_value);

	// https://www.yosoygames.com.ar/wp/2018/03/vertex-formats-part-1-compression/
	float snorm16ToFloat(short int v);
}