#include <RetrogressiveSystems/Types.h>


#include "./Platform.inl"


/// Exposed Time Functions


RGSTime RGSTimeNow() {
#if RGS_OS == RGS_OS_WINDOWS
	static double frequency_value = -1.00;
	if (frequency_value <= 0.00) {
		LARGE_INTEGER time_frequency;
		if (!QueryPerformanceFrequency(&time_frequency)) return UINT64_C(0);
		frequency_value = (double)(time_frequency.QuadPart);
	};
	LARGE_INTEGER time_counter;
	if (!QueryPerformanceCounter(&time_counter)) return UINT64_C(0);
	return (RGSTime)(((double)(time_counter.QuadPart) / frequency_value) * (double)(RGS_ONE_SECOND));
#endif
};

