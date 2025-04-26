#include "./Graphics.inl"


#include <RetrogressiveSystems/Output.h>

#include "./Input.inl"
#include "./Game.inl"

#include <stdlib.h>


#if RGS_OS == RGS_OS_WINDOWS
#include <windowsx.h>
#define RGS_GRAPHICS_CLASS_NAME "RGSWindow"
#endif


/// Internal Graphics Types

typedef struct RGSGraphicsThreadParameters {
#if RGS_DEVICE == RGS_DEVICE_COMPUTER
#if RGS_OS == RGS_OS_WINDOWS
	int window_icon;
#endif
	const char* window_title;
#endif
} RGSGraphicsThreadParameters;


/// Internal Graphics Variables

static int g_swidth = 0;
static int g_sheight = 0;
static int g_cwidth = 0;
static int g_cheight = 0;

static uint8_t* g_patterns = RGS_NULL;
static volatile bool g_modifying = false;

static int g_bits = 0;
static int g_colours = 0;
static int g_length = 0;
static uint8_t* g_pixels = RGS_NULL;

static RGSLock g_lock = RGS_LOCK_INVALID;
static RGSThread g_thread = RGS_THREAD_INVALID;

static RGSTime g_rate = 0ULL;
static RGSTime g_rendered = 0ULL;
static volatile bool g_rendering = false;
static volatile bool g_running = true;
static volatile bool g_started = false;

static uint8_t(*g_get_pixel)(int, int) = RGS_NULL;
static void(*g_set_pixel)(int, int, uint8_t) = RGS_NULL;
static void(*g_draw_sprite)(int, int, RGSPattern, RGSPalette, bool, bool, bool, bool) = RGS_NULL;
static void(*g_draw_tiles)(int, int, const RGSTile*, const RGSPalette*, bool, bool, bool) = RGS_NULL;

#if RGS_OS == RGS_OS_WINDOWS
static LPBITMAPINFO g_bitmap = NULL;
static HINSTANCE g_instance = NULL;
static HWND g_window = NULL;
static volatile bool g_created = false;
#endif


/// Internal Graphics Functions

uint8_t RGSGetPixel1(int in_x, int in_y) {
	const uint8_t sample_data = g_pixels[(in_x >> 3) + (in_y * g_length)];
	return (sample_data >> (uint8_t)(7 - (in_x & 7))) & ((1U << 1U) - 1U);
};

uint8_t RGSGetPixel4(int in_x, int in_y) {
	const uint8_t sample_data = g_pixels[(in_x >> 1) + (in_y * g_length)];
	return ((in_x & 1) ? sample_data : (sample_data >> 4U)) & ((1U << 4U) - 1U);
};

uint8_t RGSGetPixel8(int in_x, int in_y) { return g_pixels[in_x + (in_y * g_length)]; };


void RGSSetPixel1(int in_x, int in_y, uint8_t in_index) {
	const size_t sample_index = (size_t)((in_x >> 3) + (in_y * g_length));
	const uint8_t sample_data = g_pixels[sample_index];
	g_pixels[sample_index] = (sample_data & (uint8_t)(~(1 << (7 - (in_x & 7))))) | ((in_index & (uint8_t)(g_colours - 1)) << (7 - (in_x & 7)));
};

void RGSSetPixel4(int in_x, int in_y, uint8_t in_index) {
	const size_t sample_index = (size_t)((in_x >> 1) + (in_y * g_length));
	const uint8_t sample_data = g_pixels[sample_index];
	if (in_x & 1) g_pixels[sample_index] = (sample_data & 0b11110000U) | (in_index & (uint8_t)(g_colours - 1));
	else g_pixels[sample_index] = (sample_data & 0b00001111U) | ((in_index & (uint8_t)(g_colours - 1)) << 4U);
};

void RGSSetPixel8(int in_x, int in_y, uint8_t in_index) { g_pixels[in_x + (in_y * g_length)] = in_index & (uint8_t)(g_colours - 1); };


void RGSDrawSprite1(int in_x, int in_y, RGSPattern in_pattern, RGSPalette in_palette, bool in_hflip, bool in_vflip, bool in_hwrap, bool in_vwrap) {
	if (in_hwrap) in_x = in_x >= 0 ? (in_x % g_cwidth) : (g_cwidth + (in_x % g_cwidth));
	if (in_vwrap) in_y = in_y >= 0 ? (in_y % g_cheight) : (g_cheight + (in_y % g_cheight));
	const uint8_t* pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 3) * RGS_PATTERN_HEIGHT) * (size_t)(in_pattern & (RGS_PATTERN_COUNT - 1)));
	for (int sample_y = 0; sample_y < RGS_PATTERN_HEIGHT; sample_y++) {
		int pixel_y = in_y + sample_y;
		if (in_vwrap) {
			if (pixel_y >= g_cheight) pixel_y %= g_cheight;
			if (pixel_y >= g_sheight) continue;
		}
		else {
			if (pixel_y < 0) continue;
			else if (pixel_y >= g_sheight) break;
		};
		for (int sample_x = 0; sample_x < RGS_PATTERN_WIDTH; sample_x++) {
			int pixel_x = in_x + sample_x;
			if (in_hwrap) {
				if (pixel_x >= g_cwidth) pixel_x %= g_cwidth;
				if (pixel_x >= g_swidth) continue;
			}
			else {
				if (pixel_x < 0) continue;
				else if (pixel_x >= g_swidth) break;
			};
			uint8_t sample_data = pattern_data[(in_vflip ? (RGS_PATTERN_HEIGHT - 1) - sample_y : sample_y) * (RGS_PATTERN_WIDTH >> 3)];
			sample_data = (sample_data >> (in_hflip ? sample_x : (7U - sample_x))) & ((1U << 1U) - 1U); 
			if (sample_data) {
				sample_data = in_palette ? in_palette[sample_data] : sample_data;
				const size_t pixel_index = (size_t)((pixel_x >> 3) + (pixel_y * g_length));
				g_pixels[pixel_index] = (g_pixels[pixel_index] & (uint8_t)(~(1 << (7 - (pixel_x & 7))))) | ((sample_data & (uint8_t)(g_colours - 1)) << (7 - (pixel_x & 7)));
			};
		};
	};
};

