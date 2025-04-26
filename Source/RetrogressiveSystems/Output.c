#include "./Output.inl"


#include "./Threads.inl"


#if RGS_OS == RGS_OS_WINDOWS
#include <combaseapi.h>
#endif


#define RGS_LOG_CAPACITY 1024U


/// Internal Output Variables

static RGSLock g_lock = RGS_LOCK_INVALID;
static bool g_safe = false;

#if RGS_OS == RGS_OS_WINDOWS
static HANDLE g_file = NULL;
#if RGS_BUILD == RGS_BUILD_DEBUG
static HANDLE g_console = NULL;
#endif
bool g_initialised = false;
#endif


/// Internal Output Functions

static void RGSLog(
#if RGS_OS == RGS_OS_WINDOWS
	WORD in_attributes,
#endif
	const char* in_type, const char* in_sender, const char* in_message, bool in_unsafe
) {
	size_t log_size = 0U;
	char log_content[RGS_LOG_CAPACITY + 2U];
	char* destination_pointer = log_content;
	const char* source_pointer = in_type;
	while (*source_pointer && log_size++ < RGS_LOG_CAPACITY) *(destination_pointer++) = *(source_pointer++);
	source_pointer = "\nSender: ";
	while (*source_pointer && log_size++ < RGS_LOG_CAPACITY) *(destination_pointer++) = *(source_pointer++);
	source_pointer = in_sender && *in_sender ? in_sender : "Unspecified";
	while (*source_pointer && log_size++ < RGS_LOG_CAPACITY) *(destination_pointer++) = *(source_pointer++);
	source_pointer = "\nMessage: ";
	while (*source_pointer && log_size++ < RGS_LOG_CAPACITY) *(destination_pointer++) = *(source_pointer++);
	source_pointer = in_message;
	while (*source_pointer && log_size++ < RGS_LOG_CAPACITY) *(destination_pointer++) = *(source_pointer++);
	*(destination_pointer++) = '\n';
	*destination_pointer = '\0';
	log_size++;
	if (g_lock != RGS_LOCK_INVALID) RGSActivateLock(RGS_LOCK_PASS(g_lock));
#if RGS_OS == RGS_OS_WINDOWS
#if RGS_BUILD == RGS_BUILD_DEBUG
	if (g_console && g_console != INVALID_HANDLE_VALUE) {
		CONSOLE_SCREEN_BUFFER_INFO buffer_info;
		BOOL changed_attributes = GetConsoleScreenBufferInfo(g_console, &buffer_info);
		if (changed_attributes) changed_attributes = SetConsoleTextAttribute(g_console, in_attributes);
		WriteConsoleA(g_console, log_content, (DWORD)(log_size), NULL, NULL);
		if (changed_attributes) SetConsoleTextAttribute(g_console, buffer_info.wAttributes);
	};
#endif
	if (!g_file) g_file = CreateFileA("./Log.txt", GENERIC_WRITE, 0U, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (g_file) WriteFile(g_file, log_content, (DWORD)(log_size), NULL, NULL);
	if (in_unsafe) g_safe = false;
#endif
	if (g_lock != RGS_LOCK_INVALID) RGSDeactivateLock(RGS_LOCK_PASS(g_lock));
};


bool RGSPrepareOutput() {
#if RGS_OS == RGS_OS_WINDOWS
#if RGS_BUILD == RGS_BUILD_DEBUG
	g_console = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!g_console || g_console == INVALID_HANDLE_VALUE) {
		RGSLog(FOREGROUND_RED | FOREGROUND_INTENSITY, "--Fatal Error", "Output", "Failed to acquire console", true);
		RGSReleaseOutput();
		return false;
	};
#endif
	if (CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK) {
		RGSLog(FOREGROUND_RED | FOREGROUND_INTENSITY, "--Fatal Error", "Output", "Failed to initialise COM library", true);
		RGSReleaseOutput();
		return false;
	};
	g_initialised = true;
#endif
	g_lock = RGSCreateLock(false);
	if (g_lock == RGS_LOCK_INVALID) {
		RGSLog(FOREGROUND_RED | FOREGROUND_INTENSITY, "--Fatal Error", "Output", "Failed to create lock", true);
		RGSReleaseOutput();
		return false;
	};
	g_safe = true;
	return true;
};

void RGSReleaseOutput() {
	g_safe = false;
#if RGS_OS == RGS_OS_WINDOWS
	if (g_file) CloseHandle(g_file);
	if (g_initialised) CoUninitialize();
#endif
	if (g_lock != RGS_LOCK_INVALID) RGSDestroyLock(RGS_LOCK_PASS(g_lock));
};


bool RGSSafe() { return g_safe; };


/// Exposed Output Functions

void RGSReportWarning(const char* in_sender, const char* in_message) {
	RGSLog(
#if RGS_OS == RGS_OS_WINDOWS
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
#endif
		"--Warning", in_sender, in_message && *in_message ?in_message : "An unknown warning has been reported", false);
};

void RGSReportError(const char* const in_sender, const char* in_message, bool in_fatal) {
	RGSLog(
#if RGS_OS == RGS_OS_WINDOWS
		FOREGROUND_RED | FOREGROUND_INTENSITY,
#endif
		in_fatal ? "--Fatal Error" : "--Error", in_sender, in_message && *in_message ? in_message : "An unknown error has been reported", in_fatal);
};

