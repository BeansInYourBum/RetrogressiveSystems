#pragma once
#ifndef RETROGRESSIVESYSTEMS_AUDIO_H
#define RETROGRESSIVESYSTEMS_AUDIO_H


#include <RetrogressiveSystems/Types.h>


typedef float(*RGSInstrument)(uint8_t, float);																				// Instrument Function Type


/// @brief Audio Info Container
typedef struct RGSAudioInfo {
	uint32_t instrument_count;																								// Total Instrument Count (Must be lower than or equal to 256)
	uint32_t bits_per_sample;																								// Target Bits Per Sample (Must be 8 or 16)
	bool stereo;																											// Stereo Mode Enabled?
	bool threaded;																											// Try To Use A Separate Thread?
} RGSAudioInfo;


/// @brief Acquires the specified instrument function
/// @param in_index 
/// @return Instrument function
RGS_EXTERN RGSInstrument RGSGetInstrument(uint8_t in_index);

/// @brief Updates the specified instruction function
/// @param in_index 
/// @param in_instrument 
RGS_EXTERN void RGSSetInstrument(uint8_t in_index, RGSInstrument in_instrument);


/// @brief Plays a note to be mixed with the other audio
/// @param in_instrument 
/// @param in_note 
/// @param in_speed
/// @param in_volume
/// @param in_position
RGS_EXTERN void RGSPlayNote(uint8_t in_instrument, uint8_t in_note, float in_speed, float in_volume, float in_position);


#endif

