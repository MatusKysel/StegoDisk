#ifndef STEGOHEADER_H
#define STEGOHEADER_H

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

//#include <cstddef>
#include "Utils/StegoTypes.h"
#include "Utils/Config.h"
#include "Logging/logger.h"

#endif // STEGOHEADER_H
