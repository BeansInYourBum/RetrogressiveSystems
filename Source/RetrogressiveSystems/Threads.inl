#pragma once
#ifndef RETROGRESSIVESYSTEMS_THREADS_INL
#define RETROGRESSIVESYSTEMS_THREADS_INL


#include "./Platform.inl"


#if RGS_OS == RGS_OS_WINDOWS
#define RGS_LOCK_INVALID															-1L										// Invalid Lock Value
#define RGS_THREAD_INVALID															NULL									// Invalid Thread Value


#define RGSLock volatile LONG																								// Lock Type
#define RGSLockParameter RGSLock*																							// Lock Parameter Type
typedef HANDLE RGSThread;																									// Thread Type
typedef RGSThread RGSThreadParameter;																						// Thread Parameter Type
#endif

typedef void(*RGSThreadJob)(void*);																							// Thread Job Type

#if RGS_OS == RGS_OS_WINDOWS
/// @brief Passes the given lock to a function
/// @param _LOCK
/// @return Lock parameter
#define RGS_LOCK_PASS(_LOCK) (&_LOCK)

/// @brief Passes the given thread to a function
/// @param 
#define RGS_THREAD_PASS(_THREAD) (_THREAD)


/// @brief Creates a new lock object with the specified starting state
/// @param _LOCKED
/// @return Lock object
#define RGSCreateLock(_LOCKED) ((LONG)(_LOCKED))

/// @brief Destroys the given lock object
#define RGSDestroyLock(_LOCK)
#endif


/// @brief Activates the given lock so other threads can't use some resources
/// @param in_lock 
extern void RGSActivateLock(RGSLockParameter in_lock);

/// @brief Deactivates the given lock so other threads can use some resources
/// @param in_lock 
extern void RGSDeactivateLock(RGSLockParameter in_lock);


/// @brief Creates a new thread with the given job and parameters
/// @param in_job 
/// @param inout_lock 
/// @param inout_parameters 
/// @return Thread object
extern RGSThread RGSCreateThread(RGSThreadJob in_job, RGSLockParameter inout_lock, void* inout_parameters);

/// @brief Destroys the given thread object
/// @param in_thread 
extern void RGSDestroyThread(RGSThreadParameter in_thread);


/// @brief Waits for the given thread to finish
/// @param in_thread 
extern void RGSWaitForThread(RGSThreadParameter in_thread);


#endif

