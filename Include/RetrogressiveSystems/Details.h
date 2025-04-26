#pragma once
#ifndef RETROGRESSIVESYSTEMS_DETAILS_H
#define RETROGRESSIVESYSTEMS_DETAILS_H


#define RGS_OS_WINDOWS																0x00010000								// Windows Operating System

#define RGS_DEVICE_COMPUTER															0										// Computer Device Type

#define RGS_BUILD_DEBUG																0										// Debug Build Mode
#define RGS_BUILD_RELEASE															1										// Release Build Mode

#define RGS_LANGUAGE_C																0										// C Programming Language
#define RGS_LANGUAGE_CPP															1										// C++ Programming Language


#if defined(_WIN32)
#define RGS_OS																		RGS_OS_WINDOWS							// Target Operating System (Windows)
#define RGS_DEVICE																	RGS_DEVICE_COMPUTER						// Target Device Type (Computer)
#else
#error Unsupported operating system
#endif

#ifdef _DEBUG
#define RGS_BUILD																	RGS_BUILD_DEBUG							// Active Build Mode (Debug)
#else
#define RGS_BUILD																	RGS_BUILD_RELEASE						// Active Build Mode (Release)
#endif

#ifndef __cplusplus
#define RGS_LANGUAGE																RGS_LANGUAGE_C							// Active Programming Language (C)
#define RGS_EXTERN																											// External Function Helper
#define RGS_NULL																	((void*)(0))							// Contant NULL Value
#else
#define RGS_LANGUAGE																RGS_LANGUAGE_CPP						// Active Programming Language (C++)
#define RGS_EXTERN																	extern "C"								// External Function Helper
#define RGS_NULL																	nullptr									// Constant NULL Value
#endif


#endif

