#pragma once
#ifndef RETROGRESSIVESYSTEMS_PLATFORM_INL
#define RETROGRESSIVESYSTEMS_PLATFORM_INL


#include <RetrogressiveSystems/Details.h>


#if RGS_OS == RGS_OS_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif


#endif

