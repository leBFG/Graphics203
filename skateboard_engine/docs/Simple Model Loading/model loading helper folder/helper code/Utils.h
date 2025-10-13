// Header with utilitary includes, data types and functions
#pragma once
#include <vector>
#include <agc.h>

namespace PackFile
{
	struct Float4x4;
};

struct float3 {
	float x, y, z;
};

struct float2 {
	float x, y;
};

struct sint3
{
	short int x, y, z;
};

struct usint2
{
	unsigned short int x, y;
};

struct VertexType
{
	float3 position;
	float2 uv;
	float3 normal;
};

namespace Utils
{
	// A function to transform a 3D vector by a 4x4 column major matrix (CLM)
	void MatrixMul(float3& inout_vec, const PackFile::Float4x4& in_matrixCLM);

	// A function to transform a 3D vector by the first 3x3 items of a 4x4 column major matrix
	void MatrixMul3x3(float3& inout_vec, const PackFile::Float4x4& in_matrixCLM);

	// A function to normalize a 3D vector. Note that this and all functions above may already exist in the PS5 SDK!
	void Normalize(float3& normal);

	// https://github.com/microsoft/DirectXMath/blob/main/Inc/DirectXPackedVector.inl
	float halfToFloat(uint16_t float16_value);

	// https://www.yosoygames.com.ar/wp/2018/03/vertex-formats-part-1-compression/
	float snorm16ToFloat(short int v);

	// This is a temporary utility function to allocate direct memory. It's not important to understand how this works.
	uint8_t* allocDmem(sce::Agc::SizeAlign sizeAlign);

	// This is a helper function to load a texture from a Gnf file. It is not important to understand how this works.
	// (From the samples)
	SceError loadTexture(sce::Agc::Core::Texture* tex, const char* filename);
}