void RGSDrawSprite4(int in_x, int in_y, RGSPattern in_pattern, RGSPalette in_palette, bool in_hflip, bool in_vflip, bool in_hwrap, bool in_vwrap) {
	if (in_hwrap) in_x = in_x >= 0 ? (in_x % g_cwidth) : (g_cwidth + (in_x % g_cwidth));
	if (in_vwrap) in_y = in_y >= 0 ? (in_y % g_cheight) : (g_cheight + (in_y % g_cheight));
	const uint8_t* pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 1) * RGS_PATTERN_HEIGHT) * (size_t)(in_pattern & (RGS_PATTERN_COUNT - 1)));
	for (int sample_y = 0; sample_y < RGS_PATTERN_HEIGHT; sample_y++) {
		int pixel_y = in_y + sample_y;
		if (in_vwrap) {
			if (pixel_y >= g_cheight) pixel_y %= g_cheight;
			if (pixel_y >= g_sheight) continue;
		}
		else {
			if (pixel_y < 0) continue;
			else if (pixel_y >= g_sheight) break;
		};
		for (int sample_x = 0; sample_x < RGS_PATTERN_WIDTH; sample_x++) {
			int pixel_x = in_x + sample_x;
			if (in_hwrap) {
				if (pixel_x >= g_cwidth) pixel_x %= g_cwidth;
				if (pixel_x >= g_swidth) continue;
			}
			else {
				if (pixel_x < 0) continue;
				else if (pixel_x >= g_swidth) break;
			};
			uint8_t sample_data = pattern_data[((in_hflip ? (RGS_PATTERN_WIDTH - 1) - sample_x : sample_x) >> 1) + ((in_vflip ? (RGS_PATTERN_HEIGHT - 1) - sample_y : sample_y) * (RGS_PATTERN_WIDTH >> 1))];
			sample_data = (((sample_x + in_hflip) & 1) ? sample_data : (sample_data >> 4U)) & ((1U << 4U) - 1U);
			if (sample_data) {
				sample_data = in_palette ? in_palette[sample_data] : sample_data;
				const size_t pixel_index = (size_t)((pixel_x >> 1) + (pixel_y * g_length));
				if (pixel_x & 1) g_pixels[pixel_index] = (g_pixels[pixel_index] & 0b11110000U) | sample_data;
				else g_pixels[pixel_index] = (g_pixels[pixel_index] & 0b00001111U) | (sample_data << 4U);
			};
		};
	};
};

void RGSDrawSprite8(int in_x, int in_y, RGSPattern in_pattern, RGSPalette in_palette, bool in_hflip, bool in_vflip, bool in_hwrap, bool in_vwrap) {
	if (in_hwrap) in_x = in_x >= 0 ? (in_x % g_cwidth) : (g_cwidth + (in_x % g_cwidth));
	if (in_vwrap) in_y = in_y >= 0 ? (in_y % g_cheight) : (g_cheight + (in_y % g_cheight));
	const uint8_t* pattern_data = g_patterns + ((RGS_PATTERN_WIDTH * RGS_PATTERN_HEIGHT) * (size_t)(in_pattern & (RGS_PATTERN_COUNT - 1)));
	for (int sample_y = 0; sample_y < RGS_PATTERN_HEIGHT; sample_y++) {
		int pixel_y = in_y + sample_y;
		if (in_vwrap) {
			if (pixel_y >= g_cheight) pixel_y %= g_cheight;
			if (pixel_y >= g_sheight) continue;
		}
		else {
			if (pixel_y < 0) continue;
			else if (pixel_y >= g_sheight) break;
		};
		for (int sample_x = 0; sample_x < RGS_PATTERN_WIDTH; sample_x++) {
			int pixel_x = in_x + sample_x;
			if (in_hwrap) {
				if (pixel_x >= g_cwidth) pixel_x %= g_cwidth;
				if (pixel_x >= g_swidth) continue;
			}
			else {
				if (pixel_x < 0) continue;
				else if (pixel_x >= g_swidth) break;
			};
			const uint8_t sample_data = pattern_data[(in_hflip ? (RGS_PATTERN_WIDTH - 1) - sample_x : sample_x) + ((in_vflip ? (RGS_PATTERN_HEIGHT - 1) - sample_y : sample_y) * RGS_PATTERN_WIDTH)];
			if (sample_data) g_pixels[pixel_x + (pixel_y * g_length)] = in_palette ? in_palette[sample_data] : sample_data;
		};
	};
};


void RGSDrawTiles1(int in_x, int in_y, const RGSTile* in_tiles, const RGSPalette* in_palettes, bool in_hwrap, bool in_vwrap, bool in_transparent) {
	if (in_hwrap) in_x = in_x >= 0 ? (in_x % g_cwidth) : (g_cwidth + (in_x % g_cwidth));
	if (in_vwrap) in_y = in_y >= 0 ? (in_y % g_cheight) : (g_cheight + (in_y % g_cheight));
	const int layer_x_end = in_x + g_cwidth;
	const int layer_y_end = in_y + g_cheight;
	int layer_y = in_y;
	do {
		int pixel_y = layer_y;
		if (in_vwrap) {
			if (pixel_y >= g_cheight) pixel_y %= g_cheight;
			if (pixel_y >= g_sheight) continue;
		}
		else {
			if (pixel_y < 0) continue;
			else if (pixel_y >= g_sheight) break;
		};
		int tile_y = layer_y - in_y;
		const int sample_y = tile_y & (RGS_PATTERN_HEIGHT - 1);
		tile_y >>= 3;
		int layer_x = in_x;
		do {
			int pixel_x = layer_x;
			if (in_hwrap) {
				if (pixel_x >= g_cwidth) pixel_x %= g_cwidth;
				if (pixel_x >= g_swidth) continue;
			}
			else {
				if (pixel_x < 0) continue;
				else if (pixel_x >= g_swidth) break;
			};
			int tile_x = layer_x - in_x;
			const int sample_x = tile_x & (RGS_PATTERN_WIDTH - 1);
			tile_x >>= 3;
			const RGSTile tile_data = in_tiles[tile_x + (tile_y * ((g_cwidth + (RGS_PATTERN_WIDTH - 1)) >> 3))];
			const uint8_t* pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 3) * RGS_PATTERN_HEIGHT) * (size_t)(tile_data.pattern & (RGS_PATTERN_COUNT - 1)));
			uint8_t sample_data = pattern_data[(tile_data.vflip ? (RGS_PATTERN_HEIGHT - 1) - sample_y : sample_y) * (RGS_PATTERN_WIDTH >> 3)];
			sample_data = (sample_data >> (tile_data.hflip ? sample_x : (7U - sample_x))) & ((1U << 1U) - 1U); 
			if (!in_transparent || sample_data) {
				sample_data = in_palettes[tile_data.palette][sample_data];
				const size_t pixel_index = (size_t)((pixel_x >> 3) + (pixel_y * g_length));
				g_pixels[pixel_index] = (g_pixels[pixel_index] & (uint8_t)(~(1 << (7 - (pixel_x & 7))))) | ((sample_data & (uint8_t)(g_colours - 1)) << (7 - (pixel_x & 7)));
			};
		}
		while (++layer_x < layer_x_end);
	}
	while (++layer_y < layer_y_end);
};

