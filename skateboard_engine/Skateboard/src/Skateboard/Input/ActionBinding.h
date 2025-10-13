
#pragma once
#include <sktbdpch.h>
#include "Skateboard/Input/DeviceManager.h"

#define SKTBD_LOG_COMPONENT "ACTION BINDING"
#include "Skateboard/Log.h"

namespace Skateboard
{
#define BIND_INPUT_ACTION(x) std::bind(&x, this)

#define BIND_INPUT_RAW(x) std::bind(&x, this, std::placeholders::_1, std::placeholders::_2)

	constexpr uint32_t MAX_JOYSTICK_COUNT = 2;
	constexpr uint32_t MAX_INPUT_ACTION_TYPE = 3u;

	// Function pointer to a action, this could be a character jump action for instance.
	typedef void(*ActionCallBack)(void);

	// Function pointer to raw input, expects the value of the
	// the 'x' and 'y' axis to be passed as arguments.
	typedef void(*JoyStickCallback)(float, float);

	struct JoystickInput
	{
		uint8_t code;
		float x;
		float y;
	};

	class EngineInputCapture
	{
	public:
		/// <summary>
		///	When raw input is captured, the type of input action and the key/button interacted with is
		///	stored onto the input queue for processing.
		///	</summary>
		void RecordInput(uint8_t code, ActionType_ type) /*override*/
		{
			SKTBD_ASSERT(!(code & 0x00000002 || code & 0x00000004), "Joysticks cannot be recorded via action mappings!");
			KeyCodeQueue.push({ type, code });
		}

		// <summary>
		// Record raw input, such as playstation joysticks.
		// </summary>
		void RecordInput(uint8_t code, float x, float y)
		{
			SKTBD_ASSERT(code & 0x00000002 || code & 0x00000004, "Only joysticks are supported for raw input mapping!");
			RawInputQueue.push({ code, x, y });
		}

		// <summary>
		//  Fetch the current input queue.
		// </summary>
		std::queue<std::pair<ActionType_, uint8_t>>& GetFrameInputQueue()
		{
			return KeyCodeQueue;
		}

		// <summary>
		//	Fetch the current raw input queue.
		// </summary>
		std::queue<JoystickInput>& GetJoystickInputQueue() 
		{
			return RawInputQueue;
		}

	private:
		/// <summary>
		///	Input is recorded into a queue when an input event is received.
		///	The queue is processed on the updating input.
		///	</summary>
		std::queue<std::pair<ActionType_, uint8_t>> KeyCodeQueue;

		/// <summary>
		///	Input is recorded into a queue when an input event is received.
		///	The queue is processed on the updating input.
		///	</summary>
		std::queue<JoystickInput> RawInputQueue;

	};

	/// <summary>
	///	An input config object which allows the user to bind functions to buttons and keys.
	///	If a button or key has been recently pressed, then the associated action is evaluated
	///	on the next update.
	///	</summary>
	class KeyboardInputBinder
	{
		friend class Input;
	public:

		/// <summary>
		///	Processes the input queue and invokes actions.
		///	</summary>
		///	<param1>Time since last frame.</param1>
		void HandleInput(float dt, EngineInputCapture& gInputCapture);


		/// <summary>
		///	UnBinds an action assigned to the desired key/button code.
		///	</summary>
		///	<param1>The desired input type. i.e IsKeyPressed?</param1>
		///	<param2>The key/button code.</param2>
		void UnBindAction(ActionType_ type, uint8_t code);


		/// <summary>
		///	Bind an action to a key or button.
		///	</summary>
		///	<param1>The desired input type. i.e IsKeyPressed?</param1>
		///	<param2>The key/button code.</param2>
		///	<param3>The callback function to be invoked.</param3>
		void BindAction(ActionType_ type, uint8_t code, std::function<void()> function);


	private:

		/// <summary>
		///	An ordered map for bind keys to actions.
		///	</summary>
		std::array<std::map<uint8_t, std::function<void()>>, MAX_INPUT_ACTION_TYPE> ActionMap;

	};


	class GamePadInputBinder
	{
	public:

		// <summary>
		// Processes the game pad for joystick events.
		// </summary>
		void HandleInput(float dt, EngineInputCapture& gInputCapture);


		/// <summary>
		///	Binds raw input to a function. This includes playstation joysticks.
		///	</summary>
		///	<param1>The desired joystick i.e Left or Right.</param1>
		///	<param2>The callback function to be invoked.</param2>
		void BindRawInput(Side_ joystick, std::function<void(float, float)> function);

		/// <summary>
		///	UnBinds an action assigned to the desired key/button code.
		///	</summary>
		///	<param1>The desired input type. i.e IsKeyPressed?</param1>
		///	<param2>The key/button code.</param2>
		void UnBindAction(Side_ type, uint8_t code);


	private:
		/// <summary>
		///	An array for binding functions to joystick input.
		///	</summary>
		std::array<std::function<void(float, float)>, MAX_JOYSTICK_COUNT> RawMapping;
	};
}