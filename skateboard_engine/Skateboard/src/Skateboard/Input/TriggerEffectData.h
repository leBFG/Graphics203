#pragma once
#include "glm/ext.hpp"
namespace Skateboard 
{
#define SKTBD_PAD_TRIGGER_EFFECT_TRIGGER_NUM				2

	/* Definition of control point num */
#define SKTBD_PAD_TRIGGER_EFFECT_CONTROL_POINT_NUM		10


// Very basic 'syntax sugar' for distinguishing playstation joysticks.
	enum class Side_ : uint8_t
	{
		Left = 0,
		Right = 1,
	};

	typedef enum SKTBDPadTriggerEffectMode {
		SKTBD_PAD_TRIGGER_EFFECT_MODE_OFF,
		SKTBD_PAD_TRIGGER_EFFECT_MODE_FEEDBACK,
		SKTBD_PAD_TRIGGER_EFFECT_MODE_WEAPON,
		SKTBD_PAD_TRIGGER_EFFECT_MODE_VIBRATION,
		SKTBD_PAD_TRIGGER_EFFECT_MODE_MULTIPLE_POSITION_FEEDBACK,
		SKTBD_PAD_TRIGGER_EFFECT_MODE_SLOPE_FEEDBACK,
		SKTBD_PAD_TRIGGER_EFFECT_MODE_MULTIPLE_POSITION_VIBRATION,
	} SKTBDPadTriggerEffectMode;

	/**
	 *E
	 *  @brief parameter for setting the trigger effect to off mode.
	 *         Off Mode: Stop trigger effect.
	 **/
	typedef struct SKTBDPadTriggerEffectOffParam {

	} SKTBDPadTriggerEffectOffParam;

	/**
	 *E
	 *  @brief parameter for setting the trigger effect to Feedback mode.
	 *         Feedback Mode: The motor arm pushes back trigger.
	 *                        Trigger obtains stiffness at specified position.
	 **/
	typedef struct SKTBDPadTriggerEffectFeedbackParam {
		uint8_t position;	/*E position where the strength of target trigger start changing(0~9). */
		uint8_t strength;	/*E strength that the motor arm pushes back target trigger(0~8 (0: Same as Off mode)). */
	} SKTBDPadTriggerEffectFeedbackParam;

	/**
	 *E
	 *  @brief parameter for setting the trigger effect to Weapon mode.
	 *         Weapon Mode: Emulate weapon like gun trigger.
	 **/
	typedef struct SKTBDPadTriggerEffectWeaponParam {
		uint8_t startPosition;	/*E position where the stiffness of triger start changing(2~7). */
		uint8_t endPosition;	/*E position where the stiffness of trigger finish changing(startPosition+1~8). */
		uint8_t strength;		/*E strength of gun trigger(0~8 (0: Same as Off mode)). */
	} SKTBDPadTriggerEffectWeaponParam;

	/**
	 *E
	 *  @brief parameter for setting the trigger effect to Vibration mode.
	 *         Vibration Mode: Vibrates motor arm around specified position.
	 **/
	typedef struct SKTBDPadTriggerEffectVibrationParam {
		uint8_t position;	/*E position where the motor arm start vibrating(0~9). */
		uint8_t amplitude;	/*E vibration amplitude(0~8 (0: Same as Off mode)). */
		uint8_t frequency;	/*E vibration frequency(0~255[Hz] (0: Same as Off mode)). */
	} SKTBDPadTriggerEffectVibrationParam;

	/**
	 *E
	 *  @brief parameter for setting the trigger effect to SKTBDPadTriggerEffectMultiplePositionFeedbackParam mode.
	 *         Multi Position Feedback Mode: The motor arm pushes back trigger.
	 *                                       Trigger obtains specified stiffness at each control point.
	 **/
	typedef struct SKTBDPadTriggerEffectMultiplePositionFeedbackParam {
		uint8_t strength[SKTBD_PAD_TRIGGER_EFFECT_CONTROL_POINT_NUM];	/*E strength that the motor arm pushes back target trigger at position(0~8 (0: Same as Off mode)).
																	 *  strength[0] means strength of motor arm at position0.
																	 *  strength[1] means strength of motor arm at position1.
																	 *  ...
																	 * */

	} SKTBDPadTriggerEffectMultiplePositionFeedbackParam;

	/**
	 *E
	 *  @brief parameter for setting the trigger effect to Feedback3 mode.
	 *         Slope Feedback Mode: The motor arm pushes back trigger between two spedified control points.
	 *                              Stiffness of the trigger is changing depending on the set place.
	 **/
	typedef struct SKTBDPadTriggerEffectSlopeFeedbackParam {

		uint8_t startPosition;	/*E position where the strength of target trigger start changing(0~endPosition). */
		uint8_t endPosition; 	/*E position where the strength of target trigger finish changing(startPosition+1~9). */
		uint8_t startStrength;	/*E strength when trigger's position is startPosition(1~8) */
		uint8_t endStrength;	/*E strength when trigger's position is endPosition(1~8) */

	} SKTBDPadTriggerEffectSlopeFeedbackParam;

