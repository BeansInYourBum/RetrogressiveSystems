#include <RetrogressiveSystems/RetrogressiveSystems.h>


#include <math.h>


#define PI 3.1415F


static RGSTime timer_a = 0ULL;
static RGSTime timer_b = 0ULL;
static uint8_t instrument = 0U;
static int position = 0;
static int direction = 1;


static float WaveInstrument(uint8_t in_note, float in_offset) {
	switch (in_note) {
	default:
	case 0U: return sinf(2.0F * PI * 44.0F * in_offset);
	case 1U: return sinf(2.0F * PI * 88.0F * in_offset);
	case 2U: return sinf(2.0F * PI * 132.0F * in_offset);
	};
};


void RGSConfigure(RGSGameInfo* inout_game, RGSAudioInfo* inout_audio, RGSGraphicsInfo* inout_graphics) {
	inout_audio->instrument_count = 1U;
	inout_audio->bits_per_sample = 16U;
};

void RGSBegin() {
	RGSSetInstrument(0U, &WaveInstrument);
};

void RGSEnd() { };

void RGSUpdate(RGSTime in_elapsed) {
	timer_a += in_elapsed;
	if (timer_a >= RGS_ONE_SECOND) {
		timer_a -= RGS_ONE_SECOND;
		RGSPlayNote(0U, 0U, 1.0F, 0.5F, 0.0F);
	};
	timer_b += in_elapsed;
	if (timer_b >= (RGS_ONE_SECOND / 2)) {
		timer_b -= RGS_ONE_SECOND / 2;
		RGSPlayNote(0U, instrument, 2.0F, 0.5F, (float)(position));
		instrument = (instrument + 1U) % 3U;
		position += direction;
		if ((direction > 0 && position > 0) ||
			(direction < 0 && position < 0)) {
			direction = -direction;
		};
	};
};

void RGSRender() { };

