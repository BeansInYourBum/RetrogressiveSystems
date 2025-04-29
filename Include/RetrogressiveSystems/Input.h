#pragma once
#ifndef RETROGRESSIVESYSTEMS_INPUT_H
#define RETROGRESSIVESYSTEMS_INPUT_H


#include <RetrogressiveSystems/Types.h>


/// @brief Input State Type
typedef enum RGSInputState {
	RGS_INPUT_STATE_UP =															0b00,									// Up Input State
	RGS_INPUT_STATE_RELEASED =														0b10,									// Released Input State
	RGS_INPUT_STATE_DOWN =															0b01,									// Down Input State
	RGS_INPUT_STATE_PRESSED =														0b11,									// Pressed Input State
} RGSInputState;

/// @brief Keyboard Key Type
typedef enum RGSKeyboardKey {
	RGS_KEYBOARD_KEY_ESCAPE,																								// Escape Keyboard Key
	RGS_KEYBOARD_KEY_RETURN,																								// Return Keyboard Key
	RGS_KEYBOARD_KEY_MENU,																									// Menu Keyboard Key
	RGS_KEYBOARD_KEY_LCONTROL,																								// Left Control Keyboard Key
	RGS_KEYBOARD_KEY_RCONTROL,																								// Right Control Keyboard Key
	RGS_KEYBOARD_KEY_LSHIFT,																								// Left Shift Keyboard Key
	RGS_KEYBOARD_KEY_RSHIFT,																								// Right Shift Keyboard Key
	RGS_KEYBOARD_KEY_LALT,																									// Left Alt Keyboard Key
	RGS_KEYBOARD_KEY_RALT,																									// Right Alt Keyboard Key
	RGS_KEYBOARD_KEY_BSPACE,																								// Backspace Keyboard Key
	RGS_KEYBOARD_KEY_SPACE,																									// Space Keyboard Key
	RGS_KEYBOARD_KEY_TAB,																									// Tab Keyboard Key
	RGS_KEYBOARD_KEY_UP,																									// Up Keyboard Key
	RGS_KEYBOARD_KEY_DOWN,																									// Down Keyboard Key
	RGS_KEYBOARD_KEY_LEFT,																									// Left Keyboard Key
	RGS_KEYBOARD_KEY_RIGHT,																									// Right Keyboard Key
	RGS_KEYBOARD_KEY_PERIOD,																								// Period Keyboard Key
	RGS_KEYBOARD_KEY_COMMA,																									// Comma Keyboard Key
	RGS_KEYBOARD_KEY_MINUS,																									// Minus Keyboard Key
	RGS_KEYBOARD_KEY_PLUS,																									// Plus Keyboard Key
	RGS_KEYBOARD_KEY_F1 =															'1' - 0xF,								// F1 Keyboard Key
	RGS_KEYBOARD_KEY_F2,																									// F2 Keyboard Key
	RGS_KEYBOARD_KEY_F3,																									// F3 Keyboard Key
	RGS_KEYBOARD_KEY_F4,																									// F4 Keyboard Key
	RGS_KEYBOARD_KEY_F5,																									// F5 Keyboard Key
	RGS_KEYBOARD_KEY_F6,																									// F6 Keyboard Key
	RGS_KEYBOARD_KEY_F7,																									// F7 Keyboard Key
	RGS_KEYBOARD_KEY_F8,																									// F8 Keyboard Key
	RGS_KEYBOARD_KEY_F9,																									// F9 Keyboard Key
	RGS_KEYBOARD_KEY_F10,																									// F10 Keyboard Key
	RGS_KEYBOARD_KEY_F11,																									// F11 Keyboard Key
	RGS_KEYBOARD_KEY_F12,																									// F12 Keyboard Key
	RGS_KEYBOARD_KEY_0 = 															'0',									// 0 Keyboard Key
	RGS_KEYBOARD_KEY_1,		 																								// 1 Keyboard Key
	RGS_KEYBOARD_KEY_2,		 																								// 2 Keyboard Key
	RGS_KEYBOARD_KEY_3,		 																								// 3 Keyboard Key
	RGS_KEYBOARD_KEY_4,		 																								// 4 Keyboard Key
	RGS_KEYBOARD_KEY_5,		 																								// 5 Keyboard Key
	RGS_KEYBOARD_KEY_6,		 																								// 6 Keyboard Key
	RGS_KEYBOARD_KEY_7,		 																								// 7 Keyboard Key
	RGS_KEYBOARD_KEY_8,		 																								// 8 Keyboard Key
	RGS_KEYBOARD_KEY_9,		 																								// 9 Keyboard Key
	RGS_KEYBOARD_KEY_A =															'A',									// A Keyboard Key
	RGS_KEYBOARD_KEY_B,																										// B Keyboard Key
	RGS_KEYBOARD_KEY_C,																										// C Keyboard Key
	RGS_KEYBOARD_KEY_D,																										// D Keyboard Key
	RGS_KEYBOARD_KEY_E,																										// E Keyboard Key
	RGS_KEYBOARD_KEY_F,																										// F Keyboard Key
	RGS_KEYBOARD_KEY_G,																										// G Keyboard Key
	RGS_KEYBOARD_KEY_H,																										// H Keyboard Key
	RGS_KEYBOARD_KEY_I,																										// I Keyboard Key
	RGS_KEYBOARD_KEY_J,																										// J Keyboard Key
	RGS_KEYBOARD_KEY_K,																										// K Keyboard Key
	RGS_KEYBOARD_KEY_L,																										// L Keyboard Key
	RGS_KEYBOARD_KEY_M,																										// M Keyboard Key
	RGS_KEYBOARD_KEY_N,																										// N Keyboard Key
	RGS_KEYBOARD_KEY_O,																										// O Keyboard Key
	RGS_KEYBOARD_KEY_P,																										// P Keyboard Key
	RGS_KEYBOARD_KEY_Q,																										// Q Keyboard Key
	RGS_KEYBOARD_KEY_R,																										// R Keyboard Key
	RGS_KEYBOARD_KEY_S,																										// S Keyboard Key
	RGS_KEYBOARD_KEY_T,																										// T Keyboard Key
	RGS_KEYBOARD_KEY_U,																										// U Keyboard Key
	RGS_KEYBOARD_KEY_V,																										// V Keyboard Key
	RGS_KEYBOARD_KEY_W,																										// W Keyboard Key
	RGS_KEYBOARD_KEY_X,																										// X Keyboard Key
	RGS_KEYBOARD_KEY_Y,																										// Y Keyboard Key
	RGS_KEYBOARD_KEY_Z,																										// Z Keyboard Key
} RGSKeyboardKey;

