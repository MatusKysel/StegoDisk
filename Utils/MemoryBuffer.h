#ifndef MEMORY_BUFFER_H
#define MEMORY_BUFFER_H

#include "Utils/StegoTypes.h"
#include <string>

namespace StegoDisk {

class MemoryBuffer
{
public:
    MemoryBuffer();
    MemoryBuffer(std::size_t size);
    MemoryBuffer(const uint8* data, std::size_t length);
    
    MemoryBuffer(const MemoryBuffer& other); // copy constructor
    MemoryBuffer(MemoryBuffer&& other); // move constructor
    
    MemoryBuffer& operator=(const MemoryBuffer& other); // copy assignment
    MemoryBuffer& operator=(MemoryBuffer&& other); // move assignment
    
    bool operator==(const MemoryBuffer& other);
    bool operator!=(const MemoryBuffer& other);
    
    ~MemoryBuffer();

    uint8& operator[](std::size_t index);
    const uint8& operator[](std::size_t index) const;
    
    MemoryBuffer& operator^=(const MemoryBuffer& other);
    MemoryBuffer operator^(const MemoryBuffer& other);
    
    void write(std::size_t offset, const uint8* data, std::size_t length);
    
    std::size_t getSize() const;
    void resize(std::size_t newSize);
    uint8* getRawPointer();
    const uint8* getConstRawPointer() const;
    
    void clear(); // set content to zero
    void randomize(); // replace content by random data
    void fill(uint8 value); // fill entire buffer with value

private:
	void destroy();
	void init(std::size_t newSize);

    uint8* buffer;
    std::size_t size;
};

}

#endif // MEMORY_BUFFER_H
