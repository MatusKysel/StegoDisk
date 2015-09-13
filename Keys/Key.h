//
//  Key.h
//  StegoDisk
//
//  Created by Martin Kosdy on 4/14/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#ifndef __StegoDisk__Key__
#define __StegoDisk__Key__

#include <string>
#include <memory>
#include "Utils/MemoryBuffer.h"

namespace StegoDisk {
    
class Key {
    
public:
    Key();
    Key(const MemoryBuffer& data);
    ~Key();
    
    std::size_t getSize();
    const MemoryBuffer& getData() { return data; };
    
    static Key fromString(std::string input);
    
    // XOR operators
    Key& operator^=(const Key& other);
    Key operator^(const Key& other);
    
private:
    MemoryBuffer data;
    
};
    
}

#endif /* defined(__StegoDisk__Key__) */
