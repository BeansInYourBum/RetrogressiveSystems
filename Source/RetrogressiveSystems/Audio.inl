#pragma once
#ifndef RETROGRESSIVESYSTEMS_AUDIO_INL
#define RETROGRESSIVESYSTEMS_AUDIO_INL


#include <RetrogressiveSystems/Audio.h>
#include <RetrogressiveSystems/Game.h>

#include "./Threads.inl"


/// @brief Prepares the audio system before beginning the game
/// @param in_game 
/// @param in_audio 
/// @return Successfully prepared?
extern bool RGSPrepareAudio(const RGSGameInfo* in_game, const RGSAudioInfo* in_audio);

/// @brief Releases the audio system after ending the game
extern void RGSReleaseAudio();


/// @brief Tells the audio thread to start working
extern void RGSStartAudio();

/// @brief Tells the audio thread to stop working
extern void RGSStopAudio();

/// @brief Renders the game's audio
extern void RGSRenderAudio();


// @brief Locks the audio system's resources
extern void RGSLockAudio();

/// @brief Unlocks the audio system's resources
extern void RGSUnlockAudio();


/// @brief Checks to see if the audio system uses another thread
/// @return Thread is used?
extern bool RGSAudioThreaded();

/// @brief Checks to see if the audio system's thread is running
/// @return Thread is running?
extern bool RGSAudioRunning();


#endif

