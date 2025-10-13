#include "sktbdpch.h"
#include "WindowsDeviceManager.h"
#include "Skateboard/User.h"
#include "Skateboard/Platform.h"

#define SKTBD_LOG_COMPONENT "WindowsDeviceManager"
#include "Skateboard/Log.h"

namespace Skateboard
{
	void WindowsDeviceManager::Update()
	{
		IGameInputReading* reading = nullptr;

		/// Keyboard Input
		if(g_gameInput)
		if (SUCCEEDED(g_gameInput->GetCurrentReading(GameInputKindKeyboard, g_device, &reading)))
		{
			// If no device has been assigned to g_gamepad yet, set it
			// to the first device we receive input from. (This must be
			// the one the player is using because it's generating input.)
			//if (!g_gamepad) reading->GetDevice(&g_gamepad);

			// Retrieve the fixed-format gamepad state from the reading.
			//GameInputGamepadState state;
			//if(reading->GetGamepadState(&state));
			//std::memcpy(g_prevKeys, g_keys, maxKeys *sizeof(GameInputKeyState));

			const int maxKeys = 30;
			GameInputKeyState g_keys[maxKeys];

			numKeys = reading->GetKeyCount();
			reading->GetKeyState(numKeys, g_keys);
			skt_prevKeys = skt_keys;
			skt_keys.clear();
			for (uint32_t i = 0; i < numKeys; i++)
				skt_keys.push_back((Skateboard::Keys)g_keys[i].scanCode);

			//if(numKeys >=1)
			//	std::cout<<"TestedA: "<< g_keys[0].scanCode << std::endl;

			reading->Release();

			// Application-specific code to process the gamepad state goes here.
			//if(state.leftThumbstickX ) 

		}

		/// Mouse Input
		if(g_gameInput)
		if (SUCCEEDED(g_gameInput->GetCurrentReading(GameInputKindMouse, nullptr, &reading)))
		{
			GameInput::v2::GameInputMouseState g_mouse;
			auto b = reading->GetMouseState(&g_mouse);

			skt_prevMouseButtons = skt_mouseButtons;

			skt_mouseButtons = g_mouse.buttons;
			

			reading->Release();
		}

		// If an error is returned from GetCurrentReading(), it means the
		// gamepad we were reading from has disconnected. Reset the
		// device pointer, and go back to looking for an active gamepad.
		else if (g_device)
		{
			g_device->Release();
			g_device = nullptr;
		}
	}

	void WindowsDeviceManager::OnEvent(Event& e)
	{
	}

	void WindowsDeviceManager::MessageInputEvents()
	{
	}

	bool WindowsDeviceManager::OnUserLogin(AppLoginEvent& e)
	{
		return false;
	}

	bool WindowsDeviceManager::OnUserLogOut(AppLogOutEvent& e)
	{
		return false;
	}

	void WindowsDeviceManager::RemoveDevices(User user)
	{
	}

	void WindowsDeviceManager::AddDevices(User user)
	{
	}

	WindowsDeviceManager::~WindowsDeviceManager()
	{
		if (g_device) g_device->Release();
		if (g_gameInput) g_gameInput->Release();
	}

	// Callback function to handle device changes
	void CALLBACK DeviceCallback(
		GameInputCallbackToken callbackToken,
		void* context,
		IGameInputDevice* device,
		uint64_t timestamp,
		GameInputDeviceStatus currentStatus,
		GameInputDeviceStatus previousStatus)
	{

		std::string Event;
		// Output information about the device status
		if (currentStatus & GameInput::v2::GameInputDeviceConnected) {
			Event = "Device connected: ";
		}
		else {
			Event = "Device disconnected: ";
		}

		SKTBD_MSG_INFO(Event + "Mouse")

		WindowsDeviceManager::g_mouseDevices.push_back(device);
	}

	
	int32_t WindowsDeviceManager::Init()
	{
		auto res = GameInputCreate(&g_gameInput);
		hwnd = GetActiveWindow();

		GameInputCallbackToken token;
		if(g_gameInput)
		g_gameInput->RegisterDeviceCallback(
			nullptr,                                      // Don't filter to events from a specific device 
			GameInputKindGamepad | GameInputKindKeyboard, // Enumerate gamepads and keyboards
			GameInputDeviceAnyStatus,                     // Any device status 
			GameInputBlockingEnumeration,                 // Enumerate synchronously 
			nullptr,                                      // No callback context parameter 
			DeviceCallback,                           // Callback function 
			&token);                                   // Generated token 


		///auto res = g_gameInput->(GameInput::v2::GameInputKindMouse, g_mouseDevices);

		return res;
	}

	

	int32_t WindowsDeviceManager::GetDevicesConnected(UserID user, DeviceType type)
	{
		return 0;
	}

	std::vector<Skateboard::Keys>& WindowsDeviceManager::GetKeys(UserID user)
	{
		return skt_keys;
	}

	std::vector<Skateboard::Keys>& WindowsDeviceManager::GetPrevKeys(UserID user)
	{
		return skt_prevKeys;
	}

	uint8_t* WindowsDeviceManager::GetModifierKeys(UserID user)
	{
		return nullptr;
	}

	uint32_t& WindowsDeviceManager::GetButtons(UserID user)
	{
		// TODO: insert return statement here
		return m_ButtonState;
	}

	uint32_t& WindowsDeviceManager::GetPreviousButtons(UserID user)
	{
		// TODO: insert return statement here
		return  m_OldButtorState;
	}

	int2 WindowsDeviceManager::GetMousePosition(UserID user)
	{
		// Get absolute mouse position using Win32 API, welll achtuallly, relative to the main window, there is no window abstraction so french bread with it
		POINT mousePos;
		GetCursorPos(&mousePos);
		ScreenToClient(hwnd, &mousePos);
		return int2(mousePos.x,mousePos.y);
	}

	uint8_t WindowsDeviceManager::GetMouseButtons(UserID user)
	{
		return skt_mouseButtons;
	}

	uint8_t WindowsDeviceManager::GetPrevMouseButtons(UserID user)
	{
		return skt_prevMouseButtons;
	}

	void WindowsDeviceManager::SetMousePosition(int2 pos)
	{
		POINT mousePos = POINT(pos.x, pos.y);

		ClientToScreen(hwnd, &mousePos);
		SetCursorPos(mousePos.x, mousePos.y);

	}

	void WindowsDeviceManager::ShowMouse(bool b)
	{

		ShowCursor(b);
	}

	int2 WindowsDeviceManager::GetWindowSize()
	{
		RECT rect;
		int2 ret = int2();
		if (GetWindowRect(hwnd, &rect))
		{
			ret.x = rect.right - rect.left;
			ret.y= rect.bottom - rect.top;
		}
		return ret;
	}

	float2 WindowsDeviceManager::GetLeftThumbstick(UserID user)
	{
		return float2();
	}

	float2 WindowsDeviceManager::GetRightThumbstick(UserID user)
	{
		return float2();
	}

	float WindowsDeviceManager::GetTrigger(UserID user, Side_ stick)
	{
		return 0.0f;
	}

	glm::quat WindowsDeviceManager::GetGyro(UserID user)
	{
		return glm::quat();
	}

	glm::float3 WindowsDeviceManager::GetAcceleration(UserID user)
	{
		return glm::float3();
	}

	std::vector<Touch> WindowsDeviceManager::GetTouches(UserID user)
	{
		return std::vector<Touch>();
	}

	void* WindowsDeviceManager::GetRawData(UserID user, DeviceType type, size_t& dataSize)
	{
		return nullptr;
	}

}