	/**
	 *E
	 *  @brief parameter for setting the trigger effect to Vibration2 mode.
	 *         Multi Position Vibration Mode: Vibrates motor arm around specified control point.
	 *                                        Trigger vibrates specified amplitude at each control point.
	 **/
	typedef struct SKTBDPadTriggerEffectMultiplePositionVibrationParam {
		uint8_t frequency;												/*E vibration frequency(0~255 (0: Same as Off mode)) */
		uint8_t amplitude[SKTBD_PAD_TRIGGER_EFFECT_CONTROL_POINT_NUM];	/*E vibration amplitude at position(0~8 (0: Same as Off mode)).
																		 *  amplitude[0] means amplitude of vibration at position0.
																		 *  amplitude[1] means amplitude of vibration at position1.
																		 *  ...
																		 * */

	} SKTBDPadTriggerEffectMultiplePositionVibrationParam;

	/**
	 *E
	 *  @brief parameter for setting the trigger effect mode.
	 **/
	typedef union SKTBDPadTriggerEffectCommandData {
		SKTBDPadTriggerEffectOffParam							offParam;
		SKTBDPadTriggerEffectFeedbackParam						feedbackParam;
		SKTBDPadTriggerEffectWeaponParam						weaponParam;
		SKTBDPadTriggerEffectVibrationParam						vibrationParam;
		SKTBDPadTriggerEffectMultiplePositionFeedbackParam		multiplePositionFeedbackParam;
		SKTBDPadTriggerEffectSlopeFeedbackParam					slopeFeedbackParam;
		SKTBDPadTriggerEffectMultiplePositionVibrationParam		multiplePositionVibrationParam;
	} SKTBDPadTriggerEffectCommandData;


	struct SKTBDeviceHapticSettings // Haptic structure
	{
		Side_ side;
		SKTBDPadTriggerEffectMode mode;
		SKTBDPadTriggerEffectCommandData commandData;
	};

	struct Touch
	{
		float2 xy;
		uint8_t id;
	};

	typedef enum GamePadButton_
	{
		Pad_Button_L3 = 0x00000002,
		Pad_Button_R3 = 0x00000004,
		Pad_Button_Options = 0x00000008,
		Pad_Dpad_Up = 0x00000010,
		Pad_Dpad_Right = 0x00000020,
		Pad_Dpad_Down = 0x00000040,
		Pad_Dpad_Left = 0x00000080,
		Pad_Button_L2 = 0x00000100,
		Pad_Button_R2 = 0x00000200,
		Pad_Button_L1 = 0x00000400,
		Pad_Button_R1 = 0x00000800,
		Pad_Button_Top = 0x00001000,
		Pad_Button_Right = 0x00002000,
		Pad_Button_Bottom = 0x00004000,
		Pad_Button_Left = 0x00008000,

		Pad_Button_TOUCH_PAD = 0x00100000,
		Pad_Button_INTERCEPTED = 0x80000000,
	} GamePadButton_;


