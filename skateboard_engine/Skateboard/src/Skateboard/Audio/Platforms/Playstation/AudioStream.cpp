#include "AudioStream.h"

int AudioStream::ReadAudioFile(char* path)
{
   
    //make sure if the project is using the file path it is set to correct mirrored directory in the directory is replaced with “/app0/…”
    int result;
    result= fileLoad("/app0/assets/audio/Radio(32bit).wav", &waveformData, &waveformInfo.waveformDataSize);

    result = sceAudioOut2ParseWaveformData(waveformData, waveformInfo.waveformDataSize, &waveformInfo);

    if (result < 0)
    {
        printf("oh no");
    }
    return result;
}