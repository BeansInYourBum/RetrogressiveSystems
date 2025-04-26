#pragma once
#ifndef RETROGRESSIVESYSTEMS_OUTPUT_INL
#define RETROGRESSIVESYSTEMS_OUTPUT_INL


#include <RetrogressiveSystems/Output.h>


/// @brief Prepares the output system before configuring the game
/// @return Successfully prepared?
extern bool RGSPrepareOutput();

/// @brief Releases the output system after ending the game
extern void RGSReleaseOutput();


/// @brief Checks to see if a fatal error was reported to the output system
/// @return Safe to continue?
extern bool RGSSafe();


#endif

