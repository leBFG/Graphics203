#pragma once
#include <sktbdpch.h>
#include "Mathematics.h"
#include "Skateboard/Input/TriggerEffectData.h"
typedef int32_t UserID;
namespace Skateboard
{
	class DeviceManager;
	class Input
	{
	public:
		//<summary>
		// static, platform Agnostic Input class used to retrieve data from the device manager that i splatform specific and apply button and key logic
		//</summary>
		static void RegisterDeviceManager(DeviceManager* DM) { devices = DM; };

		//static void ReadyForNextFrame();
		static void ReleaseAllKeys();

		static bool IsKeyPressed(Skateboard::Keys key, UserID userid = -1);
		static bool IsKeyDown(Skateboard::Keys  key, UserID userid = -1);
		static bool IsKeyReleased(Skateboard::Keys  key, UserID userid = -1);

		static void SetKeyDown(uint8_t key);
		static void SetKeyUp(uint8_t key);

		static void SetMousePos(int x, int y, UserID userid = -1);
		static void SetMouseVisible(bool active);
		static int2 GetMousePos();
		static int2 GetWindowSize();
		static bool IsMouseButtonDown(uint8_t button);
		static bool IsMouseButtonPressed(uint8_t button);
		static bool IsMouseButtonReleased(uint8_t button);
		static bool IsMouseVisible();


		bool isGamePadConnected(UserID userId = -1);

		static bool IsButtonPressed(uint32_t button, UserID userid = -1);
		static bool IsButtonDown(uint32_t button, UserID userid = -1);
		static bool IsButtonReleased(uint32_t button, UserID userid = -1);

		static float GetLeftStickX(UserID userid = -1);
		static float GetLeftStickY(UserID userid = -1);
		static float2 GetLeftStick(UserID userid = -1);

		static float GetRightStickX(UserID userid = -1);
		static float GetRightStickY(UserID userid = -1);
		static float2 GetRightStick(UserID userid = -1);

		static float GetLeftTrigger(UserID userid = -1);
		static float GetRightTrigger(UserID userid = -1);

		static glm::quat GetGyro(UserID userid = -1);
		static glm::float3 GetAcceleration(UserID userid = -1);

		//returns a number to an array of active touches, each tuch within an array constains an id of the current touch and its x and y as UV coordinates of the touchpad
		static std::vector<Touch> GetTouches(UserID userid = -1);

		static void SetButtonDown(uint8_t button, UserID userid = -1);
		static void SetButtonUp(uint8_t button, UserID userid = -1);

		static void SetHapticResponse(const SKTBDeviceHapticSettings& settings, UserID userid = -1);
		/// <summary>
		/// Sets the vibration state of the controller
		/// </summary>
		/// <param name="largeMotorSpeed"> )0.0f -- 1.0f</param>
		/// <param name="smallMotorSpeed">0.0f -- 1.0f</param>
		/// <param name="vibrationTimeInMs">0 -- 2500</param>
		/// <param name="userid"></param>
		static void SetSimpleVibration(float largeMotorSpeed, float smallMotorSpeed, uint32_t vibrationTimeInMs, UserID userid = -1);	

	protected:

		static inline void EnsureDefaultUser(UserID& userid);
		inline static DeviceManager* devices = nullptr;
	};

	
	
}