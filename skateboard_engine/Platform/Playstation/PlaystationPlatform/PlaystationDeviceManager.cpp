
#include "PlaystationDeviceManager.h"
#include <libsysmodule.h>
#include "Skateboard/Platform.h"
#include "Skateboard/User.h"

namespace Skateboard
{

    int32_t PlaystationDeviceManager::Init()
    {
        int32_t err = 0;

        // Call this once after process start up.
        // If this function is called more than once, result SCE_OK is returned.
         // However, internal functions are not processed.
        err = scePadInit();

        if (err != SCE_OK)
        {
            SKTBD_MSG_ERROR("Failed to initialise the pad library!");
        }

        if (sceSysmoduleLoadModule(SCE_SYSMODULE_KEYBOARD) != SCE_OK) {
            // Error handling
            SKTBD_MSG_WARN("Failed to load keyboard sysmodule! :[");
        }
        else
        if (sceKeyboardInit() != SCE_OK)
        {
            SKTBD_MSG_WARN("Failed to initialise the keyboard library!");
        }

        SKTBD_LOG_ASSERT(!err, "Device manager failed to initialise");

        return err;
    }

    int32_t PlaystationDeviceManager::GetDevicesConnected(UserID user, DeviceType type)
    {
        switch (type)
        {
        case Skateboard::SKTB_Keyboard:
            break;
        case Skateboard::SKTB_GamePad:
            return GamePads[user].latestData.connected;
            break;
        case Skateboard::SKTB_Mouse:
            break;
        case Skateboard::SKTB_Custom:
            break;
        default:
            break;
          
        }

        return 0;
    }

    std::vector<Keys>& PlaystationDeviceManager::GetKeys(UserID user)
    {
        static std::vector<Keys> Keys;
        return Keys;
    }

    void PlaystationDeviceManager::Update()
    {
        // Potential solution for updating data just once
        for (auto& gpad : GamePads) 
        {
            auto& pad = gpad.second;
            pad.oldData = pad.latestData;
            scePadReadState(pad.handle, &pad.latestData);

            if(pad.latestData.connectedCount != pad.oldData.connectedCount)
            SKTBD_MSG_INFO("connected pad count: {0} user: {1}", pad.latestData.connectedCount, pad.userID);

            if (pad.latestData.connected == true && pad.oldData.connected == false)
            {
                DeviceConnectedEvent connected(pad.userID, DeviceType::SKTB_GamePad, true);
                Platform::PlatformDispatchEvent(connected);
                
                SKTBD_MSG_INFO("connected pad: {0}", pad.userID);
            }

            if (pad.latestData.connected == false && pad.oldData.connected == true)
            {
                DeviceConnectedEvent connected(pad.userID, DeviceType::SKTB_GamePad, false);
                Platform::PlatformDispatchEvent(connected);

                SKTBD_MSG_INFO("disconnected pad: {0}", pad.userID);
            }

            if (pad.isTimedVibing)
            {
                if ((sceKernelGetProcessTime() / 1000) - pad.startVibrationTime >= pad.vibrationTimeInMS)
                {
			        SetSimpleVibration(pad.userID, 0.0f, 0.0f, 0);
					pad.isTimedVibing = false;
				}
			}
        }
    }

