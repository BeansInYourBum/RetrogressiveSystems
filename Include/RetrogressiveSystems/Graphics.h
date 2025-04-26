#pragma once
#ifndef RETROGRESSIVESYSTEMS_GRAPHICS_H
#define RETROGRESSIVESYSTEMS_GRAPHICS_H


#include <RetrogressiveSystems/Types.h>


#define RGS_PATTERN_WIDTH															8										// Pattern Width In Pixels
#define RGS_PATTERN_HEIGHT															8										// Pattern Height In Pixels
#define RGS_PATTERN_COUNT															256										// Total Pattern Count


typedef uint32_t RGSColour;																									// Packed Colour Type
typedef const uint8_t* RGSPalette;																							// Colour Palette Parameter Type
typedef uint8_t RGSPalette1[1U << 1U];																						// 1-Bit Colour Palette Type
typedef uint8_t RGSPalette2[1U << 2U];																						// 2-Bit Colour Palette Type
typedef uint8_t RGSPalette4[1U << 4U];																						// 4-Bit Colour Palette Type
typedef uint8_t RGSPalette8[1U << 8U];																						// 8-Bit Colour Palette Type
typedef uint8_t RGSPattern;																									// Pattern Index Type


/// @brief Makes a colour from the supplied red, green & blue
/// @param _RED
/// @param _GREEN
/// @param _BLUE
/// @return Packed colour
#define RGS_COLOUR_MAKE(_RED, _GREEN, _BLUE) ((((RGSColour)(_RED) & 0xFFU) << 16U) | (((RGSColour)(_GREEN) & 0xFFU) << 8U) | ((RGSColour)(_BLUE) & 0xFFU))

/// @brief Acquires the given colour's red value
/// @param _COLOUR
/// @return Red value
#define RGS_COLOUR_RED(_COLOUR) (((RGSColour)(_COLOUR) >> 16U) & 0xFFU)

/// @brief Acquires the given colour's green value
/// @param _COLOUR
/// @return Green value
#define RGS_COLOUR_GREEN(_COLOUR) (((RGSColour)(_COLOUR) >> 8U) & 0xFFU)

/// @brief Acquires the given colour's blue value
/// @param _COLOUR
/// @return Blue value
#define RGS_COLOUR_BLUE(_COLOUR) ((RGSColour)(_COLOUR) & 0xFFU)


/// @brief Tile Container
typedef struct RGSTile {
	RGSPattern pattern;																										// Pattern Index
	uint8_t palette;																										// Palette Index
	bool hflip;																												// Horizontal Flip
	bool vflip;																												// Vertical Flip
} RGSTile;


/// @brief Graphics Info Container
typedef struct RGSGraphicsInfo {
#if RGS_DEVICE == RGS_DEVICE_COMPUTER
#if RGS_OS == RGS_OS_WINDOWS
	int window_icon;																										// Window Icon Resource (Optional)
#endif
	const char* window_title;																								// Window Title Pointer (Optional)
#endif
	uint32_t screen_width;																									// Screen Width In Pixels (Must be a multiple of 8 and between 1 and canvas width)
	uint32_t screen_height;																									// Screen Height In Pixels (Must be a multiple of 8 and between 1 and canvas height)
	uint32_t canvas_width;																									// Canvas Width In Pixels (Must be a multiple of 8 and between 1 and 1024)
	uint32_t canvas_height;																									// Canvas Height In Pixels (Must be a multiple of 8 and between 1 and 1024)
	uint32_t bits_per_pixel;																								// Target Bits Per Pixel (Must be 1, 2, 4 or 8)
	uint32_t frame_rate;																									// Target Frame Rate (Must be between 1 and 60)
	bool threaded;																											// Try To Use A Separate Thread?
} RGSGraphicsInfo;


/// @brief Acquires the specified colour in the palette
/// @param in_index 
/// @return Packed colour
RGS_EXTERN RGSColour RGSGetColour(uint8_t in_index);

/// @brief Updates the specified colour in the palette
/// @param in_index 
/// @param in_packed 
RGS_EXTERN void RGSSetColour(uint8_t in_index, RGSColour in_packed);

/// @brief Acquires all of the colours in the palette
/// @param out_data 
RGS_EXTERN void RGSReadColours(RGSColour* out_data);

/// @brief Updates all of the colours in the palette
/// @param in_data 
RGS_EXTERN void RGSWriteColours(const RGSColour* in_data);


/// @brief Acquires the specified pattern on the texture (Index must be lower than pattern count and bits must be 1, 2, 4 or 8)
/// @param in_index 
/// @param in_bits 
/// @param out_data 
RGS_EXTERN void RGSReadPattern(RGSPattern in_index, uint32_t in_bits, uint8_t* out_data);

/// @brief Updates the specified pattern on the texture (Index must be lower than pattern count and bits must be 1, 2, 4 or 8)
/// @param in_index 
/// @param in_bits 
/// @param in_data 
RGS_EXTERN void RGSWritePattern(RGSPattern in_index, uint32_t in_bits, const uint8_t* in_data);


/// @brief Acquires the specified pixel colour on the virtual screen
/// @param in_x 
/// @param in_y 
/// @return Colour index
RGS_EXTERN uint8_t RGSGetPixel(int in_x, int in_y);

/// @brief Updates the specified pixel colour on the virtual screen
/// @param in_x 
/// @param in_y 
/// @param in_index 
RGS_EXTERN void RGSSetPixel(int in_x, int in_y, uint8_t in_index);


/// @brief Draws a sprite to the virtual screen (Pattern index must be lower than pattern count)
/// @param in_x 
/// @param in_y 
/// @param in_pattern 
/// @param in_palette 
/// @param in_hflip 
/// @param in_vflip 
/// @param in_hwrap 
/// @param in_vwrap 
RGS_EXTERN void RGSDrawSprite(int in_x, int in_y, RGSPattern in_pattern, RGSPalette in_palette, bool in_hflip, bool in_vflip, bool in_hwrap, bool in_vwrap);

/// @brief Draws the given tiles to the virtual screen (Tile array must fill canvas)
/// @param in_x 
/// @param in_y 
/// @param in_tiles 
/// @param in_palettes 
/// @param in_hwrap 
/// @param in_vwrap 
/// @param in_transparent 
RGS_EXTERN void RGSDrawTiles(int in_x, int in_y, const RGSTile* in_tiles, const RGSPalette* in_palettes, bool in_hwrap, bool in_vwrap, bool in_transparent);


#endif

