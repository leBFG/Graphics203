#pragma once

//#include <audio_out2.h>
//#include "Skateboard/Audio/Platforms/Playstation/WavReaderUtility.h"
#include "Skateboard/Audio/AudioFile.h"

namespace Skateboard
{

	class AudioRendererAPI
	{
	public:

		virtual ~AudioRendererAPI() {};

		//Windows Init pure function
		virtual void Initialize()= 0;

		//Playstation Init pure function
		//virtual void Init(SceUserServiceUserId USERID, uint16_t PORT, uint32_t PORTDATAFORMAT) = 0;

		virtual void EndScene() {};

		//Windows Play pure function
		virtual void LoadAudio(char* path, char* ID) {};
		virtual void LoadBank(const char* path, const char* ID) {};
		
		//Playstation Play Audio pure function
		virtual uint64_t PlayAudio(const char* ID, SkbdSoundParams params= { 0 }) = 0;

		virtual uint64_t StopAudio(uint64_t sndHandle) = 0;
		virtual uint64_t PauseAudio(uint64_t sndHandle) { return 0; };
		virtual uint64_t ResumeAudio(uint64_t sndHandle) { return 0; };

		virtual void UpdateAudioRenderer() {};

		virtual void AddLocalPlayer(int32_t Usrid) {};
		virtual void RemoveLocalPlayer(int32_t Usrid) {};
		virtual void SetGroupVolume(int32_t group, float vol) {};
		virtual void SetGlobalVariable(const char* name, float value) {};
		virtual void SetGlobalRegister(int reg, int val) {};

	protected:

	};


}