/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 7.00.00.38-00.00.03.1.1
* Copyright(C) 2018 Sony Interactive Entertainment Inc.
* 
*/

#include "WavReaderUtility.h"
#ifdef SKTBD_PLATFORM_PLAYSTATION
#include <audio_out2.h>
#include <kernel.h>
#include <sceerror.h>
#endif // SKTBD_PLATFORM_PLAYSTATION

#include <math.h>

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif 

///////////////////////////////////////////////////////////////////////////////
//	Utility system
///////////////////////////////////////////////////////////////////////////////

//
//	Memory APIs
//

void memoryCopy(void* dst, const void* src, size_t size)
{
	memcpy_s(dst, size, src, size);
}

// Clear

void memoryClear(void *ptr, size_t size)
{
	memset(ptr, 0, size);
}

// Compare

int32_t memoryCompare(const void *ptr0, const void *ptr1, size_t size)
{
	return memcmp(ptr0, ptr1, size);
}

//
//	Data reader
//

// Reader definition

typedef enum SceAudioOut2DataReaderTypes {
	SCE_AUDIO_OUT2_DATA_READER_TYPE_FILE,
	SCE_AUDIO_OUT2_DATA_READER_TYPE_MEMORY,
	SCE_AUDIO_OUT2_DATA_READER_TYPE_COUNT,
} SceAudioOut2DataReaderTypes;

typedef struct SceAudioOut2DataReader {
	uint32_t type;					// Type (SceAudioOut2DataReaderTypes)
	union {
		struct {
			int32_t context;		// Context
			uint64_t offset;				// Offset
		} file;
		struct {
			const uint8_t *data;			// Pointer to data
			size_t dataSize;				// Data size
		} memory;
	};
} SceAudioOut2DataReader;

// Read

static int32_t sceAudioOut2DataReaderRead(
	SceAudioOut2DataReader *reader,
	uint32_t offset,
	void *buffer,
	size_t readSize,
	void **outPointer)
{
	int32_t result;

	switch (reader->type) {
	case SCE_AUDIO_OUT2_DATA_READER_TYPE_FILE:
		result = sceKernelPread(
			reader->file.context,
			buffer,
			readSize,
			reader->file.offset + offset);
		if (result < SCE_OK) {
			return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_DATA;
		}
		if (outPointer) *outPointer = buffer;
		break;
	case SCE_AUDIO_OUT2_DATA_READER_TYPE_MEMORY:
		if (reader->memory.dataSize < offset + readSize) {
			return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_DATA;
		}
		if (reader->memory.data == NULL) {
			return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_ADDRESS;
		}

		if (outPointer) *outPointer = (void*)&reader->memory.data[offset];
		break;
	default:
		return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_DATA;
	}

	return SCE_OK;
}

//
//	Wave file
//

// RIFF header

#define SCE_AUDIO_OUT2_WAVE_4CC(a,b,c,d)	((((uint8_t)(a))<<0)|(((uint8_t)(b))<<8)|(((uint8_t)(c))<<16)|(((uint8_t)(d))<<24))

#define SCE_AUDIO_OUT2_WAVE_RIFF		SCE_AUDIO_OUT2_WAVE_4CC('R','I','F','F')
#define SCE_AUDIO_OUT2_WAVE_WAVE		SCE_AUDIO_OUT2_WAVE_4CC('W','A','V','E')

typedef struct SceAudioOut2WaveRiffHeader {
	uint32_t RIFF;					// 'RIFF'
	uint32_t size;					// File size
	uint32_t WAVE;					// 'WAVE'
} SceAudioOut2WaveRiffHeader;

// RIFF chunk

#define SCE_AUDIO_OUT2_WAVE_CHUNK_data	SCE_AUDIO_OUT2_WAVE_4CC('d','a','t','a')
#define SCE_AUDIO_OUT2_WAVE_CHUNK_fmt	SCE_AUDIO_OUT2_WAVE_4CC('f','m','t',' ')
#define SCE_AUDIO_OUT2_WAVE_CHUNK_fact	SCE_AUDIO_OUT2_WAVE_4CC('f','a','c','t')

typedef struct SceAudioOut2WaveRiffChunk {
	uint32_t id;					// ID (4CC)
	uint32_t size;					// size
} SceAudioOut2WaveRiffChunk;

// Format information

typedef enum SceAudioOut2WaveFormatTags {
	SCE_AUDIO_OUT2_WAVE_FORMAT_TAG_PCM		= 0x0001,
	SCE_AUDIO_OUT2_WAVE_FORMAT_TAG_IEEE_FLOAT	= 0x0003,
	SCE_AUDIO_OUT2_WAVE_FORMAT_TAG_EXTENSIBLE	= 0xFFFE
} SceAudioOut2WaveFormatTags;

typedef struct SceAudioOut2WaveFormat {
	uint16_t wFormatTag;			// Format tag (SceAudioOut2WaveFormatTags)
	uint16_t nChannels;				// Number of channels
	uint32_t nSamplePerSec;			// Sample per second
	uint32_t nAvgBytePerSec;		// Average size per second
	uint16_t nBlockAlign;			// Block align
	uint16_t wBitPerSample;			// Bits per sample
} SceAudioOut2WaveFormat;

