//
//  KeccakHashImpl.h
//  StegoDiskCore
//
//  Created by Martin Kosdy on 14/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#ifndef __StegoDiskCore__KeccakHashImpl__
#define __StegoDiskCore__KeccakHashImpl__

#include "Utils/StegoHeader.h"
#include "Utils/MemoryBuffer.h"
#include "HashImpl.h"
#include <memory>

namespace StegoDisk {
    
class KeccakHashImpl : public HashImpl {
    
public:
    KeccakHashImpl(std::size_t stateSize = 32);
    virtual ~KeccakHashImpl();
    
    virtual void process(MemoryBuffer& state, const uint8* data, std::size_t length);
};

    
}


#endif /* defined(__StegoDiskCore__KeccakHashImpl__) */
