#include "sktbdpch.h"
#include "Skateboard/Input/ActionBinding.h"

//#include "Skateboard/Input.h"

namespace Skateboard
{
	///// KEYBOARD INPUT BINDER /////

	void KeyboardInputBinder::HandleInput(float dt, EngineInputCapture& gInputCapture)
	{
		std::queue<std::pair<ActionType_, uint8_t>>& inputQueue = gInputCapture.GetFrameInputQueue();
		while (!inputQueue.empty())
		{
			std::pair<ActionType_, uint8_t> element = inputQueue.back();
			const auto inputType = static_cast<uint32_t>(element.first);

			if (ActionMap[inputType].count(element.second) > 0)
			{
				std::map<uint8_t, std::function<void()>>& map = ActionMap[inputType];
				auto function = map.at(element.second);
				function();
			}
			inputQueue.pop();
		}
	}

	void KeyboardInputBinder::BindAction(ActionType_ type, uint8_t code, std::function<void()> function)
	{
		const auto inputType = static_cast<int32_t>(type);

		if (ActionMap[inputType].count(code) > 0)
		{
			SKTBD_MSG_TRACE("Input mapping assigned to code {0} is being overriden.", ActionMap[inputType].count(code));
		}

		// Assign the function to the input code.
		ActionMap[inputType].emplace(code, function);
	}

	void KeyboardInputBinder::UnBindAction(ActionType_ type, uint8_t code)
	{
		const auto inputType = static_cast<int32_t>(type);

		if (ActionMap[inputType].count(code) > 0)
		{
			SKTBD_MSG_TRACE("Input mapping assigned to code {0} is being overidden.", ActionMap[inputType].count(code));
		}

		// Assign the function to the input code.
		ActionMap[inputType].at(code) = nullptr;
		size_t size = ActionMap[inputType].erase(code);
	}


	///// RAW INPUT BINDER /////

	void GamePadInputBinder::HandleInput(float dt, EngineInputCapture& gInputCapture)
	{
		std::queue<JoystickInput>& inputQueue = gInputCapture.GetJoystickInputQueue();

		while (!inputQueue.empty())
		{
			JoystickInput& element = inputQueue.back();
			uint32_t joystick = (element.code == 0x00000002) ? 0u : 1u;


			auto function = RawMapping[joystick];
			function(element.x, element.y);
			inputQueue.pop();
		}
	}


	void GamePadInputBinder::BindRawInput(Side_ joystick, std::function<void(float, float)> function)
	{
		const auto input = static_cast<int32_t>(joystick);
		SKTBD_LOG_ASSERT(input > -1 && input < 2, "Invalid joystick, left and right joysticks are currently supported.");

		if (RawMapping[input] != nullptr)
		{
			SKTBD_MSG_TRACE("Action mapped to joystick {0} is being overriden.", input);
		}

		RawMapping[input] = function;
	}

	void GamePadInputBinder::UnBindAction(Side_ joystick, uint8_t code)
	{
		const auto input = static_cast<int32_t>(joystick);
		SKTBD_LOG_ASSERT(input > -1 && input < 2, "Invalid joystick, left and right joysticks are currently supported.");

		if (RawMapping[input] != nullptr)
		{
			SKTBD_MSG_TRACE("Action mapped to joystick {0} is being removed.", input);
		}

		RawMapping[input] = nullptr;
	}

}