void RGSDrawTiles4(int in_x, int in_y, const RGSTile* in_tiles, const RGSPalette* in_palettes, bool in_hwrap, bool in_vwrap, bool in_transparent) {
	if (in_hwrap) in_x = in_x >= 0 ? (in_x % g_cwidth) : (g_cwidth + (in_x % g_cwidth));
	if (in_vwrap) in_y = in_y >= 0 ? (in_y % g_cheight) : (g_cheight + (in_y % g_cheight));
	const int layer_x_end = in_x + g_cwidth;
	const int layer_y_end = in_y + g_cheight;
	int layer_y = in_y;
	do {
		int pixel_y = layer_y;
		if (in_vwrap) {
			if (pixel_y >= g_cheight) pixel_y %= g_cheight;
			if (pixel_y >= g_sheight) continue;
		}
		else {
			if (pixel_y < 0) continue;
			else if (pixel_y >= g_sheight) break;
		};
		int tile_y = layer_y - in_y;
		const int sample_y = tile_y & (RGS_PATTERN_HEIGHT - 1);
		tile_y >>= 3;
		int layer_x = in_x;
		do {
			int pixel_x = layer_x;
			if (in_hwrap) {
				if (pixel_x >= g_cwidth) pixel_x %= g_cwidth;
				if (pixel_x >= g_swidth) continue;
			}
			else {
				if (pixel_x < 0) continue;
				else if (pixel_x >= g_swidth) break;
			};
			int tile_x = layer_x - in_x;
			const int sample_x = tile_x & (RGS_PATTERN_WIDTH - 1);
			tile_x >>= 3;
			const RGSTile tile_data = in_tiles[tile_x + (tile_y * ((g_cwidth + (RGS_PATTERN_WIDTH - 1)) >> 3))];
			const uint8_t* pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 1) * RGS_PATTERN_HEIGHT) * (size_t)(tile_data.pattern & (RGS_PATTERN_COUNT - 1)));
			uint8_t sample_data = pattern_data[((tile_data.hflip ? (RGS_PATTERN_WIDTH - 1) - sample_x : sample_x) >> 1) + ((tile_data.vflip ? (RGS_PATTERN_HEIGHT - 1) - sample_y : sample_y) * (RGS_PATTERN_WIDTH >> 1))];
			sample_data = (((sample_x + tile_data.hflip) & 1) ? sample_data : (sample_data >> 4U)) & ((1U << 4U) - 1U);
			if (!in_transparent || sample_data) {
				sample_data = in_palettes[tile_data.palette][sample_data];
				const size_t pixel_index = (size_t)((pixel_x >> 1) + (pixel_y * g_length));
				if (pixel_x & 1) g_pixels[pixel_index] = (g_pixels[pixel_index] & 0b11110000U) | sample_data;
				else g_pixels[pixel_index] = (g_pixels[pixel_index] & 0b00001111U) | (sample_data << 4U);
			};
		}
		while (++layer_x < layer_x_end);
	}
	while (++layer_y < layer_y_end);
};

void RGSDrawTiles8(int in_x, int in_y, const RGSTile* in_tiles, const RGSPalette* in_palettes, bool in_hwrap, bool in_vwrap, bool in_transparent) {
	if (in_hwrap) in_x = in_x >= 0 ? (in_x % g_cwidth) : (g_cwidth + (in_x % g_cwidth));
	if (in_vwrap) in_y = in_y >= 0 ? (in_y % g_cheight) : (g_cheight + (in_y % g_cheight));
	const int layer_x_end = in_x + g_cwidth;
	const int layer_y_end = in_y + g_cheight;
	int layer_y = in_y;
	do {
		int pixel_y = layer_y;
		if (in_vwrap) {
			if (pixel_y >= g_cheight) pixel_y %= g_cheight;
			if (pixel_y >= g_sheight) continue;
		}
		else {
			if (pixel_y < 0) continue;
			else if (pixel_y >= g_sheight) break;
		};
		int tile_y = layer_y - in_y;
		const int sample_y = tile_y & (RGS_PATTERN_HEIGHT - 1);
		tile_y >>= 3;
		int layer_x = in_x;
		do {
			int pixel_x = layer_x;
			if (in_hwrap) {
				if (pixel_x >= g_cwidth) pixel_x %= g_cwidth;
				if (pixel_x >= g_swidth) continue;
			}
			else {
				if (pixel_x < 0) continue;
				else if (pixel_x >= g_swidth) break;
			};
			int tile_x = layer_x - in_x;
			const int sample_x = tile_x & (RGS_PATTERN_WIDTH - 1);
			tile_x >>= 3;
			const RGSTile tile_data = in_tiles[tile_x + (tile_y * ((g_cwidth + (RGS_PATTERN_WIDTH - 1)) >> 3))];
			const uint8_t* pattern_data = g_patterns + ((RGS_PATTERN_WIDTH * RGS_PATTERN_HEIGHT) * (size_t)(tile_data.pattern & (RGS_PATTERN_COUNT - 1)));
			const uint8_t sample_data = pattern_data[(tile_data.hflip ? (RGS_PATTERN_WIDTH - 1) - sample_x : sample_x) + ((tile_data.vflip ? (RGS_PATTERN_HEIGHT - 1) - sample_y : sample_y) * RGS_PATTERN_WIDTH)];
			if (!in_transparent || sample_data) g_pixels[pixel_x + (pixel_y * g_length)] = in_palettes[tile_data.palette][sample_data];
		}
		while (++layer_x < layer_x_end);
	}
	while (++layer_y < layer_y_end);
};


