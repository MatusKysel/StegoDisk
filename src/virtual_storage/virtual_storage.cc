#include "virtual_storage.h"

#include <time.h>

#include <exception>

#include "permutations/permutation.h"
#include "utils/stego_math.h"
#include "utils/stego_errors.h"
#include "utils/config.h"
#include "utils/keccak/keccak.h"
#include "logging/logger.h"
#include "hash/hash.h"

namespace stego_disk {

void VirtualStorage::Init() {
  raw_capacity_ = 0;
  usable_capacity_ = 0;
  is_set_global_permutation_ = false;
  global_permutation_ = std::shared_ptr<Permutation>(nullptr);
}

VirtualStorage::VirtualStorage() {
  Init();
}

VirtualStorage::~VirtualStorage() {
  Init();
}

VirtualStoragePtr VirtualStorage::GetNewInstance() {
  VirtualStoragePtr virtual_storage =
      shared_ptr<VirtualStorage>(new VirtualStorage());
  virtual_storage->SetPermutation(PermutationFactory::GetDefaultPermutation());
  return virtual_storage;
}

VirtualStoragePtr VirtualStorage::GetNewInstance(string permutation) {
  VirtualStoragePtr virtual_storage =
      shared_ptr<VirtualStorage>(new VirtualStorage());
  try {
    virtual_storage->SetPermutation(
          PermutationFactory::GetPermutation(permutation));
  } catch (...) { throw; }

  return virtual_storage;
}

/**
 * @brief Usable capacity of the storage in bytes (excluding checksum at the end of the storage)
 *
 * @return the capacity
 */
uint64 VirtualStorage::GetUsableCapacity() {
  if (!global_permutation_)
    throw std::invalid_argument("VirtualStorage::GetUsableCapacity: "
                                "permutation not Set yet");
  if (!is_set_global_permutation_)
    throw std::invalid_argument("VirtualStorage::GetUsableCapacity: "
                                "permutation not applied yet");

  //return (_capacity - SFS_STORAGE_HASH_LENGTH);
  return usable_capacity_;
}


/**
 * @brief Raw capacity of the storage in bytes (including checksum at the end of the storage)
 *
 * @return the capacity
 */
uint64 VirtualStorage::GetRawCapacity() {
  if (!global_permutation_)
    throw std::invalid_argument("VirtualStorage::GetRawCapacity: "
                                "permutation not Set yet");
  if (!is_set_global_permutation_)
    throw std::invalid_argument("VirtualStorage::GetRawCapacity: "
                                "permutation not applied yet");

  return raw_capacity_;
}

void VirtualStorage::SetPermutation(std::shared_ptr<Permutation> permutation) {
  if (!permutation)
    throw std::invalid_argument("VirtualStorage::SetPermutation: "
                                "arg 'permutation' is nullptr");

  if (global_permutation_)
    UnSetPermutation();
  global_permutation_ = permutation;
}

void VirtualStorage::UnSetPermutation() {
  Init();
}

/**
 * @brief Initializes storage
 *
 * This method allocates memory for virtual storage.
 * Size of the storage is determined by global permutation's capacity.
 * Global permutation is used by readByte/writeByte methods.
 *
 * @param[in] globalPermutation Correctly Initialized permutation
 * @return Error code (NO ERROR)
 */
void VirtualStorage::ApplyPermutation(uint64 requested_size, Key key) {
  if ( !global_permutation_ )
    throw std::invalid_argument("VirtualStorage::applyPermutation: "
                                "permutation not Set yet");

  try { global_permutation_->Init(requested_size, key); }
  catch (...) { throw; }

  if ( global_permutation_->GetSize() == 0 ) {
    string str = "VirtualStorage::applyPermutation: size of ";
    str += global_permutation_->GetNameInstance();
    str += " is Initialized by requested size (";
    str += std::to_string(requested_size);
    str += ") to zero";
    throw std::range_error(str);
  }

  uint64 raw_capacity = global_permutation_->GetSize();
  if (raw_capacity <= SFS_STORAGE_HASH_LENGTH)
    throw std::out_of_range("VirtualStorage::applyPermutation: "
                            "capacity ot the storage is too low");

  data_ = MemoryBuffer(raw_capacity);

  raw_capacity_ = raw_capacity;
  usable_capacity_ = raw_capacity - SFS_STORAGE_HASH_LENGTH;
  is_set_global_permutation_ = true;
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

uint8 VirtualStorage::ReadByte(uint64 position) {
  if (position >= raw_capacity_)
    throw std::out_of_range("index out of range");

  if (!global_permutation_)
    throw std::runtime_error("storage not Initialized");

  return data_[global_permutation_->Permute(position)];
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
void VirtualStorage::WriteByte(uint64 position, uint8 value) {
  if (position >= raw_capacity_)
    throw std::out_of_range("index out of range");

  if (!global_permutation_)
    throw std::runtime_error("storage not Initialized");

  data_[global_permutation_->Permute(position)] = value;
}

/**
 * @brief Reads length bytes from offset to buffer
 *
 * Reads data_ from virtual storage.
 * Doesnt use global permutation.
 *
 * @param[in]  offset   the start position
 * @param[in]  length   the number of bytes to read
 * @param[out] buffer   output buffer
 */
void VirtualStorage::Read(uint64 offset,
                          std::size_t length, uint8* buffer) const {
  if (offset + length > usable_capacity_)
    throw std::out_of_range("index out of range");

  if (length == 0)
    return;

  if (data_.GetConstRawPointer() == nullptr)
    throw std::out_of_range("storage not Initialized");

  memcpy(buffer, (void*)(data_.GetConstRawPointer() + offset), length);
}

/**
 * @brief Writes length bytes located in buffer from offset
 *
 * Writes data_ to virtual storage.
 * Doesnt use global permutation.
 *
 * @param[in]  offset   the start position
 * @param[in]  length   the number of bytes to write
 * @param[in]  buffer   data_ input buffer
 */
void VirtualStorage::Write(uint64 offset,
                           std::size_t length, const uint8* buffer) {
  if (offset + length > usable_capacity_)
    throw std::out_of_range("index out of range");

  if (length == 0)
    return;

  if (data_.GetConstRawPointer() == nullptr)
    throw std::out_of_range("storage not Initialized");

  memcpy(data_.GetRawPointer() + offset, buffer, length);
}


/**
 * @brief Fills the storage with random data_
 *
 * @return Error code (0 = NO ERROR)
 */
void VirtualStorage::RandomizeBuffer() {
  data_.Randomize();
}

/**
 * @brief Fills the storage with zeros
 *
 * @return Error code (0 = NO ERROR)
 */
void VirtualStorage::ClearBuffer() {
  data_.Clear();
}

/**
 * @brief Fills the storage with value
 *
 * @param value
 * @return
 */
void VirtualStorage::FillBuffer(uint8 value) {
  data_.Fill(value);
}


/**
 * @brief Checks integrity of the storage using hash
 *
 * Compares checksum (hash) stored at the end of the storage
 *
 * @return Error code or 0 if integrity check succeeds
 */
bool VirtualStorage::IsValidChecksum() {
  if (data_.GetSize() == 0)
    throw std::invalid_argument("VirtualStorage::checkIntegrity: "
                                "data_ storage is not Set yet");

  if (usable_capacity_ == 0)
    throw std::invalid_argument("VirtualStorage::checkIntegrity: "
                                "capacity storage is not Initialized yet");

  //TODO:    #warning Sync hash length with SFS_STORAGE_HASH_LENGTH
  Hash checksum(data_.GetConstRawPointer(), usable_capacity_);

  MemoryBuffer stored_checksum(data_.GetConstRawPointer() + usable_capacity_,
                               SFS_STORAGE_HASH_LENGTH);

  LOG_DEBUG("VirtualStorage::isValidChecksum:   Stored CHECKSUM: "
            << StegoMath::HexBufferToStr(stored_checksum));
  LOG_DEBUG("VirtualStorage::isValidChecksum: Computed CHECKSUM: "
            << StegoMath::HexBufferToStr(checksum.GetState()));
  LOG_TRACE("VirtualStorage::isValidChecksum: data_ (raw Capacity = "
            << raw_capacity_ << "): "
            << StegoMath::HexBufferToStr(&data_[0],
            static_cast<int>(raw_capacity_)));

  return (stored_checksum == checksum.GetState());
}


/**
 * @brief Writes checksum of currently stored data_ at the end of the storage
 *
 */
void VirtualStorage::WriteChecksum() {
  if ((data_.GetSize() == 0) || (usable_capacity_ == 0))
    throw runtime_error("storage not Initialized");

  //TODO:    #warning Sync hash length with SFS_STORAGE_HASH_LENGTH

  Hash checksum(data_.GetConstRawPointer(), usable_capacity_);

  data_.Write(usable_capacity_, checksum.GetState().GetConstRawPointer(),
              checksum.GetStateSize());
  LOG_DEBUG("VirtualStorage::WriteChecksum: Computed CHECKSUM: "
            << StegoMath::HexBufferToStr(checksum.GetState()));
  LOG_TRACE("VirtualStorage::WriteChecksum: data_ (raw Capacity = "
            << raw_capacity_ << "): "
            << StegoMath::HexBufferToStr(&data_[0],
            static_cast<int>(raw_capacity_)));
}

} // stego_disk
