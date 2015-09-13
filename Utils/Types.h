//
//  StegoTypes.h
//  StegoFS
//
//  Created by Martin Kosdy on 4/7/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef StegoFS_Types_h
#define StegoFS_Types_h

#ifndef __StegoFS__UINT64__
#define __StegoFS__UINT64__
//#if ((UINT_FAST64_MAX >> 63) != 1)
#if ((UINT64_MAX >> 63) != 1)
#error "There is no 64-bit unsigned type. See if enabled compilation flag '--std=c++11' or '--std=g++11'"
#endif
typedef uint64_t _uint64;
#endif // __StegoFS__UINT64__

#ifndef __StegoFS__UINT32__
#define __StegoFS__UINT32__
#if ((UINT32_MAX >> 31) != 1)
#error "There is no 64-bit unsigned type. See if enabled compilation flag 'c++11' or 'g++11'"
#endif
typedef uint32_t _uint32;
#endif // __StegoFS__UINT32__

#ifndef __StegoFS__UINT8__
#define __StegoFS__UINT8__
#if ((UINT8_MAX >> 7) != 1)
#error "There is no 64-bit unsigned type. See if enabled compilation flag 'c++11' or 'g++11'"
#endif
typedef uint8_t _uint8;
#endif // __StegoFS__UINT8__

#endif // StegoFS_Types_h
