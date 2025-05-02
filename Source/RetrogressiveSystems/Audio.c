#include "./Audio.inl"


#include <RetrogressiveSystems/Output.h>

#include "./Threads.inl"

#include <stdlib.h>
#include <math.h>


#if RGS_OS == RGS_OS_WINDOWS
#include <Audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#endif


/// Internal Audio Types

typedef enum RGSAudioActionType {
	RGS_AUDIO_ACTION_TYPE_NOTE,
} RGSAudioActionType;

typedef struct RGSAudioAction {
	RGSAudioActionType type;
	float offset;
	float length;
	float speed;
	float volume;
	float position;
	union {
		struct {
			RGSInstrument instrument;
			uint8_t index;
		} note;
	};
} RGSAudioAction;


/// Internal Audio Variables

static uint32_t g_bits = 0U;
static bool g_stereo = false;
static size_t g_samples = 0U;

static RGSInstrument* g_ilist = RGS_NULL;
static size_t g_icount = 0U;

static RGSAudioAction* g_alist = RGS_NULL;
static size_t g_acount = 0U;
static size_t g_acapacity = 0U;

static RGSLock g_lock = RGS_LOCK_INVALID;
static RGSThread g_thread = RGS_THREAD_INVALID;
static volatile bool g_modifying = false;

static RGSTime g_rendered = 0ULL;
static volatile bool g_running = true;
static volatile bool g_started = false;

#if RGS_OS == RGS_OS_WINDOWS
extern const CLSID CLSID_MMDeviceEnumerator = { 0xBCDE0395UL, 0xE52FU, 0x467CU, { 0x8EU, 0x3DU, 0xC4U, 0x57U, 0x92U, 0x91U, 0x69U, 0x2EU } };
extern const IID IID_IMMDeviceEnumerator = { 0xA95664D2UL, 0x9614U, 0x4F35U, { 0xA7U, 0x46U, 0xDEU, 0x8DU, 0xB6U, 0x36U, 0x17U, 0xE6U } };
extern const IID IID_IMMDevice = { 0xA95664D2UL, 0x9614U, 0x4F35U, { 0xA7U, 0x46U, 0xDEU, 0x8DU, 0xB6U, 0x36U, 0x17U, 0xE6U } };
extern const IID IID_IAudioClient = { 0x1CB9AD4CUL, 0xDBFAU, 0x4c32U, { 0xB1U, 0x78U, 0xC2U, 0xF5U, 0x68U, 0xA7U, 0x03U, 0xB2U } };
extern const IID IID_IAudioRenderClient = { 0xF294ACFCUL, 0x3146U, 0x4483U, { 0xA7U, 0xBFU, 0xADU, 0xDCU, 0xA7U, 0xC2U, 0x60U, 0xE2U } };
static UINT g_fcount = 0U;
static WAVEFORMATEXTENSIBLE* g_format = NULL;
static IMMDeviceEnumerator* g_enumerator = NULL;
static IMMDevice* g_device = NULL;
static IAudioClient* g_client = NULL;
static IAudioRenderClient* g_renderer = NULL;
#endif


/// Internal Audio Functions

static void RGSAddAudioAction(const RGSAudioAction* in_action) {
	if (g_acount >= g_acapacity) {
		const size_t action_capacity = g_acount + 64U;
		RGSAudioAction* action_list = (RGSAudioAction*)(realloc(g_alist, action_capacity * sizeof(*action_list)));
		if (!action_list) {
			RGSReportError("Audio", "Failed to allocate action list", false);
			return;
		};
		g_alist = action_list;
		g_acapacity = action_capacity;
	};
	g_alist[g_acount++] = *in_action;
};

static void RGSRemoveAudioAction(size_t in_index) {
	for (size_t action_index = in_index + 1U; action_index < g_acount; action_index++) {
		g_alist[action_index - 1U] = g_alist[action_index];
	};
	g_acount--;
};


