
#include "ScreamAudioRendererAPI.h"
#include "Skateboard/Platform.h"
#include <PlaystationPlatform.cpp>
namespace Skateboard {

void* Example_AllocateAlignedMemory(int32_t bytes, uint32_t alignment)
{
    uintptr_t mask = 0;
    if (alignment)
    {
        unsigned int i;
        for (i = 2; i; i <<= 1)
        {
            if (alignment <= i) break;
        }
        mask = i - 1;
    }

    char* p = (char*)malloc(bytes + mask + sizeof(int*));
    if (p)
    {
        int** memory = (int**)((uintptr_t)(p + mask + sizeof(int*)) & ~mask);
        *(memory - 1) = (int*)p;
        return memory;
    }

    return 0;
}

void Example_FreeAlignedMemory(void* memory)
{
    free(*((int**)memory - 1));
}

void* example_malloc(int32_t bytes, int32_t use)
{
#if (TARGET_OS_WIN32)
    UNREFERENCED_PARAMETER(use);
#endif
    return Example_AllocateAlignedMemory(bytes, 16);
}

/*
This is the memory freeing routine also passed to Scream
during intialization.
*/
void example_free(void* memory)
{
    Example_FreeAlignedMemory(memory);
}

SceScreamSoundBank* Example_LoadSoundBankWithPath(const char* bankPath, const char* bankName)
{
    // Concatenate folder and file names together.
    char pathBuffer[512];
    size_t roomNeeded = strlen(bankPath) + strlen(bankName) + 1;
    if (roomNeeded > sizeof(pathBuffer))
    {
        printf("pathBuffer too small in Example_LoadSoundBank(%s)\n", bankName);
        return NULL;
    }
    strcpy_s(pathBuffer, 512, bankPath);
    strcat_s(pathBuffer, 512, bankName);

    // Try to load the bank.
    SceScreamSoundBank* screamBank = sceScreamBankLoad(pathBuffer, 0);
    if (screamBank != NULL)
    {
        printf("sceScreamBankLoadEx(\"%s\",0) succeeded.\n", pathBuffer);
    }
    
    return screamBank;
}

void ScreamAudioRendererAPI::Init()
{ 
   


   

}

void ScreamAudioRendererAPI::Initialize()
{
    SceScreamPlatformInit screamInit;
    memset(&screamInit, 0, sizeof(SceScreamPlatformInit));
    screamInit.size = sizeof(SceScreamPlatformInit);
    sceScreamFillDefaultScreamPlatformInitArgs(&screamInit);

    screamInit.memAlloc = example_malloc;
    screamInit.memFree = example_free;

    //screamInit.pGroupMixerFile = s_pGroupMixerFile;
    //screamInit.pBussConfigFile = s_pBussConfigFile;
    //screamInit.pDistanceModelFile = s_pDistanceModelFile;
    //screamInit.extWaveformCallback = s_waveformCallback;
    screamInit.synthParams.initFlags |= SCE_SCREAM_SND_SYNTH_INIT_FLAG_ENABLE_PERSONAL_STEREO | SCE_SCREAM_SND_SYNTH_INIT_FLAG_ENABLE_VIBRATION | SCE_SCREAM_SND_SYNTH_INIT_FLAG_ENABLE_PAD_SPEAKER | SCE_SCREAM_SND_SYNTH_INIT_FLAG_ENABLE_BGM;

    screamInit.synthParams.numPremasterSubmixes = 12;
    screamInit.synthParams.maxLocalPlayers = 4;
    /*screamInit.synthParams.numLocalPlayers = 2;
    auto p1 = Skateboard::Platform::GetUserManager()->GetDefaultUser().id;
    auto p2 = Skateboard::Platform::GetUserManager()->GetUsersByType(Skateboard::UserType::SKTB_USER_LOGIN_PLAYER)[1].id;
    playerMap[p1] = 0;
    playerMap[p2] = 1;
    screamInit.synthParams.localPlayerIDs[0] = p1;
    screamInit.synthParams.localPlayerIDs[1] = p2;*/



    Skateboard::Platform::GetPlatform();

    auto result = sceScreamStartSoundSystem(&screamInit);
    if (result != 0)
    {
        printf("sceScreamStartSoundSystem() failed, returned %d\n", result);
    }


}

void ScreamAudioRendererAPI::LoadBank(const char* filename, const char* Key)
{

    // sanitize path --ensure that both app0 and assets folder are allowed
    std::filesystem::path path(Skateboard::SanitizeFilePath(filename));
    path.replace_extension(".bnk");


    // Load Bank binary
    // Note: Specified path to Bank file is for demonstration purposes only,
    // and may need to be changed to suit your environment
    screamBank = sceScreamBankLoad(path.c_str(), 0);
    
    auto r = sceScreamGetLastLoadError();
    

}

uint64_t ScreamAudioRendererAPI::PlayAudio(const char* AudioName, Skateboard::SkbdSoundParams params)
{
    // Instantiate SceScreamSoundParams structure and set members
    SceScreamSoundParams soundParams;
    soundParams.size = sizeof(SceScreamSoundParams);
    memset(&soundParams, 0, sizeof(SceScreamSoundParams));
    soundParams.size = sizeof(SceScreamSoundParams);
    soundParams.gain = 1.0f;
    soundParams.azimuth = 0;

    //sceScreamSynthAddLocalPlayerBySystemID(Skateboard::Platform::GetUserManager()->GetDefaultUser().id, &outLogicalPlayerID);
    soundParams.logicalPlayerId = playerMap[params.userId];//Skateboard::Platform::GetUserManager()->GetDefaultUser().id;
    soundParams.outputDest = SCE_SCREAM_SND_OUTPUT_DEST_CUSTOM(playerMap[params.userId], SCE_SCREAM_SND_OUTPUT_DEST_PERSONAL_PAD_SPEAKER);
    // Set flags to indicate which parameters are being set
    soundParams.mask = SCE_SCREAM_SND_MASK_GAIN | SCE_SCREAM_SND_MASK_PAN_AZIMUTH | SCE_SCREAM_SND_MASK_SYNTH_PARAMS ;

    if (params.userId != 0) {
        soundParams.mask |= SCE_SCREAM_SND_MASK_OUTPUT_DEST | SCE_SCREAM_SND_MASK_PLAYER_ID;
    }

    // Play Sounds contained in the SwordFight BNK file
    auto res = sceScreamPlaySoundByName(screamBank, AudioName, &soundParams);
   

    if (res != 0)
    {
        printf("sceScreamPlaySoundByName() failed, returned %d\n", res);
    }
    return res;
}

uint64_t ScreamAudioRendererAPI::StopAudio(uint64_t hnd)
{
    return sceScreamStopSound(hnd, SCE_SCREAM_SND_STOP_BEHAVIOR_SILENCE);
}

uint64_t ScreamAudioRendererAPI::PauseAudio(uint64_t hnd)
{
    return sceScreamPauseSound(hnd);
}

uint64_t ScreamAudioRendererAPI::ResumeAudio(uint64_t hnd)
{
    return sceScreamContinueSound(hnd);
}

void ScreamAudioRendererAPI::EndScene()
{
    sceScreamCloseStreaming();
    sceScreamStopSoundSystem();
}

void ScreamAudioRendererAPI::AddLocalPlayer(int32_t usrId)
{
    uint32_t logicalId;
    sceScreamSynthAddLocalPlayerBySystemID(usrId, &logicalId);
    playerMap[usrId] = logicalId;
}

void ScreamAudioRendererAPI::RemoveLocalPlayer(int32_t usrId)
{

    auto res = sceScreamSynthDeleteLocalPlayerBySystemID(usrId);
    if (res)
        playerMap.erase(usrId);

}

void ScreamAudioRendererAPI::SetGroupVolume(int32_t group, float vol)
{
    sceScreamSetMasterVolume(group, vol);
}

void ScreamAudioRendererAPI::SetGlobalVariable(const char* name, float value)
{
    sceScreamSetGlobalVariableByName(name, value);
    
}

void ScreamAudioRendererAPI::SetGlobalRegister(int reg, int val)
{
    sceScreamSetSFXGlobalReg(reg, val);

}

}