#if RGS_OS == RGS_OS_WINDOWS
static LRESULT CALLBACK RGSUpdateGraphics(HWND in_window, UINT in_message, WPARAM in_wide, LPARAM in_long) {
	switch (in_message) {
	case WM_CLOSE: {
		RGSActivateLock(RGS_LOCK_PASS(g_lock));
		g_running = false;
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		return 0L;
	};
	case WM_GETMINMAXINFO: {
		RECT window_rectangle = { 0L, 0L, (LONG)(g_swidth), (LONG)(g_sheight) };
		AdjustWindowRectEx(&window_rectangle, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
		LPMINMAXINFO min_max = (LPMINMAXINFO)(in_long);
		min_max->ptMinTrackSize = (POINT){ window_rectangle.right - window_rectangle.left, window_rectangle.bottom - window_rectangle.top };
		return 0L;
	};
	case WM_ACTIVATE: {
		if (g_created) RGSActivateLock(RGS_LOCK_PASS(g_lock));
		if (in_wide == WA_ACTIVE || in_wide == WA_CLICKACTIVE) RGSActivateInput();
		else RGSDeactivateInput();
		if (g_created) RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		return 0L;
	};
	case WM_ENTERMENULOOP:
	case WM_ENTERSIZEMOVE: {
		RGSActivateLock(RGS_LOCK_PASS(g_lock));
		RGSPauseGame();
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		return 0L;
	};
	case WM_EXITMENULOOP:
	case WM_EXITSIZEMOVE: {
		RGSActivateLock(RGS_LOCK_PASS(g_lock));
		RGSUnpauseGame();
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		return 0L;
	};
	case WM_MOUSEMOVE: {
		RECT client_rectangle;
		if (!GetClientRect(in_window, &client_rectangle)) break;
		const int surface_width = (int)(client_rectangle.right);
		const int surface_height = (int)(client_rectangle.bottom);
		const float factor_x = (float)(surface_width) / (float)(g_swidth);
		const float factor_y = (float)(surface_height) / (float)(g_sheight);
		float scaled_factor = (float)((int)(factor_x <= factor_y ? factor_x : factor_y));
		const int scaled_width = (int)((float)(g_swidth) * scaled_factor);
		const int scaled_height = (int)((float)(g_sheight) * scaled_factor);
		const int scaled_x = (surface_width >> 1) - (scaled_width >> 1);
		const int scaled_y = (surface_height >> 1) - (scaled_height >> 1);
		scaled_factor = 1.0F / scaled_factor;
		RGSActivateLock(RGS_LOCK_PASS(g_lock));
		RGSModifyMousePosition((int)((float)(GET_X_LPARAM(in_long) - scaled_x) * scaled_factor), (int)((float)(GET_Y_LPARAM(in_long) - scaled_y) * scaled_factor));
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		return 0L;
	};
	case WM_PAINT: {
		RECT client_rectangle;
		if (!GetClientRect(in_window, &client_rectangle)) break;
		const int surface_width = (int)(client_rectangle.right);
		const int surface_height = (int)(client_rectangle.bottom);
		PAINTSTRUCT paint_structure;
		HDC paint_context = BeginPaint(in_window, &paint_structure);
		if (paint_context) {
			HBITMAP surface_bitmap = CreateCompatibleBitmap(paint_context, surface_width, surface_height);
			if (surface_bitmap) {
				HDC surface_context = CreateCompatibleDC(paint_context);
				if (surface_context) {
					HGDIOBJ surface_last = SelectObject(surface_context, surface_bitmap);
					if (surface_last) {
						const float factor_x = (float)(surface_width) / (float)(g_swidth);
						const float factor_y = (float)(surface_height) / (float)(g_sheight);
						const float scaled_factor = (float)((int)(factor_x <= factor_y ? factor_x : factor_y));
						const int scaled_width = (int)((float)(g_swidth) * scaled_factor);
						const int scaled_height = (int)((float)(g_sheight) * scaled_factor);
						StretchDIBits(surface_context, (surface_width >> 1) - (scaled_width >> 1), (surface_height >> 1) - (scaled_height >> 1), scaled_width, scaled_height, 0, 0, g_swidth, g_sheight, (const void*)(g_pixels), (LPBITMAPINFO)(g_bitmap), DIB_RGB_COLORS, SRCCOPY);
						BitBlt(paint_context, 0, 0, surface_width, surface_height, surface_context, 0, 0, SRCCOPY);
						SelectObject(surface_context, surface_last);
					};
					DeleteDC(surface_context);
				};
				DeleteObject(surface_bitmap);
			};
			EndPaint(in_window, &paint_structure);
		};
		return 0L;
	};
	};
	return DefWindowProcA(in_window, in_message, in_wide, in_long);
};

static bool RGSCreateGraphicsWindow(int in_icon, const char* in_title) {
	g_instance = (HINSTANCE)(GetModuleHandle(NULL));
	HICON window_icon = NULL;
	if (IS_INTRESOURCE(in_icon)) {
		window_icon = LoadImage(g_instance, MAKEINTRESOURCE(in_icon), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
		if (!window_icon) RGSReportError("Graphics", "Failed to load icon", false);
	};
	const WNDCLASSEXA window_class = {
		sizeof(window_class), CS_HREDRAW | CS_VREDRAW, &RGSUpdateGraphics, 0, 0, g_instance,
		window_icon, LoadCursor(NULL, IDC_ARROW), (HBRUSH)(GetStockObject(BLACK_BRUSH)), NULL, RGS_GRAPHICS_CLASS_NAME, window_icon
	};
	if (!RegisterClassExA(&window_class)) {
		DestroyIcon(window_icon);
		RGSReportError("Graphics", "Failed to register window", true);
		return false;
	};
	RECT window_rectangle = { 0L, 0L, (LONG)(g_swidth), (LONG)(g_sheight) };
	while (window_rectangle.right < 512U && window_rectangle.bottom <= 512U) {
		window_rectangle.right += (LONG)(g_swidth);
		window_rectangle.bottom += (LONG)(g_sheight);
	};
	if (!AdjustWindowRectEx(&window_rectangle, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW)) {
		RGSReportError("Graphics", "Failed to adjust window", false);
	};
	g_window = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW, RGS_GRAPHICS_CLASS_NAME, in_title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, (int)(window_rectangle.right - window_rectangle.left), (int)(window_rectangle.bottom - window_rectangle.top), NULL, NULL, g_instance, NULL);
	if (!g_window) {
		UnregisterClassA(RGS_GRAPHICS_CLASS_NAME, g_instance);
		RGSReportError("Graphics", "Failed to create window", true);
		return false;
	};
	ShowWindow(g_window, SW_SHOWNORMAL);
	if (!UpdateWindow(g_window) || !IsWindowVisible(g_window)) {
		RGSReportError("Graphics", "Failed to show window", true);
		return false;
	};
	return true;
};
#endif

static void RGSGraphicsThreadJob(void* inout_parameters) {
#if RGS_OS == RGS_OS_WINDOWS
	RGSGraphicsThreadParameters* thread_parameters = (RGSGraphicsThreadParameters*)(inout_parameters);
	if (!RGSCreateGraphicsWindow(thread_parameters->window_icon, thread_parameters->window_title))
#endif
	{
		g_running = false;
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		return;
	};
	g_created = true;
	RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
	while (g_running && !g_started) { };
	if (g_running) {
		while (g_running) RGSRenderGraphics();
	};
	DestroyWindow(g_window);
	UnregisterClassA(RGS_GRAPHICS_CLASS_NAME, g_instance);
};


bool RGSPrepareGraphics(const RGSGameInfo* in_game, const RGSGraphicsInfo* in_graphics) {
	g_cwidth = (int)(in_graphics->canvas_width);
	if (!g_cwidth || g_cwidth > 1024 || g_cwidth % 8) {
		g_cwidth = 1024;
		RGSReportWarning("Graphics", "Canvas width must be a multiple of 8 and between 1 and 1024");
	};
	g_cheight = (int)(in_graphics->canvas_height);
	if (!g_cheight || g_cheight > 1024 || g_cheight % 8) {
		g_cheight = 1024;
		RGSReportWarning("Graphics", "Canvas height must be a multiple of 8 and between 1 and 1024");
	};
	g_swidth = (int)(in_graphics->screen_width);
	if (!g_swidth || g_swidth > g_cwidth) {
		g_swidth = g_cwidth;
		RGSReportWarning("Graphics", "Screen width must be between 1 and canvas width");
	};
	g_sheight = (int)(in_graphics->screen_height);
	if (!g_sheight || g_sheight > g_cheight) {
		g_sheight = g_cheight;
		RGSReportWarning("Graphics", "Screen height must be between 1 and canvas height");
	};
	g_bits = (int)(in_graphics->bits_per_pixel);
	if (g_bits != 1 && g_bits != 2 && g_bits != 4 && g_bits != 8) {
		if (g_bits > 1) {
			if (g_bits > 2) {
				if (g_bits > 4) g_bits = 8;
				else g_bits = 4;
			}
			else g_bits = 2;
		}
		else g_bits = 1;
		RGSReportWarning("Graphics", "Bits per pixel must be 1, 2, 4 or 8");
	};
	g_colours = 1 << g_bits;
	g_rate = (RGSTime)(in_graphics->frame_rate);
	if (!g_rate || g_rate > 60ULL) {
		g_rate = 60ULL;
		RGSReportWarning("Graphics", "Frame rate must be between 1 and 60");
	};
	int line_size = 0;
	switch (g_bits) {
	case 1:
		line_size = (g_swidth + 7) >> 3;
		g_get_pixel = &RGSGetPixel1;
		g_set_pixel = &RGSSetPixel1;
		g_draw_sprite = &RGSDrawSprite1;
		g_draw_tiles = &RGSDrawTiles1;
		break;
	case 2:
#if RGS_OS == RGS_OS_WINDOWS
		g_bits = 4;
#else
		line_size = (g_swidth + 3) >> 2;
		g_get_pixel = &RGSGetPixel2;
		g_set_pixel = &RGSSetPixel2;
		g_draw_sprite = &RGSDrawSprite2;
		g_draw_tiles = &RGSDrawTiles2;
		break;
#endif
	case 4:
		line_size = (g_swidth + 1) >> 1;
		g_get_pixel = &RGSGetPixel4;
		g_set_pixel = &RGSSetPixel4;
		g_draw_sprite = &RGSDrawSprite4;
		g_draw_tiles = &RGSDrawTiles4;
		break;
	case 8:
		line_size = g_swidth;
		g_get_pixel = &RGSGetPixel8;
		g_set_pixel = &RGSSetPixel8;
		g_draw_sprite = &RGSDrawSprite8;
		g_draw_tiles = &RGSDrawTiles8;
		break;
	};
	g_patterns = calloc((size_t)(((RGS_PATTERN_WIDTH * g_bits) >> 3) * RGS_PATTERN_HEIGHT * RGS_PATTERN_COUNT), sizeof(*g_patterns));
	if (!g_patterns) {
		RGSReportError("Graphics", "Failed to allocate patterns", true);
		return false;
	};
#if RGS_OS == RGS_OS_WINDOWS
	const int line_padding = line_size % 4;
	g_length = line_size + (line_padding ? 4 - line_padding : 0);
#endif
	g_pixels = (uint8_t*)(malloc((size_t)(g_length * g_sheight) * sizeof(*g_pixels)));
	if (!g_pixels) {
		free((void*)(g_patterns));
		RGSReportError("Graphics", "Failed to allocate pixels", true);
		return false;
	};
#if RGS_OS == RGS_OS_WINDOWS
	g_bitmap = (LPBITMAPINFO)(malloc(sizeof(*g_bitmap) + (sizeof(*g_bitmap->bmiColors) * ((size_t)(g_colours) - 1U))));
	if (!g_bitmap) {
		free((void*)(g_pixels));
		free((void*)(g_patterns));
		RGSReportError("Graphics", "Failed to allocate bitmap", true);
		return false;
	};
	g_bitmap->bmiHeader.biSize = sizeof(g_bitmap->bmiHeader);
	g_bitmap->bmiHeader.biWidth = (LONG)(g_swidth);
	g_bitmap->bmiHeader.biHeight = -(LONG)(g_sheight);
	g_bitmap->bmiHeader.biPlanes = 1U;
	g_bitmap->bmiHeader.biBitCount = g_bits;
	g_bitmap->bmiHeader.biCompression = BI_RGB;
	g_bitmap->bmiHeader.biSizeImage = 0UL;
	g_bitmap->bmiHeader.biXPelsPerMeter = 0L;
	g_bitmap->bmiHeader.biYPelsPerMeter = 0L;
	g_bitmap->bmiHeader.biClrUsed = (DWORD)(g_colours);
	g_bitmap->bmiHeader.biClrImportant = (DWORD)(g_colours);
	const BYTE colour_step = 255U / (BYTE)((1 << g_bits) - 1);
	for (int colour_index = 0; colour_index < g_colours; colour_index++) {
		const BYTE colour_value = (BYTE)(colour_index) * colour_step;
		g_bitmap->bmiColors[colour_index] = (RGBQUAD){ colour_value, colour_value, colour_value, 0U };
	};
#endif
	g_lock = RGSCreateLock(true);
	if (g_lock == RGS_LOCK_INVALID) {
#if RGS_OS == RGS_OS_WINDOWS
		free(g_bitmap);
#endif
		free(g_pixels);
		free(g_patterns);
		RGSReportError("Graphics", "Failed to create lock", true);
		return false;
	};
#if RGS_DEVICE == RGS_DEVICE_DESKTOP
	RGSGraphicsThreadParameters thread_parameters = { in_graphics->window_icon, in_graphics->window_title };
	g_thread = in_graphics->threaded ? RGSCreateThread(&RGSGraphicsThreadJob, RGS_LOCK_PASS(g_lock), &thread_parameters) : RGS_THREAD_INVALID;
#endif
	if (g_thread == RGS_THREAD_INVALID) {
		if (in_graphics->threaded) RGSReportError("Graphics", "Failed to create thread", false);
		if (!RGSCreateGraphicsWindow(in_graphics->window_icon, in_graphics->window_title)) {
#if RGS_OS == RGS_OS_WINDOWS
			free(g_bitmap);
#endif
			free(g_pixels);
			free(g_patterns);
			return false;
		};
		g_created = true;
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
	};
	return true;
};

void RGSReleaseGraphics() {
	if (g_thread) {
		RGSActivateLock(RGS_LOCK_PASS(g_lock));
		g_running = false;
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		RGSWaitForThread(RGS_THREAD_PASS(g_thread));
		RGSDestroyThread(RGS_THREAD_PASS(g_thread));
	}
	else {
#if RGS_OS == RGS_OS_WINDOWS
		DestroyWindow(g_window);
		UnregisterClassA(RGS_GRAPHICS_CLASS_NAME, g_instance);
#endif
	};
	RGSDestroyLock(RGS_LOCK_PASS(g_lock));
#if RGS_OS == RGS_OS_WINDOWS
	free((void*)(g_bitmap));
#endif
	free((void*)(g_pixels));
	free((void*)(g_patterns));
};


void RGSStartGraphics() { g_started = true; };

void RGSRenderGraphics() {
#if RGS_OS == RGS_OS_WINDOWS
	MSG message_info;
	while (PeekMessageA(&message_info, NULL, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&message_info);
		DispatchMessageA(&message_info);
	};
#endif
	const RGSTime current_time = RGSTimeNow();
	if ((current_time - g_rendered) >= (RGS_ONE_SECOND / g_rate)) {
		RGSActivateLock(RGS_LOCK_PASS(g_lock));
		memset((void*)(g_pixels), 0, (size_t)(g_length * g_sheight) * sizeof(*g_pixels));
		g_rendering = true;
		RGSRender();
		g_rendering = false;
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
#if RGS_OS == RGS_OS_WINDOWS
		InvalidateRect(g_window, NULL, FALSE);
		UpdateWindow(g_window);
#endif
		g_rendered = current_time;
	};
};


void RGSLockGraphics() {
	RGSActivateLock(RGS_LOCK_PASS(g_lock));
	g_modifying = true;
};

void RGSUnlockGraphics() {
	g_modifying = false;
	RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
};


bool RGSGraphicsThreaded() { return g_thread != RGS_THREAD_INVALID; };

bool RGSGraphicsRunning() { return g_running; };


/// Exposed Graphics Functions

RGSColour RGSGetColour(uint8_t in_index) {
	if (!g_modifying) return 0U;
#if RGS_OS == RGS_OS_WINDOWS
	return *((RGSColour*)(g_bitmap->bmiColors) + ((size_t)(in_index) % g_colours));
#endif
};

void RGSSetColour(uint8_t in_index, RGSColour in_packed) {
	if (!g_modifying) return;
#if RGS_OS == RGS_OS_WINDOWS
	*((RGSColour*)(g_bitmap->bmiColors) + ((size_t)(in_index) % g_colours)) = in_packed;
#endif
};

void RGSReadColours(RGSColour* out_data) {
	if (!g_modifying || !out_data) return;
#if RGS_OS == RGS_OS_WINDOWS
	memcpy(out_data, (const void*)(g_bitmap->bmiColors), (size_t)(g_colours) * sizeof(*g_bitmap->bmiColors));
#endif
};

void RGSWriteColours(const RGSColour* in_data) {
	if (!g_modifying || !in_data) return;
#if RGS_OS == RGS_OS_WINDOWS
	memcpy((void*)(g_bitmap->bmiColors), in_data, (size_t)(g_colours) * sizeof(*g_bitmap->bmiColors));
#endif
};


void RGSReadPattern(RGSPattern in_index, uint32_t in_bits, uint8_t* out_data) {
	if (!g_modifying || !out_data) return;
	const uint8_t* pattern_data;
	switch (g_bits) {
	case 1:
		pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 3) * RGS_PATTERN_HEIGHT) * (size_t)(in_index & (RGS_PATTERN_COUNT - 1)));
		switch (in_bits) {
		case 1U: {
			memcpy(out_data, pattern_data, ((RGS_PATTERN_WIDTH >> 3) * RGS_PATTERN_HEIGHT) * sizeof(*pattern_data));
			break;
		};
		case 2U: {
			const uint8_t* const data_end = out_data + (RGS_PATTERN_HEIGHT << 1);
			do {
				uint8_t sample_data = ((*pattern_data >> 7U) & ((1U << 1U) - 1U)) << 6U;
				sample_data |= ((*pattern_data >> 6U) & ((1U << 1U) - 1U)) << 4U;
				sample_data |= ((*pattern_data >> 5U) & ((1U << 1U) - 1U)) << 2U;
				*(out_data++) = sample_data | ((*pattern_data >> 4U) & ((1U << 1U) - 1U));
				sample_data = ((*pattern_data >> 3U) & ((1U << 1U) - 1U)) << 6U;
				sample_data |= ((*pattern_data >> 2U) & ((1U << 1U) - 1U)) << 4U;
				sample_data |= ((*pattern_data >> 1U) & ((1U << 1U) - 1U)) << 2U;
				*(out_data++) = sample_data | (*(pattern_data++) & ((1U << 1U) - 1U));
			}
			while (out_data < data_end);
			break;
		};
		case 4U: {
			const uint8_t* const data_end = out_data + (RGS_PATTERN_HEIGHT << 2);
			do {
				uint8_t sample_data = ((*pattern_data >> 7U) & ((1U << 1U) - 1U)) << 4U;
				*(out_data++) = sample_data | ((*pattern_data >> 6U) & ((1U << 1U) - 1U));
				sample_data = ((*pattern_data >> 5U) & ((1U << 1U) - 1U)) << 4U;
				*(out_data++) = sample_data | ((*pattern_data >> 4U) & ((1U << 1U) - 1U));
				sample_data = ((*pattern_data >> 3U) & ((1U << 1U) - 1U)) << 4U;
				*(out_data++) = sample_data | ((*pattern_data >> 2U) & ((1U << 1U) - 1U));
				sample_data = ((*pattern_data >> 1U) & ((1U << 1U) - 1U)) << 4U;
				*(out_data++) = sample_data | (*(pattern_data++) & ((1U << 1U) - 1U));
			}
			while (out_data < data_end);
			break;
		};
		case 8U: {
			const uint8_t* const data_end = out_data + (RGS_PATTERN_HEIGHT << 3);
			do {
				*(out_data++) = (*pattern_data >> 7U) & ((1U << 1U) - 1U);
				*(out_data++) = (*pattern_data >> 6U) & ((1U << 1U) - 1U);
				*(out_data++) = (*pattern_data >> 5U) & ((1U << 1U) - 1U);
				*(out_data++) = (*pattern_data >> 4U) & ((1U << 1U) - 1U);
				*(out_data++) = (*pattern_data >> 3U) & ((1U << 1U) - 1U);
				*(out_data++) = (*pattern_data >> 2U) & ((1U << 1U) - 1U);
				*(out_data++) = (*pattern_data >> 1U) & ((1U << 1U) - 1U);
				*(out_data++) = *(pattern_data++) & ((1U << 1U) - 1U);
			}
			while (out_data < data_end);
			break;
		};
		};
		break;
	case 2:
		pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 2) * RGS_PATTERN_HEIGHT) * (size_t)(in_index & (RGS_PATTERN_COUNT - 1)));
		switch (in_bits) {
		case 1U: break;
		case 2U: {
			memcpy(out_data, pattern_data, ((RGS_PATTERN_WIDTH >> 2) * RGS_PATTERN_HEIGHT) * sizeof(*pattern_data));
			break;
		};
		case 4U: break;
		case 8U: break;
		};
		break;
	case 4:
		pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 1) * RGS_PATTERN_HEIGHT) * (size_t)(in_index & (RGS_PATTERN_COUNT - 1)));
		switch (in_bits) {
		case 1U: {
			const uint8_t* const data_end = out_data + RGS_PATTERN_HEIGHT;
			do {
				uint8_t sample_data = ((*pattern_data >> 4U) & ((1U << 1U) - 1U)) << 7U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 6U;
				sample_data |= ((*pattern_data >> 4U) & ((1U << 1U) - 1U)) << 5U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 4U;
				sample_data |= ((*pattern_data >> 4U) & ((1U << 1U) - 1U)) << 3U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 2U;
				sample_data |= ((*pattern_data >> 4U) & ((1U << 1U) - 1U)) << 1U;
				*out_data = sample_data | (*(pattern_data++) & ((1U << 1U) - 1U));
			}
			while (++out_data < data_end);
			break;
		};
		case 2U: {
			const uint8_t* const data_end = out_data + (RGS_PATTERN_HEIGHT << 1);
			do {
				uint8_t sample_data = ((*pattern_data >> 4U) & ((1U << 2U) - 1U)) << 6U;
				sample_data |= (*(pattern_data++) & ((1U << 2U) - 1U)) << 4U;
				sample_data |= ((*pattern_data >> 4U) & ((1U << 2U) - 1U)) << 2U;
				*out_data = sample_data | (*(pattern_data++) & ((1U << 2U) - 1U));
			}
			while (++out_data < data_end);
			break;
		};
		case 4U: {
			memcpy(out_data, pattern_data, ((RGS_PATTERN_WIDTH >> 1) * RGS_PATTERN_HEIGHT) * sizeof(*pattern_data));
			break;
		};
		case 8U: {
			const uint8_t* const data_end = out_data + (RGS_PATTERN_WIDTH * RGS_PATTERN_HEIGHT);
			do {
				const uint8_t sample_data = *(pattern_data++);
				*(out_data++) = (sample_data >> 4U) & ((1U << 8U) - 1U);
				*(out_data++) = sample_data & ((1U << 8U) - 1U);
			}
			while (out_data < data_end);
			break;
		};
		};
		break;
	case 8:
		pattern_data = g_patterns + ((RGS_PATTERN_WIDTH * RGS_PATTERN_HEIGHT) * (size_t)(in_index & (RGS_PATTERN_COUNT - 1)));
		switch (in_bits) {
		case 1U: {
			const uint8_t* const data_end = out_data + RGS_PATTERN_HEIGHT;
			do {
				uint8_t sample_data = (*(pattern_data++) & ((1U << 1U) - 1U)) << 7U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 6U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 5U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 4U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 3U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 2U;
				sample_data |= (*(pattern_data++) & ((1U << 1U) - 1U)) << 1U;
				*out_data = sample_data | (*(pattern_data++) & ((1U << 1U) - 1U));
			}
			while (++out_data < data_end);
			break;
		};
		case 2U: {
			const uint8_t* const data_end = out_data + (RGS_PATTERN_HEIGHT << 1);
			do {
				uint8_t sample_data = (*(pattern_data++) & ((1U << 2U) - 1U)) << 6U;
				sample_data |= (*(pattern_data++) & ((1U << 2U) - 1U)) << 4U;
				sample_data |= (*(pattern_data++) & ((1U << 2U) - 1U)) << 2U;
				*out_data = sample_data | (*(pattern_data++) & ((1U << 2U) - 1U));
			}
			while (++out_data < data_end);
			break;
		};
		case 4U: {
			const uint8_t* const data_end = out_data + (RGS_PATTERN_HEIGHT << 2);
			do {
				const uint8_t sample_data = (*(pattern_data++) & ((1U << 4U) - 1U)) << 4U;
				*out_data = sample_data | (*(pattern_data++) & ((1U << 4U) - 1U));
			}
			while (++out_data < data_end);
			break;
		};
		case 8U: {
			memcpy(out_data, pattern_data, (RGS_PATTERN_WIDTH * RGS_PATTERN_HEIGHT) * sizeof(*pattern_data));
			break;
		};
		};
		break;
	};
};

