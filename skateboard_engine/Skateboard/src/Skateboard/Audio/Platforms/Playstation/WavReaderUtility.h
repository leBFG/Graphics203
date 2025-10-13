/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 7.00.00.38-00.00.03.1.1
* Copyright (C) 2018 Sony Interactive Entertainment Inc.
* 
*/

#ifndef SCE_AUDIO_OUT2_SAMPLE_UTILITY_H
#define SCE_AUDIO_OUT2_SAMPLE_UTILITY_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// 
//	File parse APIs
//

#define SCE_AUDIO_OUT2_WAVEFORM_BLOCK_REPEAT_INFINITE	(0xFFFFFFFFUL)	// Repeat infinity

#define SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_DATA		(-1)
#define SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_ADDRESS	(-2)
#define SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_FORMAT	(-3)
#define SCE_AUDIO_OUT2_ERROR_INVALID_OUT_ADDRESS		(-4)

#define SCE_AUDIO_OUT2_WAVEFORM_TYPE_NONE		0x00
#define SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_I8		0x01
#define SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_I16L	0x02
#define SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_I24L	0x03
#define SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_I32L	0x04
#define SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_F32L	0x05
#define SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_F64L	0x06

typedef struct SceAudioOut2WaveformFormat {
	uint32_t waveformType;			// Waveform type (SCE_AUDIO_OUT2_WAVEFORM_TYPE_...)
	uint32_t numChannels;			// Number of channels (1~8[ch])
	uint32_t sampleRate;			// Sample rate (1~48000[Hz])
} SceAudioOut2WaveformFormat;


#define SCE_AUDIO_OUT2_WAVEFORM_INFO_MAX_BLOCKS	4		// Maxinum of blocks

typedef struct SceAudioOut2WaveformInfo {
	SceAudioOut2WaveformFormat format;	// Format

	size_t waveformDataSize;

	uint32_t dataOffset;			// Data offset
	uint32_t dataSize;				// Data size [byte]

	uint32_t loopBeginPosition;		// Loop begin position
	uint32_t loopEndPosition;		// Loop end position
	uint32_t numSamples;			// Number of samples

									//									PCMUI8	PCMI16	PCMI24	PCMI32	PCMF32
	uint32_t audioUnitSize;			// Audio unit size [byte]			1xch	2xch	3xch	4xch	4xch	
} SceAudioOut2WaveformInfo;

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

int32_t sceAudioOut2ParseWaveformData(const void *data, size_t dataSize, SceAudioOut2WaveformInfo *outInfo);
int32_t sceAudioOut2ParseWaveformFile(const char *path, uint64_t offset, SceAudioOut2WaveformInfo *outInfo);
int32_t fileLoad(const char *path, void **outData, size_t *outSize);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	// SCE_AUDIO_OUT2_SAMPLE_UTILITY_H
