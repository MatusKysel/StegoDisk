#ifndef STEGODISK_UTILS_STEGOHEADER_H_
#define STEGODISK_UTILS_STEGOHEADER_H_

#if defined(unix) || defined(__APPLE__)
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
#include "logging/logger.h" //TODO(Matus) aby som nemusel ist o uroven vyssie

#endif // STEGODISK_UTILS_STEGO_HEADER_H_