void RGSWritePattern(RGSPattern in_index, uint32_t in_bits, const uint8_t* in_data) {
	if (!g_modifying || !in_data) return;
	uint8_t* pattern_data;
	switch (g_bits) {
	case 1:
		pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 3) * RGS_PATTERN_HEIGHT) * (size_t)(in_index & (RGS_PATTERN_COUNT - 1)));
		switch (in_bits) {
		case 1U: {
			memcpy(pattern_data, in_data, ((RGS_PATTERN_WIDTH >> 3) * RGS_PATTERN_HEIGHT) * sizeof(*pattern_data));
			break;
		};
		case 2U: {
			const uint8_t* const data_end = in_data + (RGS_PATTERN_HEIGHT << 1);
			do {
				uint8_t sample_data = *(in_data++);
				*pattern_data = ((sample_data >> 6U) & ((1U << 1U) - 1U)) << 7U;
				*pattern_data |= ((sample_data >> 4U) & ((1U << 1U) - 1U)) << 6U;
				*pattern_data |= ((sample_data >> 2U) & ((1U << 1U) - 1U)) << 5U;
				*pattern_data |= (sample_data & ((1U << 1U) - 1U)) << 4U;
				sample_data = *(in_data++);
				*pattern_data |= ((sample_data >> 6U) & ((1U << 1U) - 1U)) << 3U;
				*pattern_data |= ((sample_data >> 4U) & ((1U << 1U) - 1U)) << 2U;
				*pattern_data |= ((sample_data >> 2U) & ((1U << 1U) - 1U)) << 1U;
				*(pattern_data++) |= sample_data & ((1U << 1U) - 1U);
			}
			while (in_data < data_end);
			break;
		};
		case 4U: {
			const uint8_t* const data_end = in_data + (RGS_PATTERN_HEIGHT << 2);
			do {
				uint8_t sample_data = *(in_data++);
				*pattern_data = ((sample_data >> 4U) & ((1U << 1U) - 1U)) << 7U;
				*pattern_data |= (sample_data & ((1U << 1U) - 1U)) << 6U;
				sample_data = *(in_data++);
				*pattern_data |= ((sample_data >> 4U) & ((1U << 1U) - 1U)) << 5U;
				*pattern_data |= (sample_data & ((1U << 1U) - 1U)) << 4U;
				sample_data = *(in_data++);
				*pattern_data |= ((sample_data >> 4U) & ((1U << 1U) - 1U)) << 3U;
				*pattern_data |= (sample_data & ((1U << 1U) - 1U)) << 2U;
				sample_data = *(in_data++);
				*pattern_data |= ((sample_data >> 4U) & ((1U << 1U) - 1U)) << 1U;
				*(pattern_data++) |= sample_data & ((1U << 1U) - 1U);
			}
			while (in_data < data_end);
			break;
		};
		case 8U: {
			const uint8_t* const data_end = in_data + (RGS_PATTERN_HEIGHT << 3);
			do {
				*pattern_data = (*(in_data++) & ((1U << 1U) - 1U)) << 7U;
				*pattern_data |= (*(in_data++) & ((1U << 1U) - 1U)) << 6U;
				*pattern_data |= (*(in_data++) & ((1U << 1U) - 1U)) << 5U;
				*pattern_data |= (*(in_data++) & ((1U << 1U) - 1U)) << 4U;
				*pattern_data |= (*(in_data++) & ((1U << 1U) - 1U)) << 3U;
				*pattern_data |= (*(in_data++) & ((1U << 1U) - 1U)) << 2U;
				*pattern_data |= (*(in_data++) & ((1U << 1U) - 1U)) << 1U;
				*(pattern_data++) |= *(in_data++) & ((1U << 1U) - 1U);
			}
			while (in_data < data_end);
			break;
		};
		};
		break;
	case 2:
		pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 2) * RGS_PATTERN_HEIGHT) * (size_t)(in_index & (RGS_PATTERN_COUNT - 1)));
		switch (in_bits) {
		case 1U: break;
		case 2U: {
			memcpy(pattern_data, in_data, ((RGS_PATTERN_WIDTH >> 2) * RGS_PATTERN_HEIGHT) * sizeof(*pattern_data));
			break;
		};
		case 4U: break;
		case 8U: break;
		};
		break;
	case 4:
		pattern_data = g_patterns + (((RGS_PATTERN_WIDTH >> 1) * RGS_PATTERN_HEIGHT) * (size_t)(in_index & (RGS_PATTERN_COUNT - 1)));
		switch (in_bits) {
		case 1U: {
			const uint8_t* const data_end = in_data + RGS_PATTERN_HEIGHT;
			do {
				const uint8_t sample_data = *in_data;
				*pattern_data = ((sample_data >> 7U) & ((1U << 1U) - 1U)) << 4U;
				*(pattern_data++) |= (sample_data >> 6U) & ((1U << 1U) - 1U);
				*pattern_data = ((sample_data >> 5U) & ((1U << 1U) - 1U)) << 4U;
				*(pattern_data++) |= (sample_data >> 4U) & ((1U << 1U) - 1U);
				*pattern_data = ((sample_data >> 3U) & ((1U << 1U) - 1U)) << 4U;
				*(pattern_data++) |= (sample_data >> 2U) & ((1U << 1U) - 1U);
				*pattern_data = ((sample_data >> 1U) & ((1U << 1U) - 1U)) << 4U;
				*(pattern_data++) |= sample_data & ((1U << 1U) - 1U);
			}
			while (++in_data < data_end);
			break;
		};
		case 2U: {
			const uint8_t* const data_end = in_data + (RGS_PATTERN_HEIGHT << 1);
			do {
				const uint8_t sample_data = *in_data;
				*pattern_data = ((sample_data >> 6U) & ((1U << 2U) - 1U)) << 4;
				*(pattern_data++) |= (sample_data >> 4U) & ((1U << 2U) - 1U);
				*pattern_data = ((sample_data >> 2U) & ((1U << 2U) - 1U)) << 4;
				*(pattern_data++) |= sample_data & ((1U << 2U) - 1U);
			}
			while (++in_data < data_end);
			break;
		};
		case 4U: {
			memcpy(pattern_data, in_data, ((RGS_PATTERN_WIDTH >> 1) * RGS_PATTERN_HEIGHT) * sizeof(*pattern_data));
			break;
		};
		case 8U: {
			const uint8_t* const data_end = in_data + (RGS_PATTERN_WIDTH * RGS_PATTERN_HEIGHT);
			do {
				*pattern_data = *(in_data++) << 4U;
				*(pattern_data++) |= *(in_data++);
			}
			while (in_data < data_end);
			break;
		};
		};
		break;
	case 8:
		pattern_data = g_patterns + ((RGS_PATTERN_WIDTH * RGS_PATTERN_HEIGHT) * (size_t)(in_index & (RGS_PATTERN_COUNT - 1)));
		switch (in_bits) {
		case 1U: {
			const uint8_t* const data_end = in_data + RGS_PATTERN_HEIGHT;
			do {
				const uint8_t sample_data = *in_data;
				*(pattern_data++) = (sample_data >> 7U) & ((1U << 1U) - 1U);
				*(pattern_data++) = (sample_data >> 6U) & ((1U << 1U) - 1U);
				*(pattern_data++) = (sample_data >> 5U) & ((1U << 1U) - 1U);
				*(pattern_data++) = (sample_data >> 4U) & ((1U << 1U) - 1U);
				*(pattern_data++) = (sample_data >> 3U) & ((1U << 1U) - 1U);
				*(pattern_data++) = (sample_data >> 2U) & ((1U << 1U) - 1U);
				*(pattern_data++) = (sample_data >> 1U) & ((1U << 1U) - 1U);
				*(pattern_data++) = sample_data & ((1U << 1U) - 1U);
			}
			while (++in_data < data_end);
			break;			
		};
		case 2U: {
			const uint8_t* const data_end = in_data + (RGS_PATTERN_HEIGHT << 1);
			do {
				const uint8_t sample_data = *in_data;
				*(pattern_data++) = (sample_data >> 6U) & ((1U << 2U) - 1U);
				*(pattern_data++) = (sample_data >> 4U) & ((1U << 2U) - 1U);
				*(pattern_data++) = (sample_data >> 2U) & ((1U << 2U) - 1U);
				*(pattern_data++) = sample_data & ((1U << 2U) - 1U);
			}
			while (++in_data < data_end);
			break;
		};
		case 4U: {
			const uint8_t* const data_end = in_data + (RGS_PATTERN_HEIGHT << 2);
			do {
				const uint8_t sample_data = *in_data;
				*(pattern_data++) = (sample_data >> 4U) & ((1U << 4U) - 1U);
				*(pattern_data++) = sample_data & ((1U << 4U) - 1U);
			}
			while (++in_data < data_end);
			break;
		};
		case 8U: {
			memcpy(pattern_data, in_data, (RGS_PATTERN_WIDTH * RGS_PATTERN_HEIGHT) * sizeof(*pattern_data));
			break;
		};
		};
		break;
	};
};