    void PlaystationDeviceManager::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<AppLoginEvent>(BIND_EVENT(PlaystationDeviceManager::OnUserLogin));
        dispatcher.Dispatch<AppLogOutEvent>(BIND_EVENT(PlaystationDeviceManager::OnUserLogOut));
    }

    void PlaystationDeviceManager::MessageInputEvents()
    {
    }

    void PlaystationDeviceManager::RemoveDevices(User user)
    {
        if (user.type == UserType::SKTB_USER_LOGIN_PLAYER)
        {
            scePadClose(GamePads[user.id].handle);
            GamePads.erase(user.id);
        }
    }

    void PlaystationDeviceManager::AddDevices(User user)
    {
        if (user.type == UserType::SKTB_USER_LOGIN_PLAYER)
        {
            int32_t handle = 0;

            handle = scePadOpen(user.id, SCE_PAD_PORT_TYPE_STANDARD, 0, nullptr);

            if (handle > 0)
            {
                GamePad pad;
                memset(&pad, 0, sizeof(GamePad));

                pad.DeviceType = DeviceType::SKTB_GamePad;
                pad.handle = handle;
                pad.userID = user.id;
          
                auto ret = scePadGetControllerInformation(pad.handle, &pad.controllerInfo);
                if (ret < 0)
                    SKTBD_MSG_WARN("Failed to retrieve controller information");

                GamePads[user.id] = std::move(pad);
            }
            else if (handle<0)
                SKTBD_MSG_WARN("Failed to initialize controller port!");
        }
    }

    bool PlaystationDeviceManager::OnUserLogin(AppLoginEvent& e)
    {
        AddDevices(e.GetUser());
        return false;
    }

    bool PlaystationDeviceManager::OnUserLogOut(AppLogOutEvent& e)
    {
        RemoveDevices(e.GetUser());
        return false;
    }

    uint8_t* PlaystationDeviceManager::GetModifierKeys(UserID user)
    {
        return nullptr;
    }

    uint32_t& PlaystationDeviceManager::GetButtons(UserID user)
    {
        GamePad& pad = GamePads[user];
        return pad.latestData.buttons;
    }

    uint32_t& PlaystationDeviceManager::GetPreviousButtons(UserID user)
    {
        return GamePads[user].oldData.buttons;
    }

    float2 PlaystationDeviceManager::GetRightThumbstick(UserID user)
    {
        GamePad& pad= GamePads[user];

        constexpr float remapfloat = 1.05f / 128.f;        // 128 * 1.f / 128.f = 1.f? WRONG akchually its 0.99267jnkajirng
        
        int32_t xval = pad.latestData.rightStick.x - 0x80; // data is stored as uint8_t with default value being 0x80 (128)
        int32_t yval = pad.latestData.rightStick.y - 0x80;

        int32_t dz = pad.controllerInfo.stickInfo.deadZoneRight;
        
        return (std::max(abs(xval), abs(yval)) > dz) ? float2(std::clamp(xval * remapfloat, -1.f, 1.f), -std::clamp(yval * remapfloat, -1.f, 1.f)) : float2(0, 0);
    }
    
    float2 PlaystationDeviceManager::GetLeftThumbstick(UserID user)
    {
        GamePad& pad = GamePads[user];
        
        constexpr float remapfloat = 1.05f / 128.f;       // 128.f * 1.f / 128.f = 1.f? WRONG akchually its 0.99267jnkajirng

        int32_t xval = pad.latestData.leftStick.x - 0x80; // data is stored as uint8_t with default value being 0x80 (128)
        int32_t yval = pad.latestData.leftStick.y - 0x80;

        int32_t dz = pad.controllerInfo.stickInfo.deadZoneRight;

        return (std::max(abs(xval), abs(yval)) > dz) ? float2(std::clamp(xval * remapfloat, -1.f, 1.f), -std::clamp(yval * remapfloat, -1.f, 1.f)) : float2(0, 0);
    }

    float PlaystationDeviceManager::GetTrigger(UserID user, Side_ side)
    {
        GamePad& pad = GamePads[user];

        float ret = 0;

        if(side == Side_::Right)
            ret = Remap(pad.latestData.analogButtons.r2, 0, 255);
        else
            ret = Remap(pad.latestData.analogButtons.l2, 0, 255);
    
        return ret;
    }

    glm::quat PlaystationDeviceManager::GetGyro(UserID user)
    {
        GamePad& pad = GamePads[user];

        return glm::quat(-pad.latestData.orientation.w, pad.latestData.orientation.x,pad.latestData.orientation.y,-pad.latestData.orientation.z );

    }

    glm::float3 PlaystationDeviceManager::GetAcceleration(UserID user)
    {
        GamePad& pad = GamePads[user];
        return { pad.latestData.acceleration.x, pad.latestData.acceleration.y, pad.latestData.acceleration.z };
    }

    std::vector<Touch> PlaystationDeviceManager::GetTouches(UserID user)
    {
        GamePad& pad = GamePads[user];
        
        int touchcount = pad.latestData.touchData.touchNum;

        std::vector<Touch> touches(touchcount);

        float remapX = 1.f / pad.controllerInfo.touchPadInfo.resolution.x; //playstation is ractangle
        float remapY = 1.f / pad.controllerInfo.touchPadInfo.resolution.y;

        for (int i = 0; i < touchcount; i++)
        {
            touches[i].id = pad.latestData.touchData.touch[i].id;
            touches[i].xy.x = pad.latestData.touchData.touch[i].x * remapX;
            touches[i].xy.y = pad.latestData.touchData.touch[i].y * remapY;
        }

        return touches;
    }

    void PlaystationDeviceManager::SetHapticResponse(uint32_t user, const SKTBDeviceHapticSettings& settings)
    {
        uint8_t side = uint8_t(settings.side);
        ScePadTriggerEffectParam triggerEffectParam = {};
        memset(&triggerEffectParam, 0x00, sizeof(ScePadTriggerEffectParam));
        // Mode
        triggerEffectParam.command[side].mode = (ScePadTriggerEffectMode)settings.mode;

        if (settings.side == Side_::Left)
            triggerEffectParam.triggerMask = SCE_PAD_TRIGGER_EFFECT_TRIGGER_MASK_L2;
        else
            triggerEffectParam.triggerMask = SCE_PAD_TRIGGER_EFFECT_TRIGGER_MASK_R2;


        switch (triggerEffectParam.command[side].mode)
        {
        case SCE_PAD_TRIGGER_EFFECT_MODE_OFF:
            break;
        case SCE_PAD_TRIGGER_EFFECT_MODE_FEEDBACK:
            // Feedback Param
            triggerEffectParam.command[side].commandData.feedbackParam.position = settings.commandData.feedbackParam.position;
            triggerEffectParam.command[side].commandData.feedbackParam.strength = settings.commandData.feedbackParam.strength;
            break;
        case SCE_PAD_TRIGGER_EFFECT_MODE_WEAPON:
            triggerEffectParam.command[side].commandData.weaponParam.startPosition = settings.commandData.weaponParam.startPosition;
            triggerEffectParam.command[side].commandData.weaponParam.endPosition = settings.commandData.weaponParam.endPosition;
            triggerEffectParam.command[side].commandData.weaponParam.strength = settings.commandData.weaponParam.strength;

            break;

        case SCE_PAD_TRIGGER_EFFECT_MODE_VIBRATION:
            triggerEffectParam.command[side].commandData.vibrationParam.position = settings.commandData.vibrationParam.position;
            triggerEffectParam.command[side].commandData.vibrationParam.amplitude = settings.commandData.vibrationParam.amplitude;
            triggerEffectParam.command[side].commandData.vibrationParam.frequency = settings.commandData.vibrationParam.frequency;
            break;

        case SCE_PAD_TRIGGER_EFFECT_MODE_MULTIPLE_POSITION_FEEDBACK:
            for(int i = 0;i< SKTBD_PAD_TRIGGER_EFFECT_CONTROL_POINT_NUM;i++)
                triggerEffectParam.command[side].commandData.multiplePositionFeedbackParam.strength[i] = settings.commandData.multiplePositionFeedbackParam.strength[i];
            break;


        case SCE_PAD_TRIGGER_EFFECT_MODE_SLOPE_FEEDBACK:
            triggerEffectParam.command[side].commandData.slopeFeedbackParam.startPosition = settings.commandData.slopeFeedbackParam.startPosition;
            triggerEffectParam.command[side].commandData.slopeFeedbackParam.endPosition = settings.commandData.slopeFeedbackParam.endPosition;
            triggerEffectParam.command[side].commandData.slopeFeedbackParam.startStrength = settings.commandData.slopeFeedbackParam.startStrength;
            triggerEffectParam.command[side].commandData.slopeFeedbackParam.endStrength = settings.commandData.slopeFeedbackParam.endStrength;


            break;
        case SCE_PAD_TRIGGER_EFFECT_MODE_MULTIPLE_POSITION_VIBRATION:
            for (int i = 0; i < SKTBD_PAD_TRIGGER_EFFECT_CONTROL_POINT_NUM; i++)
				triggerEffectParam.command[side].commandData.multiplePositionVibrationParam.amplitude[i] = settings.commandData.multiplePositionVibrationParam.amplitude[i];
            triggerEffectParam.command[side].commandData.multiplePositionVibrationParam.frequency = settings.commandData.multiplePositionVibrationParam.frequency;

            break;
        default:
            break;
        }

        GamePad& pad = GamePads[user];
        auto ret = scePadSetTriggerEffect(pad.handle, &triggerEffectParam);
        if (ret < 0) {
            SKTBD_MSG_WARN("SOMETHING AINT RIGHT with trigger effects");
        }
    }

    void PlaystationDeviceManager::SetSimpleVibration(UserID user, float largeMotorSpeed, float smallMotorSpeed, uint32_t vibrationTimeInMs)
    {
        
        GamePad& pad = GamePads[user];
        ScePadVibrationParam param;

        if (largeMotorSpeed < 0.0f || largeMotorSpeed > 1.0f || smallMotorSpeed < 0.0f || smallMotorSpeed > 1.0f) {
            SKTBD_MSG_WARN("Invalid motor speed");
            return;
        }

        if (vibrationTimeInMs > 2500) {
            SKTBD_MSG_WARN("Invalid vibration time");
            return;
        }

        if ((largeMotorSpeed == 0.0f && smallMotorSpeed == 0.0f) || vibrationTimeInMs == 0) {
            param.largeMotor = 0;
            param.smallMotor = 0;
            pad.isTimedVibing = false;
        }
        else {
            param.largeMotor = 255 * largeMotorSpeed;
            param.smallMotor = 255 * smallMotorSpeed;
            pad.vibrationTimeInMS = vibrationTimeInMs;
            pad.startVibrationTime = sceKernelGetProcessTime()/1000;
            pad.isTimedVibing = true;
        }

        scePadSetVibrationMode(pad.handle, SCE_PAD_VIBRATION_MODE_COMPATIBLE);
        scePadSetVibration(pad.handle, &param);
    }

    void* PlaystationDeviceManager::GetRawData(UserID user, DeviceType type, size_t& dataSize)
    {
        switch (type)
        {
        case DeviceType::SKTB_GamePad:
            dataSize = sizeof(ScePadData);
            return &GamePads[user].latestData;
            break;

        case DeviceType::SKTB_Keyboard:

            break;
        case DeviceType::SKTB_Mouse:

            break;
        }
        return nullptr;
    }

    std::vector<Skateboard::Keys>& PlaystationDeviceManager::GetPrevKeys(UserID user)
    {
        static std::vector<Keys> prevkeys;
        return prevkeys;
    }

    int2 PlaystationDeviceManager::GetMousePosition(UserID user)
    {
        return { 0,0 };
    }

    uint8_t PlaystationDeviceManager::GetMouseButtons(UserID user)
    {
        return 0;
    }

    uint8_t PlaystationDeviceManager::GetPrevMouseButtons(UserID user)
    {
        return 0;
    }

    void PlaystationDeviceManager::SetMousePosition(int2 pos)
    {
    }

    void PlaystationDeviceManager::ShowMouse(bool b)
    {
    }

    int2 PlaystationDeviceManager::GetWindowSize()
    {
        return { 0,0 };
    }
}
