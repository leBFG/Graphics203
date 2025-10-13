#include "PlaystationAudioRendererAPI.h"
#include "pad.h"

namespace Skateboard
{
    PlaystationAudioRendererAPI::PlaystationAudioRendererAPI()
    {
        //Initialise AudioOut2 library
        sceAudioOut2Initialize();

        //user id for pad, this code will need to be moved.
        sceUserServiceGetInitialUser(&userId);

        //Create two contexts, one for controller one for main device
        controllerContext = std::make_unique<ContextStruct>();
        mainDeviceContext = std::make_unique<ContextStruct>();
    }
    PlaystationAudioRendererAPI::~PlaystationAudioRendererAPI()
    {
    }
    int PlaystationAudioRendererAPI::InitContext(ContextStruct* context, SceUserServiceUserId USERID, uint16_t PORT, uint32_t PORTDATAFORMAT)
    {
        int32_t result;

        size_t memorySize;

        // create context
        sceAudioOut2ContextResetParam(&context->contextParam);
        context->contextParam.maxObjectPorts = 0;
        context->contextParam.guaranteeObjectPorts = 0;
        context->contextParam.maxPorts = 2;
        context->contextParam.numGrains = GRAIN;
        context->contextParam.queueDepth = 64;
        context->contextParam.flags = SCE_AUDIO_OUT2_CONTEXT_PARAM_FLAG_MAIN;

        result = sceAudioOut2ContextQueryMemory(&context->contextParam, &memorySize);
        if (result < SCE_OK)
            goto exit;

        context->contextMemory = malloc(memorySize);
        if (context->contextMemory == NULL)
            goto exit;

        // create audio out 2 context
        result = sceAudioOut2ContextCreate(&context->contextParam, context->contextMemory, memorySize, &context->contextHandle);
        if (result < SCE_OK)
            goto exit;

        // create audio out2 user
        result = sceAudioOut2UserCreate(USERID, &context->userHandle);
        if (result < SCE_OK)
            goto exit;

        // create port
         context->portParam.dataFormat = PORTDATAFORMAT;
         context->portParam.portType = PORT;
         context->portParam.samplingFreq = 48000;
         context->portParam.userHandle = context->userHandle;
         context->portParam.flags = 0;
         context->portParam.pad = 0;
        //memset(context->portParam.reserved, 0, sizeof(context->portParam.reserved));
       
        result = sceAudioOut2PortCreate(context->contextHandle, &context->portParam, &context->portHandle);
        if (result < SCE_OK)
            goto exit;

        result = sceKernelCreateEventFlag(&context->m_EventFlag, "EvFlg", SCE_KERNEL_EVF_ATTR_SINGLE, 0x0, NULL);
        if (result < SCE_OK)
            goto exit;

        return result;

    exit:
        if ( context->portHandle != SCE_AUDIO_OUT2_PORT_HANDLE_INVALID)	sceAudioOut2PortDestroy(context->portHandle);
        if ( context->userHandle != SCE_AUDIO_OUT2_USER_HANDLE_INVALID)	sceAudioOut2UserDestroy(context->userHandle);
        if ( context->contextHandle != SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID)	sceAudioOut2ContextDestroy(context->contextHandle);
        if ( context->contextMemory != NULL)	free(context->contextMemory);

        return result;

    }

    void PlaystationAudioRendererAPI::LoadAudio(char* path, char* ID)
    {
        //Load audio into the map
        PlaystationAudioInstance* AudioInstance = new PlaystationAudioInstance();
        //Read Audio File giving path.
        AudioInstance->ReadAudioFile(path);
        //Create map pair using ID as key and audio file as value.
        AudioFiles.insert(std::pair<char*, PlaystationAudioInstance*>(ID, AudioInstance));
        
    }

    void PlaystationAudioRendererAPI::PlayAudio(const char* ID, int32_t userID)
    {
        //Find audio file in the map using ID
        auto it = AudioFiles.find(ID);
        //If ID is found in the map
        if (it != AudioFiles.end())
        {
            //Change boolean to play audio.
            //NOTE: CURRENTLY TWO DIFFERENT AUDIOS NEED TO BE LOADED IN ORDER TO PLAY THE AUDIO IN TWO DIFFERENT CONTEXTS.
            //i.e.: if an audio needs to be played on controller and main context, two different audios need to be created, need to fix this.
            PlaystationAudioInstance* AudioInstance = it->second;
            AudioInstance->AudioPlay = true;
            //AudioInstance->OutputType = OutputType;
            AudioInstance->AudioLoop = false;
        }
        else
        {
            printf("File with given ID not present in audio files\n");
        }
    }

