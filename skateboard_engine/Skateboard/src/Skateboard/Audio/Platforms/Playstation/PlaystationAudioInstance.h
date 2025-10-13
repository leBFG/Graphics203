#include "Skateboard/Audio/Api/AudioRendererAPI.h"

#include "AudioStream.h"
namespace Skateboard
{
	class PlaystationAudioInstance : public AudioStream
	{
	public:
		PlaystationAudioInstance();
		~PlaystationAudioInstance();

		int ReadAudioFile(char* path) override;
	};
}