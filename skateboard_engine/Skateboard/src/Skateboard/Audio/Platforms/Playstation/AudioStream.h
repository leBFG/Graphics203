#pragma once

#pragma comment(lib, "SceUserService_stub_weak")
#pragma comment(lib, "SceDbg_nosubmission_stub_weak")
#pragma comment(lib, "SceAudioOut2_stub_weak")

#include <user_service.h>
#include <audio_out2.h>
#include <vector>
#include <libdbg.h>

#include "WavReaderUtility.h"
#include "Skateboard/Audio/AudioFile.h"

#define GRAIN 256

enum AudioStatus
{
    AUDIO_STATUS_NONE                 =0   ,
    AUDIO_STATUS_END                  =1<<0,
    AUDIO_STATUS_PROCESSING           =1<<1,
    AUDIO_STATUS_CHANGE_DATA          =1<<2,
    AUDIO_STATUS_PAUSE                =1<<3,
    AUDIO_STATUS_LOOP                 =1<<4,
};

class AudioStream: public Skateboard::AudioFile
{
protected:
    size_t memorySize;
    void* contextMemory = NULL;

    uint64_t m_StateFlag;


public:
    SceKernelEventFlag m_EventFlag;
    void* waveformData = NULL;
    SceAudioOut2WaveformInfo waveformInfo;

    virtual int ReadAudioFile(char* path) override;
};