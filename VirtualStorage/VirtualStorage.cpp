#include <exception>
#include <time.h>

#include "VirtualStorage/VirtualStorage.h"
#include "Permutations/Permutation.h"
#include "Utils/StegoMath.h"
#include "Utils/StegoErrors.h"
#include "Utils/Config.h"
#include "Utils/Keccak/keccak.h"
#include "Logging/logger.h"
#include "Hash/Hash.h"

namespace StegoDisk {

void VirtualStorage::init()
{
    _rawCapacity = 0;
    _usableCapacity = 0;
    _isSetGlobalPermutation = false;
    _globalPermutation = PermutationPtr(nullptr);
}

VirtualStorage::VirtualStorage()
{
    init();
}

VirtualStorage::~VirtualStorage()
{
    init();
}

VirtualStoragePtr VirtualStorage::getNewInstance()
{
    VirtualStoragePtr virtualStorage = shared_ptr<VirtualStorage>(new VirtualStorage());
    virtualStorage->setPermutation(PermutationFactory::getDefaultPermutation());
    return virtualStorage;
}

VirtualStoragePtr VirtualStorage::getNewInstance(string permutation)
{
    VirtualStoragePtr virtualStorage = shared_ptr<VirtualStorage>(new VirtualStorage());
    try {
        virtualStorage->setPermutation(PermutationFactory::getPermutationByName(permutation));
    } catch (...) { throw; }

    return virtualStorage;
}

/**
 * @brief Usable capacity of the storage in bytes (excluding checksum at the end of the storage)
 *
 * @return the capacity
 */
uint64 VirtualStorage::getUsableCapacity()
{
    if (!_globalPermutation)
        throw std::invalid_argument("VirtualStorage::getUsableCapacity: permutation not set yet");
    if (!_isSetGlobalPermutation)
        throw std::invalid_argument("VirtualStorage::getUsableCapacity: permutation not applied yet");

    //return (_capacity - SFS_STORAGE_HASH_LENGTH);
    return _usableCapacity;
}


/**
 * @brief Raw capacity of the storage in bytes (including checksum at the end of the storage)
 *
 * @return the capacity
 */
uint64 VirtualStorage::getRawCapacity()
{
    if (!_globalPermutation)
        throw std::invalid_argument("VirtualStorage::getRawCapacity: permutation not set yet");
    if (!_isSetGlobalPermutation)
        throw std::invalid_argument("VirtualStorage::getRawCapacity: permutation not applied yet");

    return _rawCapacity;
}

void VirtualStorage::setPermutation(PermutationPtr permutation) 
{
    if (!permutation)
        throw std::invalid_argument("VirtualStorage::setPermutation: arg 'permutation' is nullptr");

    if (_globalPermutation)
        unsetPermutation();
    _globalPermutation = permutation;
}

void VirtualStorage::unsetPermutation()
{
    init();
}

/**
 * @brief Initializes storage
 *
 * This method allocates memory for virtual storage.
 * Size of the storage is determined by global permutation's capacity.
 * Global permutation is used by readByte/writeByte methods.
 *
 * @param[in] globalPermutation Correctly initialized permutation
 * @return Error code (NO ERROR)
 */
void VirtualStorage::applyPermutation(uint64 requestedSize, Key key)
{
    if ( !_globalPermutation )
        throw std::invalid_argument("VirtualStorage::applyPermutation: permutation not set yet");

    try { _globalPermutation->init(requestedSize, key); }
    catch (...) { throw; }

    if ( _globalPermutation->getSize() == 0 ) {
        string str = "VirtualStorage::applyPermutation: size of ";
        str += _globalPermutation->getNameInstance();
        str += " is initialized by requested size (";
		str += std::to_string(requestedSize);
        str += ") to zero";
        throw std::range_error(str);
    }

    uint64 rawCapacity = _globalPermutation->getSize();
    if (rawCapacity <= SFS_STORAGE_HASH_LENGTH)
        throw std::out_of_range("VirtualStorage::applyPermutation: capacity ot the storage is too low");

    data = MemoryBuffer(rawCapacity);
    
    _rawCapacity = rawCapacity;
    _usableCapacity = rawCapacity - SFS_STORAGE_HASH_LENGTH;
    _isSetGlobalPermutation = true;
}

/**
 * @brief Reads the value of one byte at permuted position
 *
 * This method is invoked in CarrierFile instances to read their own part of the storage.
 * Input position is modified by global permutation.
 *
 * @param[in]  position  offset of requested byte
 * @param[out] value     byte at position
 * @return Error code (0 = NO ERROR)
 */
    
uint8 VirtualStorage::readByte(uint64 position)
{
    if (position >= _rawCapacity)
        throw std::out_of_range("index out of range");
    
    if (!_globalPermutation)
        throw std::runtime_error("storage not initialized");

    return data[_globalPermutation->permute(position)];
}

/**
 * @brief Changes the value of one byte at permuted position
 *
 * This method is invoked in CarrierFile instances to write their own part of the storage.
 * Input position is modified by global permutation.
 *
 * @param[in] position  offset
 * @param[in] value     byte at position
 * @return Error code (0 = NO ERROR)
 */
void VirtualStorage::writeByte(uint64 position, uint8 value)
{
    if (position >= _rawCapacity)
        throw std::out_of_range("index out of range");
    
    if (!_globalPermutation)
        throw std::runtime_error("storage not initialized");

    data[_globalPermutation->permute(position)] = value;
}

/**
 * @brief Reads length bytes from offset to buffer
 *
 * Reads data from virtual storage.
 * Doesnt use global permutation.
 *
 * @param[in]  offset   the start position
 * @param[in]  length   the number of bytes to read
 * @param[out] buffer   output buffer
 */
void VirtualStorage::read(uint64 offset, std::size_t length, uint8* buffer) const
{
    if (offset+length > _usableCapacity)
        throw std::out_of_range("index out of range");

    if (length == 0)
        return;

    if (data.getConstRawPointer() == nullptr)
        throw std::out_of_range("storage not initialized");
    
    memcpy(buffer, (void*)(data.getConstRawPointer() + offset), length);
}

/**
 * @brief Writes length bytes located in buffer from offset
 *
 * Writes data to virtual storage.
 * Doesnt use global permutation.
 *
 * @param[in]  offset   the start position
 * @param[in]  length   the number of bytes to write
 * @param[in]  buffer   data input buffer
 */
void VirtualStorage::write(uint64 offset, std::size_t length, const uint8* buffer)
{
    if (offset+length > _usableCapacity)
        throw std::out_of_range("index out of range");

    if (length == 0)
        return;

    if (data.getConstRawPointer() == nullptr)
        throw std::out_of_range("storage not initialized");
    
    memcpy(data.getRawPointer() + offset, buffer, length);
}


/**
 * @brief Fills the storage with random data
 *
 * @return Error code (0 = NO ERROR)
 */
void VirtualStorage::randomizeBuffer()
{
    data.randomize();
}

/**
 * @brief Fills the storage with zeros
 *
 * @return Error code (0 = NO ERROR)
 */
void VirtualStorage::clearBuffer()
{
    data.clear();
}

/**
 * @brief Fills the storage with value
 *
 * @param value
 * @return
 */
void VirtualStorage::fillBuffer(uint8 value)
{
    data.fill(value);
}


/**
 * @brief Checks integrity of the storage using hash
 *
 * Compares checksum (hash) stored at the end of the storage
 *
 * @return Error code or 0 if integrity check succeeds
 */
bool VirtualStorage::isValidChecksum()
{
    if (data.getSize() == 0)
        throw std::invalid_argument("VirtualStorage::checkIntegrity: data storage is not set yet");
    
    if (_usableCapacity == 0) 
        throw std::invalid_argument("VirtualStorage::checkIntegrity: capacity storage is not initialized yet");

//TODO:    #warning Sync hash length with SFS_STORAGE_HASH_LENGTH
    Hash checksum(data.getConstRawPointer(), _usableCapacity);
    
    MemoryBuffer storedChecksum(data.getConstRawPointer() + _usableCapacity, SFS_STORAGE_HASH_LENGTH);

    LOG_DEBUG("VirtualStorage::isValidChecksum:   Stored CHECKSUM: " << StegoMath::hexBufferToStr(storedChecksum));
    LOG_DEBUG("VirtualStorage::isValidChecksum: Computed CHECKSUM: " << StegoMath::hexBufferToStr(checksum.getState()));
	LOG_TRACE("VirtualStorage::isValidChecksum: data (raw Capacity = " << _rawCapacity << "): " << StegoMath::hexBufferToStr(&data[0], (int)_rawCapacity));

    return (storedChecksum == checksum.getState());
}


/**
 * @brief Writes checksum of currently stored data at the end of the storage
 *
 */
void VirtualStorage::writeChecksum()
{
    if ((data.getSize() == 0) || (_usableCapacity == 0))
        throw runtime_error("storage not initialized");
    
//TODO:    #warning Sync hash length with SFS_STORAGE_HASH_LENGTH

    Hash checksum(data.getConstRawPointer(), _usableCapacity);
	
    data.write(_usableCapacity, checksum.getState().getConstRawPointer(), checksum.getStateSize());
	LOG_TRACE("VirtualStorage::isValidChecksum: data (raw Capacity = " << _rawCapacity << "): " << StegoMath::hexBufferToStr(&data[0], (int)_rawCapacity));
}

}