    void PlaystationAudioRendererAPI::StopAudio(uint64_t hnd, int32_t userID)
    {
        const char* ID = "asd";
        //Find audio file in the map using ID
        auto it = AudioFiles.find(ID);
        //If ID is found in the map
        if (it != AudioFiles.end())
        {
            //Change boolean to play audio.
            //NOTE: CURRENTLY TWO DIFFERENT AUDIOS NEED TO BE LOADED IN ORDER TO PLAY THE AUDIO IN TWO DIFFERENT CONTEXTS.
            //i.e.: if an audio needs to be played on controller and main context, two different audios need to be created, need to fix this.
            PlaystationAudioInstance* AudioInstance = it->second;
            AudioInstance->AudioPlay = false;
            AudioInstance->OutputType = AudioOutputType::Out_None;
        }
        else
        {
            printf("File with given ID not present in audio files\n");
        }
    }

    void PlaystationAudioRendererAPI::Initialize()
    {
        ScePadControllerInformation PadInfo;
        int32_t handle;

        switch (/*outputType*/ AudioOutputType::Out_PSMain)
        {
            //Add more cases if more contexts are needed
        case AudioOutputType::Out_PSController:

            //scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
            //scePadGetControllerInformation(handle, &PadInfo);

            InitContext(controllerContext.get(), userId, SCE_AUDIO_OUT2_PORT_TYPE_PADSPK, SCE_AUDIO_OUT2_DATA_FORMAT_FLOAT_MONO);
            break;
        case AudioOutputType::Out_PSMain:
            //scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
            //scePadGetControllerInformation(handle, &PadInfo);

            InitContext(mainDeviceContext.get(), SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_AUDIO_OUT2_PORT_TYPE_MAIN, SCE_AUDIO_OUT2_DATA_FORMAT_FLOAT_MONO);
            break;
        case AudioOutputType::Out_None:
            break;
        default:
            break;

        }
    }

    void PlaystationAudioRendererAPI::UpdateAudioRenderer()
    {
        // Check if a queue is available.
        int result = 0;
        uint32_t queueAvailable;
        sceAudioOut2ContextGetQueueLevel(controllerContext->contextHandle, NULL, &queueAvailable);
        
        for (auto it = AudioFiles.begin(); it != AudioFiles.end(); ++it) 
        {
            PlaystationAudioInstance* CurrentAudio = it->second;
            ContextStruct* tmpContext = GetContextFromOutputType(CurrentAudio);

            if (CurrentAudio->AudioPlay)
            {
                // fill the queue up
                for (int i = 0; i < queueAvailable; i++)
                {
                    //loop the position if looping else stop playback
                    if (CurrentAudio->AudioLoop && (CurrentAudio->OutputPosition >= CurrentAudio->waveformInfo.dataSize))
                    {
                        CurrentAudio->OutputPosition = 0;
                    }
                    else if (CurrentAudio->OutputPosition > CurrentAudio->waveformInfo.dataSize)
                    {
                        CurrentAudio->AudioPlay = false;
                        break;
                    }

                    //Audio data is pushed to the rendering device via attributes.Channel audio supports fewer attributes than object based audio. (may need further digging) for channel audio we can specify the volumeand offset as well as spread
                    SceAudioOut2Pcm pcm;
                    SceAudioOut2Attribute aAttribute[2];
                    uint32_t numAttributes = 0;

                    //Set gain
                    aAttribute[numAttributes].attributeId = SCE_AUDIO_OUT2_PORT_ATTRIBUTE_ID_GAIN;
                    aAttribute[numAttributes].value = &CurrentAudio->AudioGain;
                    aAttribute[numAttributes].valueSize = sizeof(float) * CurrentAudio->waveformInfo.format.numChannels;
                    numAttributes++;

                    // Set pcm data ie the offset into the audio data
                    pcm.data = (void*)((uintptr_t)CurrentAudio->waveformData + CurrentAudio->waveformInfo.dataOffset + CurrentAudio->OutputPosition);
                    aAttribute[numAttributes].attributeId = SCE_AUDIO_OUT2_PORT_ATTRIBUTE_ID_PCM;
                    aAttribute[numAttributes].value = &pcm;
                    aAttribute[numAttributes].valueSize = sizeof(pcm);
                    numAttributes++;

                    //update the position //audiounit size in bytes (sizeof(SamplePrecision(16 or 32 bits)) x Number of Channels) I16 MONO will be 2 bytes I16 STEREO – 4 bytes
                    CurrentAudio->OutputPosition += CurrentAudio->waveformInfo.audioUnitSize * GRAIN;

                    result = sceAudioOut2PortSetAttributes(tmpContext->portHandle, aAttribute, numAttributes);
                    SCE_DBG_ASSERT(result == SCE_OK);

                    // Advances the context queue pointer to allow more data to be buffered
                    result = sceAudioOut2ContextAdvance(tmpContext->contextHandle);
                    SCE_DBG_ASSERT(result == SCE_OK);

                    // Pushes all audio and attributes to the rendering device.
                    result = sceAudioOut2ContextPush(tmpContext->contextHandle, SCE_AUDIO_OUT2_BLOCKING_ASYNC);
                    SCE_DBG_ASSERT(result == SCE_OK);

                }
            }
        }
     
    }
    void PlaystationAudioRendererAPI::EndScene()
    {
    }

