#include "sktbdpch.h"
#include "Skateboard/Input/DeviceManager.h"

#include <GameInput.h>

#ifndef GAMEINPUT_API_VERSION
#define GAMEINPUT_API_VERSION 0
#endif

#if GAMEINPUT_API_VERSION == 1
using namespace GameInput::v1;
#endif

#if GAMEINPUT_API_VERSION >= 1
using namespace GameInput::v2;
#endif

//#if GAMEINPUT_API_VERSION >= 1
//device->GetDeviceInfo(&deviceInfo);
//#else
//deviceInfo = device->GetDeviceInfo();
//#endif

namespace Skateboard
{
	class WindowsDeviceManager final : public DeviceManager
	{
		uint32_t m_ButtonState;
		uint32_t m_OldButtorState;
		uint32_t m_keyState;


		void Update() override;
		void OnEvent(Event& e) override;
		void MessageInputEvents() override;

		bool OnUserLogin(AppLoginEvent& e);
		bool OnUserLogOut(AppLogOutEvent& e);

		void RemoveDevices(User user);
		void AddDevices(User user);
		
	public:
		DISABLE_COPY_AND_MOVE(WindowsDeviceManager)
		WindowsDeviceManager() {};
		~WindowsDeviceManager();
 

		int32_t Init() override;

		int32_t GetDevicesConnected(UserID user, DeviceType type) override;

		//immediate mode inputs 
		std::vector<Skateboard::Keys>& GetKeys(UserID user) override; // returns pointer to keyboard keys
		std::vector<Skateboard::Keys>& GetPrevKeys(UserID user) override; // returns pointer to keyboard keys
		uint8_t* GetModifierKeys(UserID user) override; // returns pointer to modifier keys

		uint32_t& GetButtons(UserID user) override; // returns button mapping
		uint32_t& GetPreviousButtons(UserID user) override; // returns button mapping

		int2   GetMousePosition(UserID user) override;
		uint8_t  GetMouseButtons(UserID user) override;
		uint8_t  GetPrevMouseButtons(UserID user) override;
		void SetMousePosition(int2 pos) override;
		void ShowMouse(bool b) override;
		int2 GetWindowSize() override;

		float2		GetLeftThumbstick(UserID user) override; //returns snorm float from thumbstick X 0 = neutral
		float2		GetRightThumbstick(UserID user) override; //returns snorm float from thumbstick X 0 = neutral
		float		GetTrigger(UserID user, Side_ stick) override; // Y thumbstick
		glm::quat   GetGyro(UserID user) override; // gyro data 
		glm::float3 GetAcceleration(UserID user) override; //accelerometer data

		std::vector<Touch>	GetTouches(UserID user) override; // touch data, depending on number of touches supported returns all active touches

		void SetHapticResponse(uint32_t user, const SKTBDeviceHapticSettings& settings) {};

		void SetSimpleVibration(UserID user, float largeMotorSpeed, float smallMotorSpeed, uint32_t vibrationTimeInMS) {};

		void* GetRawData(UserID user, DeviceType type, size_t& dataSize) override;

		


	private:
		IGameInput* g_gameInput = nullptr;
		IGameInputDevice* g_device = nullptr;
		std::vector<Skateboard::Keys> skt_keys, skt_prevKeys;
		uint32_t numKeys = 0;
		HWND hwnd;

		uint8_t skt_mouseButtons, skt_prevMouseButtons;
	public:
		inline static std::vector<IGameInputDevice*> g_mouseDevices;
	};
}