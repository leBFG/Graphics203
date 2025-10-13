#pragma once
#include <sktbdpch.h>
#include "Skateboard/Mathematics.h"
#include "Skateboard/Events/AppEvents.h"
#include "TriggerEffectData.h"

typedef int32_t UserID;

namespace Skateboard
{
	// Basic flag to distinguish what type of input should the callback be invoked.
	enum class ActionType_ : uint8_t
	{
		ActionType_Pressed = 0,
		ActionType_Down,
		ActionType_Released,
		ActionType_Joystick
	};

	enum DeviceType
	{
		SKTB_Keyboard = 0,
		SKTB_GamePad = 1,
		SKTB_Mouse = 2,
		SKTB_Custom = 3,
		SKTB_DEVICE_TYPE_COUNT,
	};
	
	template <class T>
	struct Device
	{
		UserID userID;
		int32_t handle;
		DeviceType DeviceType;
		T latestData;
		T oldData;
	};

	class SKTBD_API DeviceConnectedEvent : public Event
	{
	public:
		DeviceConnectedEvent(const UserID& user, const DeviceType& type, const bool connected) :
			user(user),
			type(type),
			connected(connected)
		{
		}
		EVENT_CLASS_TYPE(EventType::DeviceConnected)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

			const UserID& GetUser() { return user; }
		const DeviceType& GetDeviceType() { return type; }
		const bool& IsConnectionEvent() { return connected; }

	private:
		const UserID& user;
		const DeviceType& type;
		const bool connected;
	};

	class DeviceManager
	{
		friend class Platform;
		//friend class Input;

	protected:
		std::function<void(Event&)> PlatformEventCallback;
		void RegisterEventCallback(std::function<void(Event&)> callback) { PlatformEventCallback = callback; }

		virtual void Update() = 0;
		virtual void OnEvent(Event& e) = 0;
		virtual void MessageInputEvents() = 0;

	public:
		DISABLE_COPY_AND_MOVE(DeviceManager);
		virtual ~DeviceManager() {};
		DeviceManager() = default;
		virtual int32_t Init() = 0;

		virtual int32_t GetDevicesConnected(UserID user, DeviceType type) = 0;

		//immediate mode inputs
		virtual std::vector<Skateboard::Keys>& GetKeys(UserID user) = 0; // returns pointer to keyboard keys
		virtual std::vector<Skateboard::Keys>& GetPrevKeys(UserID user) = 0; // returns pointer to keyboard keys
		virtual uint8_t*    GetModifierKeys(UserID user) = 0; // returns pointer to modifier keys

		virtual uint32_t&   GetButtons(UserID user) = 0; // returns button mappings
		virtual uint32_t&   GetPreviousButtons(UserID user) = 0; // returns button mappings


		virtual int2   GetMousePosition(UserID user) = 0; // returns Mouse Positions
		virtual uint8_t   GetMouseButtons(UserID user) = 0; // returns Mouse Positions
		virtual uint8_t   GetPrevMouseButtons(UserID user) = 0; // returns Mouse Positions
		virtual void SetMousePosition(int2 pos) = 0;
		virtual void ShowMouse(bool b) = 0;
		virtual int2 GetWindowSize() = 0;

		virtual float		GetTrigger(UserID user, Side_ axis ) = 0;
		virtual float2		GetLeftThumbstick(UserID user) = 0; //returns snorm float from thumbstick X 0 = neutral
		virtual float2		GetRightThumbstick(UserID user) = 0; //returns snorm float from thumbstick X 0 = neutral
		virtual glm::quat   GetGyro(UserID user) = 0; // gyro data 
		virtual glm::float3 GetAcceleration(UserID user) = 0; //accelerometer data
		virtual std::vector<Touch>	GetTouches(UserID user) = 0; // touch data, depending on number of touches supported returns number of touhes per device

		virtual void SetHapticResponse(uint32_t user, const SKTBDeviceHapticSettings& settings) = 0;
		virtual void SetSimpleVibration(UserID user, float largeMotorSpeed, float smallMotorSpeed, uint32_t vibrationTimeInMS) = 0;


		virtual void* GetRawData(UserID user, DeviceType type, size_t& dataSize) = 0;
	};
}