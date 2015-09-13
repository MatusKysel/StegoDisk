#include "MemoryBuffer.h"
#include "Utils/StegoTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <ctime>

using namespace std;

namespace StegoDisk {

void MemoryBuffer::init(std::size_t newSize)
{
	this->size = newSize;
    if (size > 0) {
        buffer = new uint8[size];
    } else {
		size = 0;
        buffer = nullptr;
    }
}

MemoryBuffer::MemoryBuffer() : buffer(nullptr), size(0)
{
}


MemoryBuffer::MemoryBuffer(std::size_t newSize) : size(newSize)
{
	init( newSize );
}

MemoryBuffer::MemoryBuffer(const uint8* data, std::size_t length)
{
	init( length );
    if (size == 0)
        return;
    
    memcpy(buffer, data, size);
}
    
/*
 * copy constructor
 */
MemoryBuffer::MemoryBuffer(const MemoryBuffer& other)
{
	init( other.size );
    if (size == 0)
        return;

    memcpy(buffer, other.buffer, size);
}

/*
 * move constructor
 * other buffer is not destroyed - data are moved to new buffer
 */
MemoryBuffer::MemoryBuffer(MemoryBuffer&& other)
{
	if ( buffer != nullptr )
		destroy();

    buffer = other.buffer;
    size = other.size;
    
    other.buffer = nullptr;
    other.size = 0;
}
    
// copy assignment
MemoryBuffer& MemoryBuffer::operator=(const MemoryBuffer& other)
{
	// self-assignment check expected
	if ( this == &other )
		return *this;

    if (size != other.size)
    {
		destroy();
		init(other.size);
    }
    memcpy(buffer, other.buffer, size);
    return *this;
}
    
// move assignment
MemoryBuffer& MemoryBuffer::operator=(MemoryBuffer&& other)
{
    destroy();
    buffer = other.buffer;
    size = other.size;
    other.size = 0;
    other.buffer = nullptr;
    return *this;
}
    
MemoryBuffer::~MemoryBuffer()
{
    destroy();
}
    
bool MemoryBuffer::operator==(const MemoryBuffer& other)
{
    if (size != other.size)
        return false;
    
    if (size == 0)
        return true;
    
    if ((other.buffer == nullptr) || (buffer == nullptr))
        return false;
    
    return !(memcmp(buffer, other.buffer, size));
}
    
bool MemoryBuffer::operator!=(const MemoryBuffer& other)
{
    return !(*this == other);
}


void MemoryBuffer::resize(std::size_t newSize)
{
    if (newSize == size)
        return;

    uint8* originalBuffer = buffer;
    std::size_t originalSize = size;

    uint8* newBuffer = new uint8[newSize];
    size = newSize;

	memset(newBuffer, 0, newSize);
    if (originalBuffer != nullptr) {		
        memcpy(newBuffer, originalBuffer, min(newSize, originalSize));
		randomize();
        delete[] originalBuffer;
    }

    buffer = newBuffer;
}

uint8& MemoryBuffer::operator[](std::size_t index)
{
    if (index >= size)
        throw std::out_of_range("MemoryBuffer: index out of range");
    
    return buffer[index];
}

const uint8& MemoryBuffer::operator[](std::size_t index) const
{
    if (index >= size)
        throw std::out_of_range("MemoryBuffer: index out of range");
    
    return buffer[index];
}
    
MemoryBuffer& MemoryBuffer::operator^=(const MemoryBuffer& other)
{
    if (size != other.size)
        throw std::length_error("MemoryBuffer: size must be equal to use xor operator");
 
    for (size_t i=0; i<size; i++)
        (*this)[i] ^= other[i];
    
    return *this;
}

MemoryBuffer MemoryBuffer::operator^(const MemoryBuffer& other)
{
    if (size != other.size)
        throw std::length_error("MemoryBuffer: size must be equal to use xor operator");
 
    MemoryBuffer result(*this);
    result^=other;
    return result;
}

std::size_t MemoryBuffer::getSize() const
{
    return size;
}

uint8* MemoryBuffer::getRawPointer()
{
    return buffer;
}
    
const uint8* MemoryBuffer::getConstRawPointer() const
{
    return buffer;
}

void MemoryBuffer::write(std::size_t offset, const uint8* data, std::size_t length)
{
    if ((offset + length) > size)
        throw  std::out_of_range("MemoryBuffer: write: offset + length > size");
    
    memcpy((void*)(buffer + offset), data, length);
}
    
    
void MemoryBuffer::clear()
{
    if ((buffer == nullptr) || (size == 0))
        return;

    memset(buffer, 0, size);
}

void MemoryBuffer::randomize()
{
    if ((buffer == nullptr) || (size == 0))
        return;

    memset(buffer, 0, size);
    memset(buffer, 0xFF, size);

    srand((unsigned int)time(NULL));
    for (std::size_t i=0; i<size; i++) {
        buffer[i] = (uint8)rand();
    }
}
    
void MemoryBuffer::fill(uint8 value)
{
    if ((buffer == nullptr) || (size == 0))
        return;
    
    memset(buffer, value, size);
}
    
void MemoryBuffer::destroy()
{
	if ( buffer == nullptr )
		return;

    //TODO: instead of clearing rewrite buffer by random data
    //memset(buffer, 0, size);
	randomize();
    delete[] buffer;        
	buffer = nullptr;
    size = 0;
}

}
