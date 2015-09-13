//
//  Hash.h
//  StegoDiskCore
//
//  Created by Martin Kosdy on 14/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#ifndef __StegoDiskCore__Hash__
#define __StegoDiskCore__Hash__

#include "Utils/StegoHeader.h"
#include "Utils/MemoryBuffer.h"
#include "Utils/Cpp11Compat.h"
#include "HashImpl.h"
#include <memory>

namespace StegoDisk {
    
class Hash KEYWORD_FINAL
{
public:
    Hash();
    Hash(const std::string& data);
    Hash(const uint8* data, std::size_t length);
    
    void process(const std::string& data);
    void process(const MemoryBuffer& data);
    void process(const uint8* data, std::size_t length);
    
    void append(const std::string& data);
    void append(const MemoryBuffer& data);
    void append(const uint8* data, std::size_t length);
    const MemoryBuffer& getState() const;
    std::size_t getStateSize() const;
    
private:
	void init();
    MemoryBuffer state;
    
//TODO: implementation could be stored as shared ptr in each hash instance
//  std::shared_ptr<HashImpl> hashImpl;
    
// static stuff
public:
    static void setDefaultHashImpl(std::unique_ptr<HashImpl> newHashImpl);
private:
    static std::unique_ptr<HashImpl> defaultHashImpl;

};
    
    
}

#endif /* defined(__StegoDiskCore__Hash__) */