static bool RGSCheckAudio() {
#if RGS_OS == RGS_OS_WINDOWS
	if (g_device) {
		DWORD device_state = DEVICE_STATE_DISABLED;
		if (g_device->lpVtbl->GetState(g_device, &device_state) == S_OK && device_state == DEVICE_STATE_ACTIVE) return true;
		g_client->lpVtbl->Stop(g_client);
		g_renderer->lpVtbl->Release(g_renderer);
		g_renderer = NULL;
		g_client->lpVtbl->Release(g_client);
		g_client = NULL;
		g_device->lpVtbl->Release(g_device);
		g_device = NULL;
		CoTaskMemFree(g_format);
		g_format = NULL;
		g_fcount = 0U;
	};
	IMMDevice* audio_device = NULL;
	if (g_enumerator->lpVtbl->GetDefaultAudioEndpoint(g_enumerator, eRender, eConsole, &audio_device) != S_OK) return false;
	IAudioClient* audio_client = NULL;
	if (audio_device->lpVtbl->Activate(audio_device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)(&audio_client)) != S_OK) {
		audio_device->lpVtbl->Release(audio_device);
		return false;
	};
	WAVEFORMATEXTENSIBLE* audio_format = NULL;
	if (audio_client->lpVtbl->GetMixFormat(audio_client, (WAVEFORMATEX**)(&audio_format)) != S_OK) {
		audio_client->lpVtbl->Release(audio_client);
		audio_device->lpVtbl->Release(audio_device);
		return false;
	};
	if (audio_client->lpVtbl->Initialize(audio_client, AUDCLNT_SHAREMODE_SHARED, 0UL, 1000LL, 0LL, (const WAVEFORMATEX*)(audio_format), NULL) != S_OK) {
		CoTaskMemFree(audio_format);
		audio_client->lpVtbl->Release(audio_client);
		audio_device->lpVtbl->Release(audio_device);
		return false;
	};
	IAudioRenderClient* audio_renderer = NULL;
	if (audio_client->lpVtbl->GetService(audio_client, &IID_IAudioRenderClient, (void**)(&audio_renderer)) != S_OK) {
		CoTaskMemFree(audio_format);
		audio_client->lpVtbl->Release(audio_client);
		audio_device->lpVtbl->Release(audio_device);
		return false;
	};
	UINT frame_count = 0U;
	if (audio_client->lpVtbl->GetBufferSize(audio_client, &frame_count) != S_OK ||
		audio_client->lpVtbl->Start(audio_client) != S_OK) {
		audio_renderer->lpVtbl->Release(audio_renderer);
		CoTaskMemFree(audio_format);
		audio_client->lpVtbl->Release(audio_client);
		audio_device->lpVtbl->Release(audio_device);
		return false;
	};
	g_samples = (size_t)(audio_format->Format.nSamplesPerSec);
	g_format = audio_format;
	g_fcount = frame_count;
	g_device = audio_device;
	g_client = audio_client;
	g_renderer = audio_renderer;
#endif
	return true;
};

static void RGSAudioThreadJob(void* inout_parameters) {
	RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
	while (g_running && !g_started) { };
	if (g_running) {
		while (g_running) RGSRenderAudio();
#if RGS_OS == RGS_OS_WINDOWS
		if (g_client) g_client->lpVtbl->Stop(g_client);
		if (g_renderer) g_renderer->lpVtbl->Release(g_renderer);
		if (g_client) g_client->lpVtbl->Release(g_client);
		if (g_device) g_device->lpVtbl->Release(g_device);
		if (g_format) CoTaskMemFree(g_format);
#endif
	};
};


bool RGSPrepareAudio(const RGSGameInfo* in_game, const RGSAudioInfo* in_audio) {
	g_icount = (size_t)(in_audio->instrument_count);
	if (g_icount) {
		if (g_icount > 256) {
			RGSReportWarning("Audio", "Instrument count must be lower than or equal to 256");
			g_icount = 256U;
		};
		g_ilist = (RGSInstrument*)(calloc(g_icount, sizeof(*g_ilist)));
		if (!g_ilist) {
			RGSReportError("Audio", "Failed to allocate instrument list", true);
			return false;
		};
	};
	g_bits = in_audio->bits_per_sample;
	if (g_bits != 8U && g_bits != 16U) {
		if (g_bits > 8U) g_bits = 16U;
		else g_bits = 8U;
		RGSReportWarning("Audio", "Bits per sample must be 8 or 16");
	};
	g_stereo = in_audio->stereo;
	g_lock = RGSCreateLock(true);
	if (g_lock == RGS_LOCK_INVALID) {
		if (g_ilist) free(g_ilist);
		RGSReportError("Audio", "Failed to create lock", true);
		return false;
	};
#if RGS_OS == RGS_OS_WINDOWS
	if (CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &IID_IMMDeviceEnumerator, &g_enumerator) != S_OK) {
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		RGSDestroyLock(RGS_LOCK_PASS(g_lock));
		if (g_ilist) free(g_ilist);
		RGSReportError("Audio", "Failed to create device enumerator", true);
		return false;
	};
