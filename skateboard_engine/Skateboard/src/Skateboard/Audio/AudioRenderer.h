#pragma once

#include "AudioFile.h"
namespace Skateboard
{
	typedef uint64_t SkbdSoundHandle;
	class AudioRendererAPI;

	class AudioRenderer
	{
	public:
		~AudioRenderer() {
			delete m_AudioRendererAPI;
			m_AudioRendererAPI = nullptr;
		};
		//Audio Renderer Functions
		static void InitPlatform();
		//static void Init(SceUserServiceUserId USERID, uint16_t PORT, uint32_t PORTDATAFORMAT);
		static void End();
		static void LoadAudio(char* path, char* ID);
		static void LoadBank(const char* path, const char* ID = "");
		//static void PlayAudio(uintptr_t tmpWavFileData, SceAudioOut2WaveformInfo tempinfo);
		static SkbdSoundHandle PlayAudio(const char* ID, SkbdSoundParams params = {0});
		static SkbdSoundHandle StopAudio(SkbdSoundHandle hnd);
		static SkbdSoundHandle PauseAudio(SkbdSoundHandle hnd);
		static SkbdSoundHandle ResumeAudio(SkbdSoundHandle hnd);

		//static void UpdateAudioRenderer();

		static void AddLocalPlayer(int32_t usrid);
		static void RemoveLocalPlayer(int32_t usrid);
		static void SetGroupVolume(float vol, int32_t group=0);
		static void SetGlobalVariable(const char* name, float value);
		static void SetGlobalRegister(int reg, int val) ; // int reg maps to the same value in Scream Audio->Tools->Register(Global) | ranges from 1 -> 64 | val ranges from -127 -> +127
		

	private:
		static AudioRendererAPI* m_AudioRendererAPI;
	};



}