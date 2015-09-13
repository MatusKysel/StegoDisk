#ifndef VIRTUALSTORAGE_H
#define VIRTUALSTORAGE_H

#include "Utils/StegoTypes.h"
#include "Permutations/PermutationFactory.h"
#include "Keys/Key.h"
#include <memory>

namespace StegoDisk {

#ifndef __SHARED_PTR_VIRTUALSTORAGE__
#define __SHARED_PTR_VIRTUALSTORAGE__
class VirtualStorage;
typedef std::shared_ptr<VirtualStorage> VirtualStoragePtr;
#endif // __SHARED_PTR_VIRTUALSTORAGE__

/**
 * Main storage buffer that utilizes global permutation in readByte/writeByte ops
 *
 * [ STORAGE (len: usable capacity) | CHECKSUM/HASH (len: hash length) ]
*/

class VirtualStorage
{
private:
    void init();

public:
    VirtualStorage();
    ~VirtualStorage();

    static VirtualStoragePtr getNewInstance();
    static VirtualStoragePtr getNewInstance(string permutation);

    // Initialization of the VirtualStorage depends on permutation
    void setPermutation(PermutationPtr permutation);
    void unsetPermutation();
    void applyPermutation(uint64 requestedSize, Key key);

    // Accessed by CarrierFile during save/load operation
    void writeByte(uint64 position, uint8 value);
    uint8 readByte(uint64 position);

    // Accessed by main I/O layer (Fuse, VirtualDisc driver..)
    void read(uint64 offset, std::size_t length, uint8* buffer) const;
    void write(uint64 offset, std::size_t length, const uint8* buffer);

    uint64 getRawCapacity();
    uint64 getUsableCapacity();

    void randomizeBuffer();
    void clearBuffer();
    void fillBuffer(uint8 value);

    bool isValidChecksum();
    void writeChecksum();

private:
    PermutationPtr _globalPermutation;
    bool   _isSetGlobalPermutation;
    uint64 _rawCapacity;                // raw capacity (hash + storage)
    uint64 _usableCapacity;             // usable capacity (storage only)

    MemoryBuffer data;
};
    
}

#endif // VIRTUALSTORAGE_H
