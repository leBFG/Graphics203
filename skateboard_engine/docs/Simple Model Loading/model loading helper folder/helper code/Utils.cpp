#include "Utils.h"
#include "libScePackParser/pack_file.h"
namespace Utils
{
	void MatrixMul(float3 & inout_vec, const PackFile::Float4x4 & in_matrixCLM)
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

	void MatrixMul3x3(float3 & inout_vec, const PackFile::Float4x4 & in_matrixCLM)
	{
		float x = inout_vec.x * in_matrixCLM.values[0] + inout_vec.y * in_matrixCLM.values[4] + inout_vec.z * in_matrixCLM.values[8];
		float y = inout_vec.x * in_matrixCLM.values[1] + inout_vec.y * in_matrixCLM.values[5] + inout_vec.z * in_matrixCLM.values[9];
		float z = inout_vec.x * in_matrixCLM.values[2] + inout_vec.y * in_matrixCLM.values[6] + inout_vec.z * in_matrixCLM.values[10];
		inout_vec.x = x;
		inout_vec.y = y;
		inout_vec.z = z;
	}

	void Normalize(float3 & normal)
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

	uint8_t* allocDmem(sce::Agc::SizeAlign sizeAlign)
	{
		if (!sizeAlign.m_size)
		{
			return nullptr;
		}

		static uint32_t allocCount = 0;
		off_t offsetOut;

		uint32_t alignment = (sizeAlign.m_align + 0xffffu) & ~0xffffu;
		uint64_t size = (sizeAlign.m_size + 0xffffu) & ~0xffffu;

		int32_t ret = sceKernelAllocateMainDirectMemory(size, alignment, SCE_KERNEL_MTYPE_C, &offsetOut);
		if (ret) {
			printf("sceKernelAllocateMainDirectMemory error:0x%x size:0x%zx\n", ret, size);
			return nullptr;
		}

		void* ptr = NULL;
		char namedStr[32];
		snprintf_s(namedStr, sizeof(namedStr), "agc_binder %d_%zuKB", allocCount++, size >> 10);
		ret = sceKernelMapNamedDirectMemory(&ptr, size, SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_CPU_RW, 0, offsetOut, alignment, namedStr);
		SCE_AGC_ASSERT_MSG(ret == SCE_OK, "Unable to map memory");
		return (uint8_t*)ptr;
	}

	// This is a helper function to load a texture from a Gnf file. It is not important to understand how this works.
	// (From the samples)
	SceError loadTexture(sce::Agc::Core::Texture* tex, const char* filename)
	{
		FILE *fp = fopen(filename, "rb");
		SCE_AGC_ASSERT_MSG_RETURN(fp, SCE_AGC_ERROR_FAILURE, "Could not open file %s", filename);
		fseek(fp, 0, SEEK_END);
		long sz = ftell(fp);
		if (sz <= 0)
		{
			SCE_AGC_ASSERT_MSG("Could not get the size of the file %s", filename);
			fclose(fp);
			return SCE_AGC_ERROR_FAILURE;
		}
		fseek(fp, 0, SEEK_SET);
		void *gnfData = allocDmem({ (uint64_t)sz, 64 * 1024 });
		printf("%s(%s): p=%p, sz=%lu\n", __func__, filename, gnfData, sz);
		size_t bytesRead = fread(gnfData, 1, sz, fp);
		if (bytesRead != sz)
		{
			SCE_AGC_ASSERT_MSG("Could not read the entire contents of the file %s", filename);
			fclose(fp);
			return SCE_AGC_ERROR_FAILURE;
		}
		fclose(fp);

		// The packfile exporter uses Gnf Version 5!
		SceError err = sce::Agc::Core::translate(tex, (sce::Gnf::GnfFileV5*)gnfData);
		if (err == SCE_OK)
		{
			sce::Agc::Core::registerResource(tex, "%s", filename);
		}
		return err;
	}
}