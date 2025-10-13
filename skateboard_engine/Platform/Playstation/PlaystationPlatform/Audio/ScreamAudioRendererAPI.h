#pragma once

#pragma comment(lib, "SceUserService_stub_weak")
#pragma comment(lib, "SceDbg_nosubmission_stub_weak")
#pragma comment(lib, "SceAudioOut2_stub_weak")
#pragma comment (lib, "libSceAjm_stub_weak.a")
#pragma comment (lib, "libSceSndstream.a")
#pragma comment (lib, "libSceScream.a")

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel.h>

#include <libsysmodule.h>
#include <scream/sce_scream.h>
#include <scream/sce_snd_data.h>
#include <scream/sce_sndstream.h>
#include <unordered_map>

#include "Skateboard/Audio/Api/AudioRendererAPI.h"
//size_t sceLibcHeapSize = 16 * 1024 * 1024; // 16 MB


//typedef void* (*SceScreamExternSndMemAlloc)(int32_t bytes, int32_t use);
//typedef void (*SceScreamExternSndMemFree)(void* mem);
namespace Skateboard {

	class ScreamAudioRendererAPI :public Skateboard::AudioRendererAPI {
	
	public:
		static void Init(); 

		virtual void Initialize() override;
		virtual void LoadBank(const char* path, const char* ID) override;
		//virtual void PlayAudio( uintptr_t tmpWavFileData, SceAudioOut2WaveformInfo tempinfo ) override;
		virtual uint64_t PlayAudio(const char* AudioName, SkbdSoundParams params) override;
		virtual uint64_t StopAudio(uint64_t) override;
		virtual uint64_t PauseAudio(uint64_t) override;
		virtual uint64_t ResumeAudio(uint64_t) override;
		virtual void EndScene() override;
		virtual void AddLocalPlayer(int32_t usrId) override;
		virtual void RemoveLocalPlayer(int32_t usrId) override;
		virtual void SetGroupVolume(int32_t group, float vol) override;
		virtual void SetGlobalVariable(const char* name, float value) override;
		virtual void SetGlobalRegister(int reg, int val) override;
	

	private:
		SceScreamSoundBank* screamBank;
		std::unordered_map<int32_t, uint8_t> playerMap;
	};

}