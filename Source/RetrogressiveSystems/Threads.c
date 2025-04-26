#include "./Threads.inl"


/// Internal Thread Types

typedef struct RGSThreadLaunchParameters {
	RGSThreadJob job;
	void* parameters;
} RGSThreadLaunchParameters;


/// Internal Thread Functions

void RGSActivateLock(RGSLockParameter in_lock) {
#if RGS_OS == RGS_OS_WINDOWS
	while (InterlockedIncrement(in_lock) > 1L) {
		InterlockedDecrement(in_lock);
		Sleep(0UL);
	};
#endif
};

void RGSDeactivateLock(RGSLockParameter in_lock) {
#if RGS_OS == RGS_OS_WINDOWS
	InterlockedDecrement(in_lock);
#endif
};


#if RGS_OS == RGS_OS_WINDOWS
static DWORD WINAPI RGSThreadJobRunner(LPVOID inout_parameters) {
	RGSThreadLaunchParameters* launch_parameters = (RGSThreadLaunchParameters*)(inout_parameters);
	launch_parameters->job(launch_parameters->parameters);
	return 0UL;
};
#endif

RGSThread RGSCreateThread(RGSThreadJob in_job, RGSLockParameter inout_lock, void* inout_parameters) {
	RGSThreadLaunchParameters launch_parameters = { in_job, inout_parameters };
#if RGS_OS == RGS_OS_WINDOWS
	RGSThread launched_thread = CreateThread(NULL, 0U, &RGSThreadJobRunner, (LPVOID)(&launch_parameters), 0UL, NULL);
	while (InterlockedIncrement(inout_lock) > 1L) {
		InterlockedDecrement(inout_lock);
		Sleep(0UL);
	};
	InterlockedDecrement(inout_lock);
#endif
	return launched_thread;
};

void RGSDestroyThread(RGSThreadParameter in_thread) {
#if RGS_OS == RGS_OS_WINDOWS
	CloseHandle(in_thread);
#endif
};


void RGSWaitForThread(RGSThreadParameter in_thread) {
#if RGS_OS == RGS_OS_WINDOWS
	WaitForSingleObject(in_thread, INFINITE);
#endif
};

