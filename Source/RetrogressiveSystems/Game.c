#include "./Game.inl"


#include "./Output.inl"
#include "./Input.inl"
#include "./Audio.inl"
#include "./Graphics.inl"


/// Internal Game Variables

static RGSTime g_updated = 0ULL;
static bool g_active = true;
static bool g_paused = false;
static bool g_running = true;


/// Internal Game Functions

#if RGS_BUILD == RGS_BUILD_DEBUG
int main()
#else
#if RGS_OS == RGS_OS_WINDOWS
int WINAPI WinMain(HINSTANCE in_instance, HINSTANCE in_previous, LPSTR in_command, int in_show)
#endif
#endif
{
	if (!RGSPrepareOutput()) return 0;
	RGSGameInfo game_info = {
		RGS_NULL,
		0U, 0U, 0U
	};
	RGSAudioInfo audio_info = {
		0U,
		16U, true,
		true
	};
	RGSGraphicsInfo graphics_info = {
#if RGS_DEVICE == RGS_DEVICE_COMPUTER
#if RGS_OS == RGS_OS_WINDOWS
		-1,
#endif
		RGS_NULL,
#endif
		1024, 1024,
		1024, 1024,
		32U, 32U, 256U,
		8U, 60U,
		true
	};
	RGSConfigure(&game_info, &audio_info, &graphics_info);
	if (!g_running || !RGSSafe()) {
		RGSReleaseOutput();
		return 0;
	};
	if (RGSPrepareInput()) {
		if (RGSPrepareAudio(&game_info, &audio_info)) {
			if (!graphics_info.window_title || !*graphics_info.window_title) graphics_info.window_title = game_info.name;
			if (RGSPrepareGraphics(&game_info, &graphics_info)) {
				RGSLockAudio();
				RGSLockGraphics();
				RGSBegin();
				if (g_running) RGSStartGraphics();
				RGSUnlockGraphics();
				if (g_running) RGSStartAudio();
				RGSUnlockAudio();
				if (g_running && RGSSafe() && RGSAudioRunning() && RGSGraphicsRunning()) {
					g_updated = RGSTimeNow();
					do {
						RGSTime current_time = RGSTimeNow();
						const RGSTime elapsed_time = current_time - g_updated;
						if (elapsed_time >= (RGS_ONE_SECOND / 1000ULL)) {
							if (g_active) {
								if (g_paused) {
									g_paused = false;
									current_time = RGSTimeNow();
								}
								else {
									RGSLockAudio();
									RGSLockGraphics();
									RGSUpdateInput();
									RGSUpdate(elapsed_time);
									RGSUnlockGraphics();
									RGSUnlockAudio();
								};
							};
							g_updated = current_time;
						};
						if (!RGSAudioThreaded()) RGSRenderAudio();
						if (!RGSGraphicsThreaded()) RGSRenderGraphics();
					} while (g_running && RGSSafe() && RGSAudioRunning() && RGSGraphicsRunning());
					RGSLockAudio();
					RGSLockGraphics();
					RGSEnd();
					RGSUnlockGraphics();
					RGSUnlockAudio();
				};
				RGSReleaseGraphics();
			};
			RGSReleaseAudio();
		};
		RGSReleaseInput();
	};
	RGSReleaseOutput();
	return 0;
};


void RGSPauseGame() {
	g_active = false;
	g_paused = true;
};

void RGSUnpauseGame() { g_active = true; };


/// Exposed Game Functions

void RGSQuit() { g_running = false; };

