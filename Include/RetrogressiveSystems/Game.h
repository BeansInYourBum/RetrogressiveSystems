#pragma once
#ifndef RETROGRESSIVESYSTEMS_GAME_H
#define RETROGRESSIVESYSTEMS_GAME_H


#include <RetrogressiveSystems/Audio.h>
#include <RetrogressiveSystems/Graphics.h>


/// @brief Game Info Container
typedef struct RGSGameInfo {
	const char* name;																										// Name Pointer
	uint32_t version_major;																									// Version Major Value
	uint32_t version_minor;																									// Version Minor Value
	uint32_t version_patch;																									// Version Patch Value
} RGSGameInfo;


/// @brief Called when configuring the game
/// @param out_game 
/// @param out_audio 
/// @param out_graphics 
extern void RGSConfigure(RGSGameInfo* out_game, RGSAudioInfo* out_audio, RGSGraphicsInfo* out_graphics);

/// @brief Called when beginning the game
extern void RGSBegin();

/// @brief Called when ending the game
extern void RGSEnd();

/// @brief Called when updating the game
/// @param in_elapsed 
extern void RGSUpdate(RGSTime in_elapsed);

/// @brief Called when rendering the game
extern void RGSRender();


/// @brief Quits the game and stops it from running
RGS_EXTERN void RGSQuit();


#endif