typedef struct SceAudioOut2WaveFormatEx {
	SceAudioOut2WaveFormat format;		// Format
	uint16_t cbSize;				// Extensible data size
	uint16_t wSamplesPerBlock;		// Number of audio block samples
	uint32_t dwChannelMask;			// Speaker channel map
	uint8_t SubFormat[16];			// Codec ID (47E142D2-36BA-4d8d-88FC-61654F8C836C)
} SceAudioOut2WaveFormatEx;

// Fact information

typedef struct SceAudioOut2WaveFact {
	uint32_t dwSampleLength;		// Total samples
} SceAudioOut2WaveFact;

// SubFormat GUID

static uint8_t s_sceAudioOut2GuidPcm[]    ={0x01, 0x00, 0x00, 0x00,   0x00, 0x00,   0x10, 0x00,   0x80, 0x00,   0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};

//
//	Parse waveform data/file
//

// Get chunk

static int32_t sceAudioOut2WaveParseChunk(
	SceAudioOut2DataReader *reader,
	uint32_t chunkId,
	void *buffer, size_t bufferSize,
	uint32_t *outDataOffset,
	uint32_t *outDataSize,
	void **outPointer)
{
	SceAudioOut2WaveRiffHeader *header;
	SceAudioOut2WaveRiffHeader headerBuffer;
	uint32_t offset;
	int32_t result;
	const uint32_t chunkAlignSize = 4;

	offset = 0;

	// Riff header

	result = sceAudioOut2DataReaderRead(
		reader,
		offset,
		&headerBuffer, sizeof(headerBuffer),
		(void**)&header);
	if (result < SCE_OK)
		return result;
	

	offset += sizeof(*header);

	// Find in chunks

	while (offset < header->size - sizeof(uint32_t)) {
		SceAudioOut2WaveRiffChunk *chunk;
		SceAudioOut2WaveRiffChunk chunkBuffer;

		result = sceAudioOut2DataReaderRead(
			reader,
			offset,
			&chunkBuffer, sizeof(chunkBuffer),
			(void**)&chunk);
		if (result < SCE_OK)
			return result;


		offset += sizeof(*chunk);

		if (chunk->id == chunkId) {
			result = sceAudioOut2DataReaderRead(
				reader,
				offset,
				buffer, min(chunk->size, (uint32_t)bufferSize),
				outPointer);
			if (result < SCE_OK)
				return result;


			if (outDataOffset) { *outDataOffset = offset; }
			if (outDataSize) { *outDataSize = chunk->size; }
			return SCE_OK;
		}

		offset += chunk->size;
		if (chunk->size & (chunkAlignSize - 1))
			offset += (chunkAlignSize - (chunk->size & (chunkAlignSize - 1)));

		if (chunk->size == 0) break;
	}

	return SCE_AUDIO_OUT2_ERROR_FATAL;	// Not found
}

// Parse waveform core

