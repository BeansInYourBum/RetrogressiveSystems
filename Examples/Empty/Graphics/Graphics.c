#include <RetrogressiveSystems/RetrogressiveSystems.h>


#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256
#define CANVAS_WIDTH SCREEN_WIDTH
#define CANVAS_HEIGHT SCREEN_HEIGHT
#define TILEMAP_WIDTH (CANVAS_WIDTH / RGS_PATTERN_WIDTH)
#define TILEMAP_HEIGHT (CANVAS_HEIGHT / RGS_PATTERN_HEIGHT)


static RGSTile tilemap[TILEMAP_WIDTH * TILEMAP_HEIGHT];


void RGSConfigure(RGSGameInfo* in_game, RGSAudioInfo* in_audio, RGSGraphicsInfo* in_graphics) {
	in_graphics->window_title = "Graphics Example";
	in_graphics->screen_width = SCREEN_WIDTH;
	in_graphics->screen_height = SCREEN_HEIGHT;
	in_graphics->canvas_width = CANVAS_WIDTH;
	in_graphics->canvas_height = CANVAS_HEIGHT;
	in_graphics->bits_per_pixel = 2U;
};

void RGSBegin() {
	RGSSetColour(0U, RGS_COLOUR_MAKE(0, 0, 255));
	RGSSetColour(1U, RGS_COLOUR_MAKE(255, 0, 0));
	RGSSetColour(2U, RGS_COLOUR_MAKE(0, 255, 0));
	RGSSetColour(3U, RGS_COLOUR_MAKE(255, 255, 255));
	const uint8_t tile_pattern[] = {
		0b00110011, 0b00110011,
		0b11001100, 0b11001100,
		0b00110011, 0b00110011,
		0b11001100, 0b11001100,
		0b00110011, 0b00110011,
		0b11001100, 0b11001100,
		0b00110011, 0b00110011,
		0b11001100, 0b11001100,
	};
	const uint8_t sprite_pattern[] = {
		0b00001111,
		0b00001111,
		0b00001111,
		0b00001111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
	};
	RGSWritePattern(1U, 2U, tile_pattern);
	RGSWritePattern(2U, 1U, sprite_pattern);
	for (int tile_y = 0; tile_y < TILEMAP_HEIGHT; tile_y++) {
		for (int tile_x = 0; tile_x < TILEMAP_WIDTH; tile_x++) {
			const size_t tile_index = tile_x + (tile_y * TILEMAP_WIDTH);
			tilemap[tile_index].pattern = 1U;
			tilemap[tile_index].palette = 0U;
			tilemap[tile_index].hflip = false;
			tilemap[tile_index].vflip = false;
		};
	};
};

void RGSEnd() { };

void RGSUpdate(RGSTime in_elapsed) { };

void RGSRender() {
	RGSDrawTiles(0, 0, tilemap, RGS_NULL, false, false, false);
	RGSDrawSprite(16, 16, 2U, RGS_NULL, false, false, false, false);
	RGSDrawSprite(16 + RGS_PATTERN_WIDTH, 16, 2U, RGS_NULL, true, false, false, false);
	RGSDrawSprite(16, 16 + RGS_PATTERN_HEIGHT, 2U, RGS_NULL, false, true, false, false);
	RGSDrawSprite(16 + RGS_PATTERN_WIDTH, 16 + RGS_PATTERN_HEIGHT, 2U, RGS_NULL, true, true, false, false);
};

