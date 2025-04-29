#include "./Input.inl"


#include <RetrogressiveSystems/Output.h>


#if RGS_OS == RGS_OS_WINDOWS
#define RGS_INPUT_KEYBOARD_UK_STANDARD 0x00000809U
#endif


/// Internal Input Variables

#if RGS_OS == RGS_OS_WINDOWS
static bool g_focused = false;
static size_t g_keyboard = 0U;
#endif
static RGSInputState g_mkstates[256U] = { 0 };
static int g_mx = 0;
static int g_my = 0;


/// Internal Input Functions

bool RGSPrepareInput() {
#if RGS_DEVICE == RGS_DEVICE_COMPUTER
	RGSInputState* current_state = g_mkstates;
	const RGSInputState* const last_state = g_mkstates + (sizeof(g_mkstates) / sizeof(*g_mkstates));
	do *current_state = RGS_INPUT_STATE_UP;
	while (++current_state < last_state);
#if RGS_OS == RGS_OS_WINDOWS
	char layout_name[KL_NAMELENGTH];
	if (GetKeyboardLayoutNameA(layout_name) != 0) {
		char layout_character = layout_name[0U];
		uint32_t layout_identity = (uint32_t)((layout_character >= '0' && layout_character <= '9') ? (layout_character - '0') : (10 + (layout_character - 'A'))) << 26U;
		layout_character = layout_name[1U];
		layout_identity |= (uint32_t)((layout_character >= '0' && layout_character <= '9') ? (layout_character - '0') : (10 + (layout_character - 'A'))) << 24U;
		layout_character = layout_name[2U];
		layout_identity |= (uint32_t)((layout_character >= '0' && layout_character <= '9') ? (layout_character - '0') : (10 + (layout_character - 'A'))) << 20U;
		layout_character = layout_name[3U];
		layout_identity |= (uint32_t)((layout_character >= '0' && layout_character <= '9') ? (layout_character - '0') : (10 + (layout_character - 'A'))) << 16U;
		layout_character = layout_name[4U];
		layout_identity |= (uint32_t)((layout_character >= '0' && layout_character <= '9') ? (layout_character - '0') : (10 + (layout_character - 'A'))) << 12U;
		layout_character = layout_name[5U];
		layout_identity |= (uint32_t)((layout_character >= '0' && layout_character <= '9') ? (layout_character - '0') : (10 + (layout_character - 'A'))) << 8U;
		layout_character = layout_name[6U];
		layout_identity |= (uint32_t)((layout_character >= '0' && layout_character <= '9') ? (layout_character - '0') : (10 + (layout_character - 'A'))) << 4U;
		layout_character = layout_name[7U];
		layout_identity |= (uint32_t)((layout_character >= '0' && layout_character <= '9') ? (layout_character - '0') : (10 + (layout_character - 'A')));
		switch (layout_identity) {
		case RGS_INPUT_KEYBOARD_UK_STANDARD: g_keyboard = VK_OEM_8; break;
		default: RGSReportError("Input", "Failed to identify keyboard layout", false); break;
		};
	}
	else RGSReportError("Input", "Failed to acquire keyboard layout", false);
#endif
#endif
	return true;
};

void RGSReleaseInput() { };


void RGSUpdateInput() {
#if RGS_DEVICE == RGS_DEVICE_COMPUTER
	for (int state_index = 0; state_index < (sizeof(g_mkstates) / sizeof(*g_mkstates)); state_index++) {
#if RGS_OS == RGS_OS_WINDOWS
		const SHORT input_state = GetAsyncKeyState(state_index);
		if (g_focused && input_state & 0x8000) {
			if (!(g_mkstates[state_index] & RGS_INPUT_STATE_DOWN)) g_mkstates[state_index] = RGS_INPUT_STATE_PRESSED;
			else g_mkstates[state_index] = RGS_INPUT_STATE_DOWN;
		}
		else {
			if ((g_mkstates[state_index]) & RGS_INPUT_STATE_DOWN) g_mkstates[state_index] = RGS_INPUT_STATE_RELEASED;
			else g_mkstates[state_index] = RGS_INPUT_STATE_UP;
		};
#endif
	};
#endif
};


#if RGS_OS == RGS_OS_WINDOWS
void RGSActivateInput() { g_focused = true; };

void RGSDeactivateInput() { g_focused = false; };


