//
//  HashImpl.h
//  StegoDiskCore
//
//  Created by Martin Kosdy on 14/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#ifndef __StegoDiskCore__HashImpl__
#define __StegoDiskCore__HashImpl__

#include "Utils/StegoHeader.h"
#include "Utils/MemoryBuffer.h"
#include <memory>

namespace StegoDisk {
    
class HashImpl {
    
public:
    HashImpl() : stateSize(0) {};
    virtual ~HashImpl() {};
    
    virtual void process(MemoryBuffer& state, const uint8* data, std::size_t length) = 0;
    virtual void append(MemoryBuffer& state, const uint8* data, std::size_t length);

    std::size_t getStateSize() { return stateSize; };
    
protected:
    size_t stateSize;
    
};
    
    
}

#endif /* defined(__StegoDiskCore__HashImpl__) */
