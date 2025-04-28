#include <RetrogressiveSystems/RetrogressiveSystems.h>


#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256
#define CANVAS_WIDTH SCREEN_WIDTH
#define CANVAS_HEIGHT SCREEN_HEIGHT
#define PATTERN_WIDTH 16
#define PATTERN_HEIGHT 8
#define TILEMAP_WIDTH (CANVAS_WIDTH / PATTERN_WIDTH)
#define TILEMAP_HEIGHT (CANVAS_HEIGHT / PATTERN_HEIGHT)


static RGSTile tilemap[TILEMAP_WIDTH * TILEMAP_HEIGHT];


void RGSConfigure(RGSGameInfo* inout_game, RGSAudioInfo* inout_audio, RGSGraphicsInfo* inout_graphics) {
	inout_graphics->window_title = "Graphics Example";
	inout_graphics->screen_width = SCREEN_WIDTH;
	inout_graphics->screen_height = SCREEN_HEIGHT;
	inout_graphics->canvas_width = CANVAS_WIDTH;
	inout_graphics->canvas_height = CANVAS_HEIGHT;
	inout_graphics->pattern_width = PATTERN_WIDTH;
	inout_graphics->pattern_height = PATTERN_HEIGHT;
	inout_graphics->pattern_count = 3U;
	inout_graphics->bits_per_pixel = 4U;
};

void RGSBegin() {
	RGSSetColour(0U, RGS_COLOUR_MAKE(0, 0, 255));
	RGSSetColour(1U, RGS_COLOUR_MAKE(255, 0, 0));
	RGSSetColour(2U, RGS_COLOUR_MAKE(0, 255, 0));
	RGSSetColour(3U, RGS_COLOUR_MAKE(255, 255, 255));
	const uint8_t tile_pattern[] = {
		0b00110011, 0b00110011, 0b00110011, 0b11111111,
		0b11001100, 0b11001100, 0b11001100, 0b11111111,
		0b00110011, 0b00110011, 0b00110011, 0b00111111,
		0b11001100, 0b11001100, 0b11001100, 0b11001111,
		0b00110011, 0b00110011, 0b00110011, 0b00110011,
		0b11001100, 0b11001100, 0b11001100, 0b11001100,
		0b00110011, 0b00110011, 0b00110011, 0b00110011,
		0b11001100, 0b11001100, 0b11001100, 0b11001100,
	};
	const uint8_t sprite_pattern[] = {
		0b00000000, 0b11111111,
		0b00000000, 0b11111111,
		0b00000000, 0b11111111,
		0b00000000, 0b11111111,
		0b11111111, 0b11111111,
		0b11111111, 0b11111111,
		0b11111111, 0b11111111,
		0b11111111, 0b11111111,
	};
	RGSWritePattern(1U, 2U, tile_pattern);
	RGSWritePattern(2U, 1U, sprite_pattern);
	for (int tile_y = 0; tile_y < TILEMAP_HEIGHT; tile_y++) {
		for (int tile_x = 0; tile_x < TILEMAP_WIDTH; tile_x++) {
			const size_t tile_index = tile_x + (tile_y * TILEMAP_WIDTH);
			tilemap[tile_index].pattern = (tile_x + tile_y) % 2;
			tilemap[tile_index].palette = 0U;
			tilemap[tile_index].hflip = ((tile_x + 1) / 2) % 2;
			tilemap[tile_index].vflip = ((tile_y + 1) / 2) % 2;
		};
	};
};

void RGSEnd() { };

void RGSUpdate(RGSTime in_elapsed) { };

void RGSRender() {
	const RGSPalette1 palette_tl = { 0, 1 };
	const RGSPalette1 palette_tr = { 0, 1 };
	const RGSPalette1 palette_bl = { 0, 1 };
	const RGSPalette1 palette_br = { 0, 1 };
	RGSDrawTiles(0, 0, tilemap, RGS_NULL, false, false, false);
	RGSDrawSprite(16, 16, 2U, palette_tl, false, false, false, false);
	RGSDrawSprite(16 + PATTERN_WIDTH, 16, 2U, palette_tr, true, false, false, false);
	RGSDrawSprite(16, 16 + PATTERN_HEIGHT, 2U, palette_bl, false, true, false, false);
	RGSDrawSprite(16 + PATTERN_WIDTH, 16 + PATTERN_HEIGHT, 2U, palette_br, true, true, false, false);
};

