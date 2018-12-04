/**
* @file stego_header.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Header with basic includes
*
*/

#pragma once

#if defined(__unix__) || defined(__APPLE__)
#define STEGO_OS_UNIX   1
#ifdef __APPLE__
#define STEGO_OS_MAC
#endif
#endif

#ifdef _WIN32
#define STEGO_OS_WIN
#ifdef _WIN64
#define STEGO_OS_WIN64
#else
#define STEGO_OS_WIN32
#endif
#endif

#include "stego_types.h"
#include "config.h"
#include "logging/logger.h"