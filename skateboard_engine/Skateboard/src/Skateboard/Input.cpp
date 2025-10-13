#include "sktbdpch.h"

#include "Input.h"
#include "Skateboard/Input/DeviceManager.h"
#include "Platform.h"
//#include "Skateboard/Input/ActionBinding.h"

namespace Skateboard
{

	// <summary>
	// Input class uses inputs stored and managed by the device manager which is platform specific abstarction to access to the devices, input class  
	// </summary>

	//void Input::ReadyForNextFrame()
	//{
	//	// TODO: With playstation they record the last 64 'input events' hence we may not need this?
	//
	//	// Copy the frame previous keys (really bad, but really simple. Can you think of smth better? hint: pointer)
	//	//memcpy(s_PlaystationInputData.LastFrameButtons, s_PlaystationInputData.CurrentFrameButtons, sizeof(s_PlaystationInputData.CurrentFrameButtons));
	//}

	void Input::ReleaseAllKeys()
	{

		//memset(s_PlaystationInputData.CurrentFrameButtons, NULL, sizeof(s_PlaystationInputData.CurrentFrameButtons));
	}

	bool Input::IsKeyPressed(Skateboard::Keys key, UserID userid)
	{
		EnsureDefaultUser(userid);

		// If not currently pressed, return false
		if(!IsKeyDown(key,userid))
			return false;

		// Check if previous frame was not pressed
		auto prevKeys = devices->GetPrevKeys(userid);
		for (auto& k : prevKeys)
			if (k == key)
				return false;

		return true;
	}

	bool Input::IsKeyDown(Skateboard::Keys key, UserID userid)
	{
		EnsureDefaultUser(userid);
		// Even though this looks complex, the array with pressed keys will usually be empty or low so not an expensive loop
		auto keys = devices->GetKeys(userid);
		for (auto& k : keys)
			if (k == key)
				return true;


		return false;
	}

	bool Input::IsKeyReleased(Skateboard::Keys key, UserID userid)
	{
		EnsureDefaultUser(userid);

		// If currently pressed, return false
		if (IsKeyDown(key, userid))
			return false;

		// Check if previous frame was pressed
		auto prevKeys = devices->GetPrevKeys(userid);
		for (auto& k : prevKeys)
			if (k == key)
				return true;

		return false;
	}

	void Input::SetKeyDown(uint8_t key)
	{
	}

	void Input::SetKeyUp(uint8_t key)
	{
	}

	void Input::SetMousePos(int x, int y, UserID userid)
	{
		devices->SetMousePosition(int2(x, y));
	}

	void Input::SetMouseVisible(bool active)
	{
		devices->ShowMouse(active);
	}

	float Input::GetLeftTrigger(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetTrigger(userId, Side_::Left);
	}

	float Input::GetRightTrigger(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetTrigger(userId, Side_::Right);
	}

	glm::quat Input::GetGyro(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetGyro(userId);
	}

	glm::float3 Input::GetAcceleration(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetAcceleration(userId);
	}

	std::vector<Touch> Input::GetTouches(UserID userid)
	{
		EnsureDefaultUser(userid);
		return devices->GetTouches(userid);
	}

	void Input::SetButtonDown(uint8_t button, UserID userId)
	{
		EnsureDefaultUser(userId);
		devices->GetButtons(userId) |= button;
	}
	void Input::SetHapticResponse(const SKTBDeviceHapticSettings& settings, UserID userId)
	{
		EnsureDefaultUser(userId);
		devices->SetHapticResponse(userId, settings);
		
	}

	void Input::SetSimpleVibration(float largeMotorSpeed, float smallMotorSpeed, uint32_t vibrationTimeInMs, UserID userId)
	{
		EnsureDefaultUser(userId);
		devices->SetSimpleVibration(userId, largeMotorSpeed, smallMotorSpeed, vibrationTimeInMs);
	}

	void Input::SetButtonUp(uint8_t button, UserID userId)
	{
		EnsureDefaultUser(userId);
		devices->GetButtons(userId) |= ~button;
	}

	bool Input::IsButtonDown(uint32_t button, UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetButtons(userId) & button;
	}

	bool Input::isGamePadConnected(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetDevicesConnected(userId, DeviceType::SKTB_GamePad);
	}

	bool Input::IsButtonPressed(uint32_t button, UserID userId)
	{
		EnsureDefaultUser(userId);
		return !(devices->GetPreviousButtons(userId) & button) && (devices->GetButtons(userId) & button);
	}

	bool Input::IsButtonReleased(uint32_t button, UserID userId)
	{
		EnsureDefaultUser(userId);
		return (devices->GetPreviousButtons(userId) & button) && !(devices->GetButtons(userId) & button);
	}

	float Input::GetLeftStickX(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetLeftThumbstick(userId).x;
	}

	float Input::GetLeftStickY(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetLeftThumbstick(userId).y;
	}
	
	float2 Input::GetLeftStick(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetLeftThumbstick(userId);
	}

	float Input::GetRightStickX(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetRightThumbstick(userId).x;
	}

	float Input::GetRightStickY(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetRightThumbstick(userId).y;
	}
	
	float2 Input::GetRightStick(UserID userId)
	{
		EnsureDefaultUser(userId);
		return devices->GetRightThumbstick(userId);
	}

	int2 Input::GetMousePos()
	{
		return devices->GetMousePosition(-1);
	}

	int2 Input::GetWindowSize()
	{
		return devices->GetWindowSize();
	}

	bool Input::IsMouseButtonDown(uint8_t button)
	{

		return devices->GetMouseButtons(-1) & button;
	}

	bool Input::IsMouseButtonPressed(uint8_t button)
	{
		return !(devices->GetPrevMouseButtons(-1) & button) && (devices->GetMouseButtons(-1) & button);
	}

	bool Input::IsMouseButtonReleased(uint8_t button)
	{
		return  (devices->GetPrevMouseButtons(-1) & button) && !(devices->GetMouseButtons(-1) & button);
	}



	bool Input::IsMouseVisible()
	{
		return false;
	}

	void Input::EnsureDefaultUser(UserID& userId)
	{
		if (userId == -1) userId = Platform::GetPlatform().GetUserManager()->GetDefaultUser().id;
	}
}