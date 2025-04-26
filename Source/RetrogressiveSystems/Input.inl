#pragma once
#ifndef RETROGRESSIVESYSTEMS_INPUT_INL
#define RETROGRESSIVESYSTEMS_INPUT_INL


#include <RetrogressiveSystems/Input.h>


#include "./Platform.inl"


/// @brief Prepares the input system before beginning the game
/// @return Successfully prepared?
extern bool RGSPrepareInput();

/// @brief Releases the input system after ending the game
extern void RGSReleaseInput();


/// @brief Updates the input system before updating the game
extern void RGSUpdateInput();


#if RGS_OS == RGS_OS_WINDOWS
/// @brief Activates handling of the internal input states
extern void RGSActivateInput();

/// @brief Deactivates handling of the internal input states
extern void RGSDeactivateInput();


/// @brief Modifies the internal mouse position
/// @param in_x 
/// @param in_y 
extern void RGSModifyMousePosition(int in_x, int in_y);
#endif


#endif