uint8_t RGSGetPixel(int in_x, int in_y) {
	if (!g_rendering || in_x < 0 || in_x >= g_swidth || in_y < 0 || in_y >= g_sheight) return 0U;
	return g_get_pixel(in_x, in_y);
};

void RGSSetPixel(int in_x, int in_y, uint8_t in_index) {
	if (!g_rendering || in_x < 0 || in_x >= g_swidth || in_y < 0 || in_y >= g_sheight) return;
	g_set_pixel(in_x, in_y, in_index);
};


void RGSDrawSprite(int in_x, int in_y, RGSPattern in_pattern, RGSPalette in_palette, bool in_hflip, bool in_vflip, bool in_hwrap, bool in_vwrap) {
	if (!g_rendering) return;
	g_draw_sprite(in_x, in_y, in_pattern, in_palette, in_hflip, in_vflip, in_hwrap, in_vwrap);
};

void RGSDrawTiles(int in_x, int in_y, const RGSTile* in_tiles, const RGSPalette* in_palettes, bool in_hwrap, bool in_vwrap, bool in_transparent) {
	if (!g_rendering || !in_tiles || !in_palettes) return;
	g_draw_tiles(in_x, in_y, in_tiles, in_palettes, in_hwrap, in_vwrap, in_transparent);
};

