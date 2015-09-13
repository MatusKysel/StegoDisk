//
//  Key.cpp
//  StegoDisk
//
//  Created by Martin Kosdy on 4/14/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#include <string.h> // memset

#include "Keys/Key.h"
#include "Utils/StegoHeader.h"
#include "Hash/Hash.h"

//#define DEFAULT_KEY_SIZE        32

namespace StegoDisk {

Key::Key() : data(0)
{
}
    
Key::Key(const MemoryBuffer& dataBuffer) : data(dataBuffer)
{
}
    
Key::~Key()
{
}
    
Key& Key::operator^=(const Key& other)
{
    data ^= other.data;
    return *this;
}
    
Key Key::operator^(const Key& other)
{
    return Key(data ^ other.data);
}
    
std::size_t Key::getSize()
{
    return data.getSize();
}

Key Key::fromString(std::string input)
{
    return Key(Hash(input).getState());
}
    
}