/// @brief Mouse Button Type
typedef enum RGSMouseButton {
	RGS_MOUSE_BUTTON_LEFT,																									// Left Mouse Button
	RGS_MOUSE_BUTTON_MIDDLE,																								// Middle Mouse Button
	RGS_MOUSE_BUTTON_RIGHT,																									// Right Mouse Button
} RGSMouseButton;


/// @brief Checks to see if the specified state is up or released
/// @param _STATE
/// @return State is up or released?
#define RGS_INPUT_UP(_STATE) (!((_STATE) & RGS_INPUT_STATE_DOWN))

/// @brief Checks to see if the specified state is released
/// @param _STATE
/// @return State is released?
#define RGS_INPUT_RELEASED(_STATE) ((_STATE) == RGS_INPUT_STATE_RELEASED)

/// @brief Checks to see if the specified state is down or pressed
/// @param _STATE
/// @return State is down or pressed?
#define RGS_INPUT_DOWN(_STATE) ((_STATE) & RGS_INPUT_STATE_DOWN)

/// @brief Checks to see if the specified state is pressed
/// @param _STATE
/// @return State is pressed?
#define RGS_INPUT_PRESSED(_STATE) (((_STATE) & RGS_INPUT_STATE_PRESSED) == RGS_INPUT_STATE_PRESSED)


/// @brief Acquires the specified keyboard key state
/// @param in_key 
/// @return Input state
RGS_EXTERN RGSInputState RGSKeyboardState(RGSKeyboardKey in_key);

/// @brief Checks to see if the specified keyboard key is up
/// @param _KEY
/// @return Key is up?
#define RGSKeyboardUp(_KEY) (!(RGSKeyboardState(_KEY) & RGS_INPUT_STATE_DOWN))

/// @brief Checks to see if the specified keyboard key was released
/// @param _KEY
/// @return Key was released?
#define RGSKeyboardReleased(_KEY) (RGSKeyboardState(_KEY) == RGS_INPUT_STATE_RELEASED)

/// @brief Checks to see if the specified keyboard key is down
/// @param _KEY
/// @return Key is down?
#define RGSKeyboardDown(_KEY) (RGSKeyboardState(_KEY) & RGS_INPUT_STATE_DOWN)

/// @brief Checks to see if the specified keyboard key was pressed
/// @param _KEY
/// @return Key was pressed?
#define RGSKeyboardPressed(_KEY) (RGSKeyboardState(_KEY) == RGS_INPUT_STATE_PRESSED)

/// @brief Acquires the specified mouse button state
/// @param in_button 
/// @return Input state
RGS_EXTERN RGSInputState RGSMouseState(RGSMouseButton in_button);

/// @brief Checks to see if the specified mouse button is up
/// @param _BUTTON
/// @return Button is up?
#define RGSMouseUp(_BUTTON) (!(RGSMouseState(_BUTTON) & RGS_INPUT_STATE_DOWN))

/// @brief Checks to see if the specified mouse button was released
/// @param _BUTTON
/// @return Button was released?
#define RGSMouseReleased(_BUTTON) (RGSMouseState(_BUTTON) == RGS_INPUT_STATE_RELEASED)

/// @brief Checks to see if the specified mouse button is down
/// @param _BUTTON
/// @return Button is down?
#define RGSMouseDown(_BUTTON) (RGSMouseState(_BUTTON) & RGS_INPUT_STATE_DOWN)

/// @brief Checks to see if the specified mouse button was pressed
/// @param _BUTTON
/// @return Button was pressed?
#define RGSMousePressed(_BUTTON) (RGSMouseState(_BUTTON) == RGS_INPUT_STATE_PRESSED)

/// @brief Acquires the mouse x position on screen
/// @return Mouse x
RGS_EXTERN int RGSMouseX();

/// @brief Acquires the mouse y position on screen
/// @return Mouse y
RGS_EXTERN int RGSMouseY();


#endif

