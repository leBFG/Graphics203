#pragma once
#include "Skateboard/Audio/Api/AudioRendererAPI.h"

#pragma comment(lib, "SceUserService_stub_weak")
#pragma comment(lib, "SceDbg_nosubmission_stub_weak")
#pragma comment(lib, "SceAudioOut2_stub_weak")
#include <user_service.h>
#include <audio_out2.h>
#include <vector>
#include <libdbg.h>
#include <map>
#include <memory>
#include "PlaystationAudioInstance.h"
#define GRAIN 256


namespace Skateboard
{
	enum AudioStatus
	{
		AUDIO_STATUS_NONE = 0,
		AUDIO_STATUS_END = 1 << 0,
		AUDIO_STATUS_PROCESSING = 1 << 1,
		AUDIO_STATUS_CHANGE_DATA = 1 << 2,
		AUDIO_STATUS_PAUSE = 1 << 3,
		AUDIO_STATUS_LOOP = 1 << 4,
	};
	struct ContextStruct
	{
		SceAudioOut2ContextParam contextParam;
		SceAudioOut2ContextHandle contextHandle;
		SceAudioOut2PortParam portParam;
		SceAudioOut2PortHandle portHandle = SCE_AUDIO_OUT2_PORT_HANDLE_INVALID;
		SceAudioOut2UserHandle userHandle;
		SceKernelEventFlag m_EventFlag;
		void* contextMemory = NULL;
	};
	class PlaystationAudioRendererAPI : public Skateboard::AudioRendererAPI
	{
	public:

		//virtual void Init(SceUserServiceUserId USERID, uint16_t PORT, uint32_t PORTDATAFORMAT) override;

		PlaystationAudioRendererAPI();
		~PlaystationAudioRendererAPI();
		
		int InitContext(ContextStruct* context, SceUserServiceUserId USERID, uint16_t PORT, uint32_t PORTDATAFORMAT);

		//virtual void PlayAudio( uintptr_t tmpWavFileData, SceAudioOut2WaveformInfo tempinfo ) override;
		virtual void LoadAudio(char* path, char* ID) override;
		virtual uint64_t PlayAudio(const char* ID, int32_t userID) override;
		virtual uint64_t StopAudio(uint64_t, int32_t userID) override;

		virtual void Initialize() override;

		virtual void UpdateAudioRenderer() override;

		void EndScene() override;

		std::vector<PlaystationAudioInstance*> Audios;

		ContextStruct* GetContextFromOutputType(AudioFile* Audio);


	private:
		
		std::unique_ptr<ContextStruct> controllerContext;
		std::unique_ptr<ContextStruct> mainDeviceContext;

		SceUserServiceUserId userId;

		std::map<const char*, PlaystationAudioInstance*> AudioFiles;




	};

}