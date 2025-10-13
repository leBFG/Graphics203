#pragma once
#define SCE_OK 0

#define SKTBD_LOG_COMPONENT "DEVICE MANAGER IMPL"
#include "Skateboard/Log.h"

#include "Skateboard/Input/DeviceManager.h"
#pragma comment (lib,"SceKeyboard_stub_weak")
#include <keyboard.h>
#pragma comment (lib,"SceMouse_stub_weak")
#include <mouse.h>
#include <pad.h>

namespace Skateboard
{
	struct GamePad  :	public Device<ScePadData> {
		bool isTimedVibing = false;
		uint64_t startVibrationTime = 0;
		int vibrationTimeInMS = 0;
		ScePadControllerInformation controllerInfo;
	};
	struct Keyboard	:	public Device<SceKeyboardData> {};
	struct Mouse    :	public Device<SceMouseData> {};

	class PlaystationDeviceManager final : public DeviceManager
	{
		friend class PlaystationPlatform;

		inline static float Remap(float value, float min, float max, float newMin = -1.0f, float newMax = 1.0f)
		{
			return (value - min) / (max - min) * (newMax - newMin) + newMin;
		}

		inline static float RemapNormalized(float value, float min, float max, float newMin = 0.0f, float newMax = 1.0f)
		{
			return (value - min) / (max - min) * (newMax - newMin) + newMin;
		}
		
		void Update() override;
		void OnEvent(Event& e) override;
		void MessageInputEvents() override;

		bool OnUserLogin(AppLoginEvent& e);
		bool OnUserLogOut(AppLogOutEvent& e);

		void RemoveDevices(User user);
		void AddDevices(User user);

		std::unordered_map<UserID, GamePad> GamePads;
		//std::unordered_map<int32_t, Keyboard> kbds;
		//std::unordered_map<int32_t, Mouse> mc;
	

	public:
		DISABLE_COPY_AND_MOVE(PlaystationDeviceManager);
		PlaystationDeviceManager() = default;
		int32_t Init() override;

		int32_t GetDevicesConnected(UserID user, DeviceType type) override;

		//immediate mode inputs 
		std::vector<Keys>&    GetKeys(UserID user) override; // returns pointer to keyboard keys
		uint8_t*    GetModifierKeys(UserID user) override; // returns pointer to modifier keys

		uint32_t&	GetButtons(UserID user) override; // returns button mapping
		uint32_t&   GetPreviousButtons(UserID user) override; // returns button mapping

		float2		GetLeftThumbstick(UserID user) override; //returns snorm float from thumbstick X 0 = neutral
		float2		GetRightThumbstick(UserID user) override; //returns snorm float from thumbstick X 0 = neutral
		float		GetTrigger(UserID user, Side_ stick) override; // Y thumbstick
		glm::quat   GetGyro(UserID user) override; // gyro data 
		glm::float3 GetAcceleration(UserID user) override; //accelerometer data

		std::vector<Touch>	GetTouches(UserID user) override; // touch data, depending on number of touches supported returns all active touches

		void SetHapticResponse(uint32_t user, const SKTBDeviceHapticSettings& settings) override;

		void SetSimpleVibration(UserID user, float largeMotorSpeed, float smallMotorSpeed, uint32_t vibrationTimeInMS) override;

		void* GetRawData(UserID user, DeviceType type, size_t& dataSize) override;

		std::vector<Skateboard::Keys>& GetPrevKeys(UserID user) override;
		int2 GetMousePosition(UserID user) override;
		uint8_t GetMouseButtons(UserID user) override;
		uint8_t GetPrevMouseButtons(UserID user) override;
		void SetMousePosition(int2 pos) override;
		void ShowMouse(bool b) override;
		int2 GetWindowSize() override;
	};
}