#endif
	g_thread = in_audio->threaded ? RGSCreateThread(&RGSAudioThreadJob, RGS_LOCK_PASS(g_lock), RGS_NULL) : RGS_THREAD_INVALID;
	if (g_thread == RGS_THREAD_INVALID) {
		if (in_audio->threaded) RGSReportError("Audio", "Failed to create thread", false);
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
	};
	return true;
};

void RGSReleaseAudio() {
	if (g_thread) {
		RGSActivateLock(RGS_LOCK_PASS(g_lock));
		g_running = false;
		RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		RGSWaitForThread(RGS_THREAD_PASS(g_thread));
		RGSDestroyThread(RGS_THREAD_PASS(g_thread));
	}
	else {
#if RGS_OS == RGS_OS_WINDOWS
		if (g_client) g_client->lpVtbl->Stop(g_client);
		if (g_renderer) g_renderer->lpVtbl->Release(g_renderer);
		if (g_client) g_client->lpVtbl->Release(g_client);
		if (g_device) g_device->lpVtbl->Release(g_device);
		if (g_format) CoTaskMemFree(g_format);
#endif
	};
	RGSDestroyLock(RGS_LOCK_PASS(g_lock));
#if RGS_OS == RGS_OS_WINDOWS
	g_enumerator->lpVtbl->Release(g_enumerator);
#endif
	if (g_alist) free(g_alist);
	if (g_ilist) free(g_ilist);
};


void RGSStartAudio() { g_started = true; };

void RGSStopAudio() {
	RGSActivateLock(RGS_LOCK_PASS(g_lock));
	g_running = false;
	RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
	RGSWaitForThread(RGS_THREAD_PASS(g_thread));
};


static void RGSRenderAudioSampleActions(float in_offset, size_t in_actions, float* out_left, float* out_right) {
	for (size_t action_index = 0U; action_index < in_actions; action_index++) {
		const float action_offset = g_alist[action_index].offset + (in_offset * g_alist[action_index].speed);
		const float action_length = g_alist[action_index].length;
		if (action_offset < action_length) {
			float left_value = 0.0F, right_value = 0.0F;
			const float action_volume = (action_offset >= 0.1F ? (action_offset >= (action_length - 0.1F) ? (1.0F - ((action_offset - (action_length - 0.1F)) * 10.0F)) : 1.0F) : (action_offset * 10.0F));
			switch (g_alist[action_index].type) {
			case RGS_AUDIO_ACTION_TYPE_NOTE: {
				const float note_value = g_alist[action_index].note.instrument(g_alist[action_index].note.index, action_offset) * g_alist[action_index].volume * action_volume;
				left_value = note_value;
				right_value = note_value;
				break;
			};
			};
			const float right_volume = ((g_alist[action_index].position + 1.0F) * 0.5F);
			*out_left = (*out_left != 0.0F ? (*out_left * 0.5F) + (left_value * 0.5F) : left_value) * (1.0F - right_volume);
			*out_right = (*out_right != 0.0F ? (*out_right * 0.5F) + (right_value * 0.5F) : right_value) * right_volume;
		};
	};
	if (*out_left < -1.0F) *out_left = -1.0F;
	else if (*out_left > 1.0F) *out_left = 1.0F;
	if (*out_right < -1.0F) *out_right = -1.0F;
	else if (*out_right > 1.0F) *out_right = 1.0F;
};

static float RGSRenderAudioCrunchBits(float in_value) {
	const float bit_factor = (float)(((1U << g_bits) >> 1U) - 1U);
	return floorf(in_value * bit_factor) / bit_factor;
};

