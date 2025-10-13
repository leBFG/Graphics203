#pragma once
//#include "Skateboard/Audio/Platforms/Playstation/WavReaderUtility.h"
#include "Skateboard/Mathematics.h"
namespace Skateboard
{

	struct SkbdSoundParams {
		int32_t userId;
	};

	enum class AudioOutputType
	{
		Out_WindowsMain = 0,
		Out_PSMain = 1,
		Out_PSController = 2,
		Out_None = 4,
	};

	class AudioFile
	{
	public:
		static AudioFile* Create();

		//virtual void Init() = 0;
		virtual int ReadAudioFile(char* path);

		//TEMPORARILY PUBLIC
		bool AudioPlay;
		bool AudioLoop;
		bool AudioGain;
		size_t OutputPosition = 0;
		AudioOutputType OutputType = AudioOutputType::Out_None;
	private:
		//add volume parameters
	};
}