    ContextStruct* PlaystationAudioRendererAPI::GetContextFromOutputType(AudioFile* Audio)
    {
        ContextStruct* tmpContext;
        switch (Audio->OutputType)
        {
        case AudioOutputType::Out_PSController:
            tmpContext = controllerContext.get();
               break;
        case AudioOutputType::Out_PSMain:
            tmpContext = mainDeviceContext.get();
            break;
        case AudioOutputType::Out_None:
            break;
        default:
            break;
        }

        return tmpContext;

    }

}

//if (Audios.size() != 0)
     //{
     //    for (int a = 0; a < Audios.size(); a++)
     //    {
     //        if (CurrentAudio->AudioPlay)
     //        {
     //            ContextStruct* tmpContext = GetContextFromOutputType(CurrentAudio);
     //            // fill the queueue up
     //            for (int i = 0; i < queueAvailable; i++)
     //            {
     //                //loop the position if looping else stop playback
     //                if (CurrentAudio->AudioLoop && CurrentAudio->OutputPosition >= CurrentAudio->waveformInfo.dataSize)
     //                {
     //                    CurrentAudio->OutputPosition = 0;
     //                }
     //                else if (CurrentAudio->OutputPosition > CurrentAudio->waveformInfo.dataSize)
     //                {
     //                    CurrentAudio->AudioPlay = false;
     //                    break;
     //                }
     //
     //                //Audio data is pushed to the rendering device via attributes.Channel audio supports fewer attributes than object based audio. (may need further digging) for channel audio we can specify the volumeand offset as well as spread
     //                SceAudioOut2Pcm pcm;
     //                SceAudioOut2Attribute aAttribute[2];
     //                uint32_t numAttributes = 0;
     //
     //                //Set gain
     //                aAttribute[numAttributes].attributeId = SCE_AUDIO_OUT2_PORT_ATTRIBUTE_ID_GAIN;
     //                aAttribute[numAttributes].value = &CurrentAudio->AudioGain;
     //                aAttribute[numAttributes].valueSize = sizeof(float) * CurrentAudio->waveformInfo.format.numChannels;
     //                numAttributes++;
     //
     //                // Set pcm data ie the offset into the audio data
     //                pcm.data = (void*)((uintptr_t)CurrentAudio->waveformData + CurrentAudio->waveformInfo.dataOffset + CurrentAudio->OutputPosition);
     //                aAttribute[numAttributes].attributeId = SCE_AUDIO_OUT2_PORT_ATTRIBUTE_ID_PCM;
     //                aAttribute[numAttributes].value = &pcm;
     //                aAttribute[numAttributes].valueSize = sizeof(pcm);
     //                numAttributes++;
     //
     //                //update the position //audiounit size in bytes (sizeof(SamplePrecision(16 or 32 bits)) x Number of Channels) I16 MONO will be 2 bytes I16 STEREO – 4 bytes
     //                CurrentAudio->OutputPosition += CurrentAudio->waveformInfo.audioUnitSize * GRAIN;
     //
     //                result = sceAudioOut2PortSetAttributes(tmpContext->portHandle, aAttribute, numAttributes);
     //                SCE_DBG_ASSERT(result == SCE_OK);
     //
     //                // Advances the context queue pointer to allow more data to be buffered
     //                result = sceAudioOut2ContextAdvance(tmpContext->contextHandle);
     //                SCE_DBG_ASSERT(result == SCE_OK);
     //
     //                // Pushes all audio and attributes to the rendering device.
     //                result = sceAudioOut2ContextPush(tmpContext->contextHandle, SCE_AUDIO_OUT2_BLOCKING_ASYNC);
     //                SCE_DBG_ASSERT(result == SCE_OK);
     //            }
     //        }
     //    }
     //}
