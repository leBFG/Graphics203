#include "Utils.h"
#include "vendor/model-loading/pack_file.h"

namespace Skateboard
{
	void MatrixMul(glm::float3 & inout_vec, const PackFile::Float4x4 & in_matrixCLM)
	{
		float x = inout_vec.x * in_matrixCLM.values[0] + inout_vec.y * in_matrixCLM.values[4] + inout_vec.z * in_matrixCLM.values[8] + 1.f * in_matrixCLM.values[12];
		float y = inout_vec.x * in_matrixCLM.values[1] + inout_vec.y * in_matrixCLM.values[5] + inout_vec.z * in_matrixCLM.values[9] + 1.f * in_matrixCLM.values[13];
		float z = inout_vec.x * in_matrixCLM.values[2] + inout_vec.y * in_matrixCLM.values[6] + inout_vec.z * in_matrixCLM.values[10] + 1.f * in_matrixCLM.values[14];
		// W will always be 1 so it is useless to calculate it and divide all components by w.
		//float w = inout_vec.x * in_matrixCLM.values[3] + inout_vec.y * in_matrixCLM.values[7] + inout_vec.z * in_matrixCLM.values[11] + 1.f * in_matrixCLM.values[15];
		inout_vec.x = x /*/ w*/;
		inout_vec.y = y /*/ w*/;
		inout_vec.z = z /*/ w*/;
	}

	void MatrixMul3x3(glm::float3 & inout_vec, const PackFile::Float4x4 & in_matrixCLM)
	{
		float x = inout_vec.x * in_matrixCLM.values[0] + inout_vec.y * in_matrixCLM.values[4] + inout_vec.z * in_matrixCLM.values[8];
		float y = inout_vec.x * in_matrixCLM.values[1] + inout_vec.y * in_matrixCLM.values[5] + inout_vec.z * in_matrixCLM.values[9];
		float z = inout_vec.x * in_matrixCLM.values[2] + inout_vec.y * in_matrixCLM.values[6] + inout_vec.z * in_matrixCLM.values[10];
		inout_vec.x = x;
		inout_vec.y = y;
		inout_vec.z = z;
	}

	void Normalize(glm::float3 & normal)
	{
		float length = normal.x * normal.x + normal.y * normal.y + normal.z * normal.z;
		if (length > 0.f)
			length = 1.f / sqrtf(length);
		normal.x *= length;
		normal.y *= length;
		normal.z *= length;
	}

	// https://github.com/microsoft/DirectXMath/blob/main/Inc/DirectXPackedVector.inl
	float halfToFloat(uint16_t float16_value)
	{
		auto Mantissa = static_cast<uint32_t>(float16_value & 0x03FF);

		uint32_t Exponent = (float16_value & 0x7C00);
		if (Exponent == 0x7C00) // INF/NAN
		{
			Exponent = 0x8f;
		}
		else if (Exponent != 0)  // The value is normalized
		{
			Exponent = static_cast<uint32_t>((static_cast<int>(float16_value) >> 10) & 0x1F);
		}
		else if (Mantissa != 0)     // The value is denormalized
		{
			// Normalize the value in the resulting float
			Exponent = 1;

			do
			{
				Exponent--;
				Mantissa <<= 1;
			} while ((Mantissa & 0x0400) == 0);

			Mantissa &= 0x03FF;
		}
		else                        // The value is zero
		{
			Exponent = static_cast<uint32_t>(-112);
		}

		uint32_t Result =
			((static_cast<uint32_t>(float16_value) & 0x8000) << 16) // Sign
			| ((Exponent + 112) << 23)                      // Exponent
			| (Mantissa << 13);                             // Mantissa

		return reinterpret_cast<float*>(&Result)[0];
	}

	float snorm16ToFloat(short int v)
	{
		// -32768 & -32767 both map to -1 according to D3D10 rules.
		return std::max(v / 32767.0f, -1.0f);
	}


}