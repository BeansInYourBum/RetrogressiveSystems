#pragma once
#ifndef RETROGRESSIVESYSTEMS_OUTPUT_H
#define RETROGRESSIVESYSTEMS_OUTPUT_H


#include <RetrogressiveSystems/Types.h>


/// @brief Sends the given warning to the logger
/// @param in_sender 
/// @param in_message 
RGS_EXTERN void RGSReportWarning(const char* in_sender, const char* in_message);

/// @brief Sends the given error to the logger
/// @param in_sender 
/// @param in_message 
/// @param in_fatal 
RGS_EXTERN void RGSReportError(const char* const in_sender, const char* in_message, bool in_fatal);


#endif

