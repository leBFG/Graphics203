#include "sktbdpch.h"
#include "AudioFile.h"
#include "AudioRenderer.h"

#ifdef SKTBD_PLATFORM_PLAYSTATION
#include "Platforms/Playstation/PlaystationAudioInstance.h"
#endif // DEBUG

namespace Skateboard
{

	AudioFile* AudioFile::Create()
	{
#ifdef SKTBD_PLATFORM_PLAYSTATION
			return new PlaystationAudioInstance;
#endif
		    
			return nullptr;
		
		
	}

	int AudioFile::ReadAudioFile(char* path)
	{
		return 0;
	}

}