void RGSModifyMousePosition(int in_x, int in_y) {
#if RGS_OS == RGS_OS_WINDOWS
	if (!g_focused) return;
#endif
	g_mx = in_x;
	g_my = in_y;
};
#endif


/// Exposed Input Functions

RGSInputState RGSKeyboardState(RGSKeyboardKey in_key) {
#if RGS_OS == RGS_OS_WINDOWS
	if ((in_key >= '0' && in_key <= '9') || (in_key >= 'a' && in_key <= 'z') || (in_key >= 'A' && in_key <= 'Z')) return g_mkstates[in_key];
#endif
	switch (in_key) {
#if RGS_OS == RGS_OS_WINDOWS
	case RGS_KEYBOARD_KEY_ESCAPE: return g_mkstates[VK_ESCAPE];
	case RGS_KEYBOARD_KEY_RETURN: return g_mkstates[VK_RETURN];
	case RGS_KEYBOARD_KEY_MENU: return g_mkstates[g_keyboard];
	case RGS_KEYBOARD_KEY_LCONTROL: return g_mkstates[VK_LCONTROL];
	case RGS_KEYBOARD_KEY_RCONTROL: return g_mkstates[VK_RCONTROL];
	case RGS_KEYBOARD_KEY_LSHIFT: return g_mkstates[VK_LSHIFT];
	case RGS_KEYBOARD_KEY_RSHIFT: return g_mkstates[VK_RSHIFT];
	case RGS_KEYBOARD_KEY_LALT: return g_mkstates[VK_LMENU];
	case RGS_KEYBOARD_KEY_RALT: return g_mkstates[VK_RMENU];
	case RGS_KEYBOARD_KEY_BSPACE: return g_mkstates[VK_BACK];
	case RGS_KEYBOARD_KEY_SPACE: return g_mkstates[VK_SPACE];
	case RGS_KEYBOARD_KEY_TAB: return g_mkstates[VK_TAB];
	case RGS_KEYBOARD_KEY_UP: return g_mkstates[VK_UP];
	case RGS_KEYBOARD_KEY_DOWN: return g_mkstates[VK_DOWN];
	case RGS_KEYBOARD_KEY_LEFT: return g_mkstates[VK_LEFT];
	case RGS_KEYBOARD_KEY_RIGHT: return g_mkstates[VK_RIGHT];
	case RGS_KEYBOARD_KEY_PERIOD: return g_mkstates[VK_OEM_PERIOD];
	case RGS_KEYBOARD_KEY_COMMA: return g_mkstates[VK_OEM_COMMA];
	case RGS_KEYBOARD_KEY_MINUS: return g_mkstates[VK_OEM_MINUS];
	case RGS_KEYBOARD_KEY_PLUS: return g_mkstates[VK_OEM_PLUS];
	case RGS_KEYBOARD_KEY_F1: return g_mkstates[VK_F1];
	case RGS_KEYBOARD_KEY_F2: return g_mkstates[VK_F2];
	case RGS_KEYBOARD_KEY_F3: return g_mkstates[VK_F3];
	case RGS_KEYBOARD_KEY_F4: return g_mkstates[VK_F4];
	case RGS_KEYBOARD_KEY_F5: return g_mkstates[VK_F5];
	case RGS_KEYBOARD_KEY_F6: return g_mkstates[VK_F6];
	case RGS_KEYBOARD_KEY_F7: return g_mkstates[VK_F7];
	case RGS_KEYBOARD_KEY_F8: return g_mkstates[VK_F8];
	case RGS_KEYBOARD_KEY_F9: return g_mkstates[VK_F9];
	case RGS_KEYBOARD_KEY_F10: return g_mkstates[VK_F10];
	case RGS_KEYBOARD_KEY_F11: return g_mkstates[VK_F11];
	case RGS_KEYBOARD_KEY_F12: return g_mkstates[VK_F12];
#endif
	default: return 0;
	};
};


RGSInputState RGSMouseState(RGSMouseButton in_button) {
	switch (in_button) {
#if RGS_OS == RGS_OS_WINDOWS
	case RGS_MOUSE_BUTTON_LEFT: return g_mkstates[VK_LBUTTON];
	case RGS_MOUSE_BUTTON_MIDDLE: return g_mkstates[VK_MBUTTON];
	case RGS_MOUSE_BUTTON_RIGHT: return g_mkstates[VK_RBUTTON];
#endif
	default: return 0;
	};
};

int RGSMouseX() { return g_mx; };

int RGSMouseY() { return g_my; };