static void RGSRenderAudio32(size_t in_channels, size_t in_frames, float* out_buffer) {
	for (size_t frame_index = 0U; frame_index < in_frames; frame_index++) {
		float left_value = 0.0F, right_value = 0.0F;
		RGSRenderAudioSampleActions((float)(frame_index) / (float)(g_samples), g_acount, &left_value, &right_value);
		if (in_channels == 1U) *(out_buffer++) = RGSRenderAudioCrunchBits((left_value * 0.5F) + (right_value * 0.5F));
		else if (in_channels == 2U) {
			if (g_stereo) {
				*(out_buffer++) = RGSRenderAudioCrunchBits(left_value);
				*(out_buffer++) = RGSRenderAudioCrunchBits(right_value);
			}
			else {
				left_value = RGSRenderAudioCrunchBits((left_value * 0.5F) + (right_value * 0.5F));
				*(out_buffer++) = left_value;
				*(out_buffer++) = left_value;
			};
		};
	};
};

void RGSRenderAudio() {
	const RGSTime current_time = RGSTimeNow();
	const RGSTime elapsed_time = current_time - g_rendered;
	if (elapsed_time >= 100ULL) {
		if (RGSCheckAudio()) {
#if RGS_OS == RGS_OS_WINDOWS
			UINT32 frame_padding = 0U;
			UINT32 frame_count = 0U;
			BYTE* buffer_data = RGS_NULL;
			if (g_client->lpVtbl->GetCurrentPadding(g_client, &frame_padding) != S_OK ||
				g_renderer->lpVtbl->GetBuffer(g_renderer, frame_count = (g_fcount - frame_padding), &buffer_data) != S_OK) {
				g_rendered = current_time;
				return;
			};
#endif
			size_t action_index = 0U;
			RGSActivateLock(RGS_LOCK_PASS(g_lock));
#if RGS_OS == RGS_OS_WINDOWS
			if (g_format->Format.wBitsPerSample == 32U) RGSRenderAudio32((size_t)(g_format->Format.nChannels), (size_t)(frame_count), (float*)(buffer_data));
#endif
			while (action_index < g_acount) {
				g_alist[action_index].offset += (1.0F / (float)(g_samples)) * (float)(frame_count) * g_alist[action_index].speed;
				if (g_alist[action_index].offset >= g_alist[action_index].length) RGSRemoveAudioAction(action_index);
				else action_index++;
			};
			RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
#if RGS_OS == RGS_OS_WINDOWS
			g_renderer->lpVtbl->ReleaseBuffer(g_renderer, frame_count, 0UL);
#endif
		}
		else {
			size_t action_index = 0U;
			RGSActivateLock(RGS_LOCK_PASS(g_lock));
			while (action_index < g_acount) {
				g_alist[action_index].offset += ((1.0F / RGS_ONE_SECOND) * (float)(elapsed_time) * g_alist[action_index].speed);
				if (g_alist[action_index].offset >= g_alist[action_index].length) RGSRemoveAudioAction(action_index);
				else action_index++;
			};
			RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
		};
		g_rendered = current_time;
	};
};


void RGSLockAudio() {
	RGSActivateLock(RGS_LOCK_PASS(g_lock));
	g_modifying = true;
};

void RGSUnlockAudio() {
	g_modifying = false;
	RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
};


bool RGSAudioThreaded() { return g_thread != RGS_THREAD_INVALID; };

bool RGSAudioRunning() { return g_running; };


/// Exposed Audio Functions

RGSInstrument RGSGetInstrument(uint8_t in_index) { return (g_modifying && g_ilist) ? g_ilist[(size_t)(in_index) % g_icount] : RGS_NULL; };

void RGSSetInstrument(uint8_t in_index, RGSInstrument in_instrument) { if (g_modifying && g_ilist) g_ilist[(size_t)(in_index) % g_icount] = in_instrument; };


void RGSPlayNote(uint8_t in_instrument, uint8_t in_note, float in_speed, float in_volume, float in_position) {
	if (!g_modifying) return;
	if (g_ilist && in_speed > 0.0F && in_volume > 0.0F) {
		RGSInstrument instrument_function = g_ilist[(size_t)(in_instrument) % g_icount];
		if (!instrument_function) return;
		const RGSAudioAction action_info = {
			.type = RGS_AUDIO_ACTION_TYPE_NOTE,
			.offset = 0.0F,
			.length = 1.0F,
			.speed = in_speed,
			.volume = in_volume >= 1.0F ? 1.0F : in_volume,
			.position = (float)(roundf((in_position >= 0.0F ? (in_position >= 1.0F ? 1.0F : in_position) : (in_position <= -1.0F ? -1.0F : in_position)) * 10.0F)) / 10.0F,
			.note = { instrument_function, in_note }
		};
		RGSAddAudioAction(&action_info);
	};
};

