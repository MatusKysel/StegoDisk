//
//  Hash.cpp
//  StegoDiskCore
//
//  Created by Martin Kosdy on 14/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#include "Hash.h"
#include <stdexcept>
#include "KeccakHashImpl.h"
#include "Utils/StegoTypes.h"
#include "HashImpl.h"

namespace StegoDisk {

std::unique_ptr<HashImpl> Hash::defaultHashImpl = std::unique_ptr<HashImpl>(new KeccakHashImpl());
    

void Hash::init()
{
    if (defaultHashImpl == nullptr)
        throw std::runtime_error("Hash: default hash implementation not set");
        
    state.resize(defaultHashImpl->getStateSize());
    state.clear();
}

Hash::Hash()
{
	init();
}


Hash::Hash(const std::string& data)
{
	init();
    process(data);
}

Hash::Hash(const uint8* data, std::size_t length)
{
	init();
    process(data, length);
}
    
void Hash::process(const std::string& data)
{
    if (defaultHashImpl == nullptr)
        throw std::runtime_error("Hash: default hash implementation not set");
    
    defaultHashImpl->process(state, (uint8*)data.c_str(), data.length());
}

void Hash::process(const MemoryBuffer& data)
{
    if (defaultHashImpl == nullptr)
        throw std::runtime_error("Hash: default hash implementation not set");
    
    if (data.getSize() == 0)
        throw std::invalid_argument("Hash: input data cannot be empty");
    
    defaultHashImpl->process(state, data.getConstRawPointer(), data.getSize());
}
    
void Hash::process(const uint8* data, std::size_t length)
{
    if (defaultHashImpl == nullptr)
        throw std::runtime_error("Hash: default hash implementation not set");
    
    defaultHashImpl->process(state, data, length);
}

void Hash::append(const std::string& data)
{
    if (defaultHashImpl == nullptr)
        throw std::runtime_error("Hash: default hash implementation not set");
    
    defaultHashImpl->append(state, (uint8*)data.c_str(), data.length());
}

void Hash::append(const uint8* data, std::size_t length)
{
    if (defaultHashImpl == nullptr)
        throw std::runtime_error("Hash: default hash implementation not set");
    
    defaultHashImpl->append(state, data, length);
}

void Hash::append(const MemoryBuffer& data)
{
    if (defaultHashImpl == nullptr)
        throw std::runtime_error("Hash: default hash implementation not set");
        
    defaultHashImpl->append(state, data.getConstRawPointer(), data.getSize());
}
    
const MemoryBuffer& Hash::getState() const
{
    return state;
}

std::size_t Hash::getStateSize() const
{
    return state.getSize();
}



}