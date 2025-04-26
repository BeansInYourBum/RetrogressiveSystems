#pragma once
#ifndef RETROGRESSIVESYSTEMS_TYPES_H
#define RETROGRESSIVESYSTEMS_TYPES_H


#include <RetrogressiveSystems/Details.h>


#if RGS_LANGUAGE == RGS_LANGUAGE_C
#include <stdbool.h>
#include <stdint.h>
#else
#include <cstdint>
#endif


#define RGS_ONE_SECOND																1000000									// One Second Duration Value


typedef uint64_t RGSTime;																									// Time Point/Duration Type


/// @brief Acquires the current time
/// @return Time point
RGS_EXTERN RGSTime RGSTimeNow();


#endif

