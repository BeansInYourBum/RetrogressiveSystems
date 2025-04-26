#pragma once
#ifndef RETROGRESSIVESYSTEMS_GRAPHICS_INL
#define RETROGRESSIVESYSTEMS_GRAPHICS_INL


#include <RetrogressiveSystems/Graphics.h>
#include <RetrogressiveSystems/Game.h>

#include "./Threads.inl"


/// @brief Prepares the graphics system before beginning the game
/// @param in_game 
/// @param in_graphics 
/// @return Successfully prepared?
extern bool RGSPrepareGraphics(const RGSGameInfo* in_game, const RGSGraphicsInfo* in_graphics);

/// @brief Releases the graphics system after ending the game
extern void RGSReleaseGraphics();


/// @brief Tells the graphics thread to start working
extern void RGSStartGraphics();

/// @brief Renders the games content and puts it on screen
extern void RGSRenderGraphics();


/// @brief Locks the graphics system's resources
extern void RGSLockGraphics();

/// @brief Unlocks the graphics system's resources
extern void RGSUnlockGraphics();


/// @brief Checks to see if the graphics system uses another thread
/// @return Thread is used?
extern bool RGSGraphicsThreaded();

/// @brief Checks to see if the graphics system's thread is running
/// @return Thread is running?
extern bool RGSGraphicsRunning();


#endif