static int32_t sceAudioOut2ParseWaveform(SceAudioOut2DataReader *reader, SceAudioOut2WaveformInfo *outInfo)
{
	char *fcc;
	char fccBuffer[4];
	uint32_t waveformType, sampleRate, numChannels, numSamples, dataOffset, dataSize, blockSize;
	int32_t result;

	// Error Check

	if (outInfo == NULL)	return SCE_AUDIO_OUT2_ERROR_INVALID_OUT_ADDRESS;

	waveformType      = SCE_AUDIO_OUT2_WAVEFORM_TYPE_NONE;
	numChannels       = 0;
	sampleRate        = 0;
	numSamples        = 0;
	dataOffset        = 0;
	dataSize          = 0;
	blockSize         = 0;

	// Check 1st 4byte

	result = sceAudioOut2DataReaderRead(
		reader,
		0,
		fccBuffer, sizeof(fccBuffer),
		(void**)&fcc);
	if (result < SCE_OK)
		return result;

	if (memoryCompare(fcc, "RIFF", 4) == 0) {
		//
		//	PCM
		//

		const SceAudioOut2WaveFormat *format;
		const SceAudioOut2WaveFact *fact;
		char formatData[sizeof(SceAudioOut2WaveformFormat)];
		SceAudioOut2WaveFact factData;
		uint32_t formatDataSize;

		// 'fmt'
		//	- Waveform type
		//	- Number of channels

		result = sceAudioOut2WaveParseChunk(
			reader,
			SCE_AUDIO_OUT2_WAVE_CHUNK_fmt,
			&formatData, sizeof(formatData),
			NULL,
			&formatDataSize,
			(void**)&format);
		if (result < SCE_OK)
			return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_DATA;

		numChannels = format->nChannels;
		sampleRate  = format->nSamplePerSec;

		switch (format->wFormatTag) {
		case SCE_AUDIO_OUT2_WAVE_FORMAT_TAG_PCM:
		pcm:
			switch (format->wBitPerSample) {
			case 8:
				waveformType = SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_I8;
				blockSize    = 1 * numChannels;
				break;
			case 16:
				waveformType = SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_I16L;
				blockSize    = 2 * numChannels;
				break;
			case 24:
				waveformType = SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_I24L;
				blockSize    = 3 * numChannels;
				break;
			case 32:
				waveformType = SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_I32L;
				blockSize    = 4 * numChannels;
				break;
			default:
				return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_FORMAT;
			}
			break;

		case SCE_AUDIO_OUT2_WAVE_FORMAT_TAG_IEEE_FLOAT:
			switch (format->wBitPerSample) {
			case 32:
				waveformType = SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_F32L;
				blockSize    = 4 * numChannels;
				break;
			case 64:
				waveformType = SCE_AUDIO_OUT2_WAVEFORM_TYPE_PCM_F64L;
				blockSize    = 8 * numChannels;
				break;
			default:
				return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_FORMAT;
			}
			break;
		case SCE_AUDIO_OUT2_WAVE_FORMAT_TAG_EXTENSIBLE: {
				const SceAudioOut2WaveFormatEx *formatEx = (SceAudioOut2WaveFormatEx*)format;

				if (formatDataSize < sizeof(*formatEx)) return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_FORMAT;

				if (memoryCompare(formatEx->SubFormat, s_sceAudioOut2GuidPcm, sizeof(formatEx->SubFormat)) == 0) {
					goto pcm;
				}
			}
			break;

		default:
			return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_FORMAT;
		}

		// 'fact'
		//	- Number of samples

		if (SCE_AUDIO_OUT2_SUCCEEDED(sceAudioOut2WaveParseChunk(
			reader,
			SCE_AUDIO_OUT2_WAVE_CHUNK_fact,
			&factData, sizeof(factData),
			NULL,
			NULL,
			(void**)&fact))) {
			numSamples = fact->dwSampleLength;
		} 

		// 'data'
		//	- Data offset
		//	- Data size
		//	- Number of samples (if there isn't fact chunk)
		result = sceAudioOut2WaveParseChunk(
			reader,
			SCE_AUDIO_OUT2_WAVE_CHUNK_data,
			NULL, 0,
			&dataOffset,
			&dataSize,
			NULL);
		if (result < SCE_OK)
			return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_DATA;

	} else {
		//
		//	Unknown file format
		//

		return SCE_AUDIO_OUT2_ERROR_INVALID_WAVEFORM_DATA;
	}

	// Output information

	if (outInfo) {

		outInfo->format.waveformType    = waveformType;
		outInfo->format.numChannels     = numChannels;
		outInfo->format.sampleRate      = sampleRate;

		outInfo->dataOffset             = dataOffset;
		outInfo->dataSize               = dataSize;
		outInfo->numSamples             = numSamples;

		outInfo->audioUnitSize          = blockSize;
	}

	return SCE_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	External APIs
///////////////////////////////////////////////////////////////////////////////

// Parse waveform on memory

int32_t sceAudioOut2ParseWaveformData(const void *data, size_t dataSize, SceAudioOut2WaveformInfo *outInfo)
{
	SceAudioOut2DataReader reader;

	if (outInfo) { memoryClear(outInfo, sizeof(SceAudioOut2WaveformInfo)); }

	reader.type = SCE_AUDIO_OUT2_DATA_READER_TYPE_MEMORY;
	reader.memory.data = (uint8_t*)data;
	reader.memory.dataSize = dataSize;

	return sceAudioOut2ParseWaveform(&reader, outInfo);
}

// Parse waveform on file

int32_t sceAudioOut2ParseWaveformFile(const char *path, uint64_t offset, SceAudioOut2WaveformInfo *outInfo)
{
	SceAudioOut2DataReader reader;
	int32_t result;

	if (outInfo) { memoryClear(outInfo, sizeof(SceAudioOut2WaveformInfo)); }

	reader.type = SCE_AUDIO_OUT2_DATA_READER_TYPE_FILE;
	reader.file.offset = offset;

	result = sceKernelOpen(path, SCE_KERNEL_O_RDONLY, 0);
	if (result < SCE_OK)
		return result;

	reader.file.context = result;

	result = sceAudioOut2ParseWaveform(&reader, outInfo);

	sceKernelClose(reader.file.context);

	return result;
}

// Load a file as binary data

int32_t fileLoad(const char *path, void **outData, size_t *outSize)
{
    void *data;
    int32_t file, result;
    size_t size;
    SceKernelStat stat;

    data = NULL;
    file = -1;
    if (outData) *outData = NULL;
    if (outSize) *outSize = 0;

    file = sceKernelOpen(path, SCE_KERNEL_O_RDONLY, 0);
    if (file < 0)
        return -1;

    result = sceKernelFstat(file, &stat);
    if (result < SCE_OK) {
        sceKernelClose(file);
        return -1;
    }

    size = stat.st_size;

    data = malloc(size);
    if (data == NULL) {
        sceKernelClose(file);
        return -1;
    }

    result = sceKernelRead(file, data, size);
    if (result < 0) {
        sceKernelClose(file);
        free(data);
        return -1;
    }

    sceKernelClose(file);

    if (outData) *outData = data;
    if (outSize) *outSize = size;

    return 0;
}