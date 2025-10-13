#include "PlaystationAudioInstance.h"
namespace Skateboard
{
	PlaystationAudioInstance::PlaystationAudioInstance()
	{
	}

	PlaystationAudioInstance::~PlaystationAudioInstance()
	{
	}

	int PlaystationAudioInstance::ReadAudioFile(char* path)
	{
		return AudioStream::ReadAudioFile(path);
	}
}