	enum Keys {
        sc_escape = 0x01,
        sc_1 = 0x02,
        sc_2 = 0x03,
        sc_3 = 0x04,
        sc_4 = 0x05,
        sc_5 = 0x06,
        sc_6 = 0x07,
        sc_7 = 0x08,
        sc_8 = 0x09,
        sc_9 = 0x0A,
        sc_0 = 0x0B,
        sc_minus = 0x0C,
        sc_equals = 0x0D,
        sc_backspace = 0x0E,
        sc_tab = 0x0F,
        sc_q = 0x10,
        sc_w = 0x11,
        sc_e = 0x12,
        sc_r = 0x13,
        sc_t = 0x14,
        sc_y = 0x15,
        sc_u = 0x16,
        sc_i = 0x17,
        sc_o = 0x18,
        sc_p = 0x19,
        sc_bracketLeft = 0x1A,
        sc_bracketRight = 0x1B,
        sc_enter = 0x1C,
        sc_controlLeft = 0x1D,
        sc_a = 0x1E,
        sc_s = 0x1F,
        sc_d = 0x20,
        sc_f = 0x21,
        sc_g = 0x22,
        sc_h = 0x23,
        sc_j = 0x24,
        sc_k = 0x25,
        sc_l = 0x26,
        sc_semicolon = 0x27,
        sc_apostrophe = 0x28,
        sc_grave = 0x29,
        sc_shiftLeft = 0x2A,
        sc_backslash = 0x2B,
        sc_z = 0x2C,
        sc_x = 0x2D,
        sc_c = 0x2E,
        sc_v = 0x2F,
        sc_b = 0x30,
        sc_n = 0x31,
        sc_m = 0x32,
        sc_comma = 0x33,
        sc_preiod = 0x34,
        sc_slash = 0x35,
        sc_shiftRight = 0x36,
        sc_numpad_multiply = 0x37,
        sc_altLeft = 0x38,
        sc_space = 0x39,
        sc_capsLock = 0x3A,
        sc_f1 = 0x3B,
        sc_f2 = 0x3C,
        sc_f3 = 0x3D,
        sc_f4 = 0x3E,
        sc_f5 = 0x3F,
        sc_f6 = 0x40,
        sc_f7 = 0x41,
        sc_f8 = 0x42,
        sc_f9 = 0x43,
        sc_f10 = 0x44,
        sc_numLock = 0x45,
        sc_scrollLock = 0x46,
        sc_numpad_7 = 0x47,
        sc_numpad_8 = 0x48,
        sc_numpad_9 = 0x49,
        sc_numpad_minus = 0x4A,
        sc_numpad_4 = 0x4B,
        sc_numpad_5 = 0x4C,
        sc_numpad_6 = 0x4D,
        sc_numpad_plus = 0x4E,
        sc_numpad_1 = 0x4F,
        sc_numpad_2 = 0x50,
        sc_numpad_3 = 0x51,
        sc_numpad_0 = 0x52,
        sc_numpad_period = 0x53,
        sc_alt_printScreen = 0x54, /* Alt + print screen. MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54. */
        sc_bracketAngle = 0x56, /* Key between the left shift and Z. */
        sc_f11 = 0x57,
        sc_f12 = 0x58,
        sc_oem_1 = 0x5a, /* VK_OEM_WSCTRL */
        sc_oem_2 = 0x5b, /* VK_OEM_FINISH */
        sc_oem_3 = 0x5c, /* VK_OEM_JUMP */
        sc_eraseEOF = 0x5d,
        sc_oem_4 = 0x5e, /* VK_OEM_BACKTAB */
        sc_oem_5 = 0x5f, /* VK_OEM_AUTO */
        sc_zoom = 0x62,
        sc_help = 0x63,
        sc_f13 = 0x64,
        sc_f14 = 0x65,
        sc_f15 = 0x66,
        sc_f16 = 0x67,
        sc_f17 = 0x68,
        sc_f18 = 0x69,
        sc_f19 = 0x6a,
        sc_f20 = 0x6b,
        sc_f21 = 0x6c,
        sc_f22 = 0x6d,
        sc_f23 = 0x6e,
        sc_oem_6 = 0x6f, /* VK_OEM_PA3 */
        sc_katakana = 0x70,
        sc_oem_7 = 0x71, /* VK_OEM_RESET */
        sc_f24 = 0x76,
        sc_sbcschar = 0x77,
        sc_convert = 0x79,
        sc_nonconvert = 0x7B, /* VK_OEM_PA1 */

        sc_media_previous = 0xE010,
        sc_media_next = 0xE019,
        sc_numpad_enter = 0xE01C,
        sc_controlRight = 0xE01D,
        sc_volume_mute = 0xE020,
        sc_launch_app2 = 0xE021,
        sc_media_play = 0xE022,
        sc_media_stop = 0xE024,
        sc_volume_down = 0xE02E,
        sc_volume_up = 0xE030,
        sc_browser_home = 0xE032,
        sc_numpad_divide = 0xE035,
        sc_printScreen = 0xE037,
        /*
        sc_printScreen:
        - make: 0xE02A 0xE037
        - break: 0xE0B7 0xE0AA
        - MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54;
        - There is no VK_KEYDOWN with VK_SNAPSHOT.
        */
        sc_altRight = 0xE038,
        sc_cancel = 0xE046, /* CTRL + Pause */
        sc_home = 0xE047,
        sc_arrowUp = 0xE048,
        sc_pageUp = 0xE049,
        sc_arrowLeft = 0xE04B,
        sc_arrowRight = 0xE04D,
        sc_end = 0xE04F,
        sc_arrowDown = 0xE050,
        sc_pageDown = 0xE051,
        sc_insert = 0xE052,
        sc_delete = 0xE053,
        sc_metaLeft = 0xE05B,
        sc_metaRight = 0xE05C,
        sc_application = 0xE05D,
        sc_power = 0xE05E,
        sc_sleep = 0xE05F,
        sc_wake = 0xE063,
        sc_browser_search = 0xE065,
        sc_browser_favorites = 0xE066,
        sc_browser_refresh = 0xE067,
        sc_browser_stop = 0xE068,
        sc_browser_forward = 0xE069,
        sc_browser_back = 0xE06A,
        sc_launch_app1 = 0xE06B,
        sc_launch_email = 0xE06C,
        sc_launch_media = 0xE06D,

        sc_pause = 0xE11D45,
		


	};

    enum MouseBtn {
        mb_None = 0x00000000,
        mb_LeftButton = 0x00000001,
        mb_RightButton = 0x00000002,
        mb_MiddleButton = 0x00000004,
        mb_Button4 = 0x00000008,
        mb_Button5 = 0x00000010,
        mb_WheelTiltLeft = 0x00000020,
        mb_WhelTiltRight = 0x00000040
    };








}