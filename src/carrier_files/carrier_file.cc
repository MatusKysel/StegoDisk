/**
* @file carrier_file.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File with implementation of interfce for loading and saving files
*
*/

#include "carrier_file.h"

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include <algorithm>

#include "encoders/encoder.h"
#include "permutations/permutation_factory.h"
#include "utils/config.h"
#include "utils/exceptions.h"
#include "utils/keccak/keccak.h"
#include "utils/stego_errors.h"
#include "utils/stego_header.h"

namespace stego_disk {

CarrierFile::CarrierFile(File file,
                         std::shared_ptr<Encoder> encoder,
                         std::shared_ptr<Permutation> permutation,
                         std::unique_ptr<Fitness> fitness)
  : width_(0),
    height_(0),
    is_grayscale_(false),
    codeword_block_size_(0),
    data_block_size_(0),
    block_count_(0),
    capacity_(0),
    raw_capacity_(0),
    blocks_used_(0),
    virtual_storage_offset_(0),
    file_loaded_(false),
    file_(file),
    encoder_(encoder),
    permutation_(permutation),
    fitness_(std::move(fitness)),
    virtual_storage_(std::shared_ptr<VirtualStorage>(nullptr)) {

  if (encoder)  {
    data_block_size_ = encoder->GetDataBlockSize();
    codeword_block_size_ = encoder->GetCodewordBlockSize();
  }
}

CarrierFile::~CarrierFile() {}

File CarrierFile::GetFile() {
  return file_;
}

void CarrierFile::UnSetEncoder() {
  SetEncoder(std::shared_ptr<Encoder>(nullptr));
}

void CarrierFile::SetEncoder(std::shared_ptr<Encoder> encoder) {
  if (!encoder) {
    data_block_size_ = 1;
    codeword_block_size_ = 1;
    block_count_ = 0;
    capacity_ = 0;
    encoder_ = std::shared_ptr<Encoder>(nullptr);
  } else {
    encoder_ = encoder;
    data_block_size_ = encoder->GetDataBlockSize();
    codeword_block_size_ = encoder->GetCodewordBlockSize();
    block_count_ = static_cast<uint32>((permutation_->GetSizeUsingParams(
                                          raw_capacity_ * 8, subkey_) / 8)
                                       / encoder->GetCodewordBlockSize());
    capacity_ = static_cast<uint64>(block_count_ ) * static_cast<uint64>(encoder->GetDataBlockSize());
  }
}

uint64 CarrierFile::GetCapacityUsingEncoder(std::shared_ptr<Encoder> encoder) {
  if (!encoder || !permutation_) return 0;
  uint64 block_count = ((permutation_->GetSizeUsingParams(
                           raw_capacity_ * 8, subkey_) / 8)
                        / encoder->GetCodewordBlockSize());
  return (block_count * encoder->GetDataBlockSize());
}

uint64 CarrierFile::GetCapacity() {
  return capacity_;
}

uint32 CarrierFile::GetBlockCount() {
  return block_count_;
}

Key CarrierFile::GetPermKey() {
  string buf = "";
  buf.append(file_.GetNormalizedPath());
  LOG_TRACE("CarrierFile::GetPermKeyHash: hashing file attributes: " << buf);
  return Key::FromString(buf);
}

bool CarrierFile::IsFileLoaded() {
  return file_loaded_;
}

int CarrierFile::AddToVirtualStorage(std::shared_ptr<VirtualStorage> storage,
                                     uint64 offset,
                                     uint64 bytes_used) {
  virtual_storage_ = storage;
  virtual_storage_offset_ = offset;

  if (bytes_used) {
    blocks_used_ = static_cast<uint32>(((bytes_used - 1) / data_block_size_) + 1);
  } else {
    blocks_used_ = 0;
  }

  return 0;
}

bool CarrierFile::operator< (const CarrierFile& val) const {

  string str_a = file_.GetRelativePath();
  string str_b = val.file_.GetRelativePath();

  std::transform(str_a.begin(), str_a.end(), str_a.begin(), ::tolower);
  std::transform(str_b.begin(), str_b.end(), str_b.begin(), ::tolower);

  //LOG_DEBUG("operator< was called for string: " << _relativePath << " vs " << val._relativePath << ", transformed: " << str_a << " vs " << str_b);

  return (str_a.compare(str_b) < 0);
}

bool CarrierFile::CompareByPointers(CarrierFile* a, CarrierFile* b) {
  if (a == nullptr)
    return 0;


  if (b == nullptr)
    return 0;

  return (*a < *b);
}

bool CarrierFile::CompareBySharedPointers(std::shared_ptr<CarrierFile> a,
                                          std::shared_ptr<CarrierFile> b) {
  if (a == nullptr)
    return 0;


  if (b == nullptr)
    return 0;

  return (*(a.get()) < *(b.get()));
}


uint64 CarrierFile::GetRawCapacity() {
  return raw_capacity_;
}

void CarrierFile::SetSubkey(const Key& subkey) {
  this->subkey_ = subkey;
}

void CarrierFile::SetBitInBufferPermuted(uint64 index) {
  const uint64 size = permutation_->GetSize();
  if (index >= size) {
    LOG_INFO("CarrierFile::SetBitInBufferPermuted: index " << index <<
             " is too big!");
    throw std::out_of_range("Index is out of range!");
  }

  uint64 permuted_index = permutation_->Permute(index);

  if (permuted_index >= size) {
    LOG_INFO("CarrierFile::SetBitInBufferPermuted: permuted index " << permuted_index << " is too big!");
    throw std::out_of_range("Permuted index is out of range!");
  }

  buffer_[permuted_index / 8] |= ( 1 << (permuted_index % 8));
}

// Checks that the permuted bit range fits the buffer, so the bulk loops below
// can index it directly instead of bounds checking every single bit.
uint64 CarrierFile::PermutedBitCapacity() const {
  const uint64 size = permutation_->GetSize();
  if (size > static_cast<uint64>(buffer_.GetSize()) * 8)
    throw std::out_of_range("Permutation does not fit the carrier buffer!");
  return size;
}

/**
 * @brief Copies the low bit of each source byte into the permuted buffer
 *
 * Equivalent to calling SetBitInBufferPermuted() for every set low bit, but
 * the range checks and the buffer lookup are hoisted out of the loop. This
 * runs once per carrier bit, so only the permutation call remains per bit.
 *
 * @param[in] source  bytes whose low bit carries the data
 * @param[in] count   number of bytes to read
 */
void CarrierFile::ExtractLsbToBufferPermuted(const uint8 *source,
                                             uint64 count) {
  if (source == nullptr)
    throw exception::NullptrArgument{"source"};

  const uint64 size = PermutedBitCapacity();
  if (count > size)
    throw std::out_of_range("Index is out of range!");

  uint8 *buffer = buffer_.GetRawPointer();
  Permutation &permutation = *permutation_;

  for (uint64 i = 0; i < count; ++i) {
    if ((source[i] & 0x01) == 0) continue;
    const uint64 permuted_index = permutation.Permute(i);
    // Kept from the per-bit version. Against a cached size this is a compare,
    // not the virtual call it used to be.
    if (permuted_index >= size)
      throw std::out_of_range("Permuted index is out of range!");
    buffer[permuted_index / 8] |= static_cast<uint8>(1 << (permuted_index % 8));
  }
}

/**
 * @brief Writes permuted buffer bits back into the low bit of each byte
 *
 * The counterpart of ExtractLsbToBufferPermuted(), with the same checks
 * hoisted out of the loop.
 *
 * @param[in,out] destination  bytes whose low bit is replaced
 * @param[in]     count        number of bytes to write
 */
void CarrierFile::ApplyBufferPermutedToLsb(uint8 *destination, uint64 count) {
  if (destination == nullptr)
    throw exception::NullptrArgument{"destination"};

  const uint64 size = PermutedBitCapacity();
  if (count > size)
    throw std::out_of_range("Index is out of range!");

  const uint8 *buffer = buffer_.GetConstRawPointer();
  Permutation &permutation = *permutation_;

  for (uint64 i = 0; i < count; ++i) {
    const uint64 permuted_index = permutation.Permute(i);
    if (permuted_index >= size)
      throw std::out_of_range("Permuted index is out of range!");
    const uint8 bit = (buffer[permuted_index / 8] >>
                       (permuted_index % 8)) & 0x01;
    destination[i] = static_cast<uint8>((destination[i] & 0xFE) | bit);
  }
}

uint8 CarrierFile::GetBitInBufferPermuted(uint64 index) {

  if (index >= permutation_->GetSize()) {
    LOG_INFO("CarrierFile::GetBitInBufferPermuted: index " << index <<
             " is too big!");
    throw std::out_of_range("Index is out of range!");
  }

  uint64 permuted_index = permutation_->Permute(index);

  return ((buffer_[permuted_index / 8] & (1 << (permuted_index % 8))) != 0);
}

int CarrierFile::ExtractBufferUsingEncoder() {
  if (!buffer_.GetSize()) return -1;
  if (!encoder_) return -2;
  if (!codeword_block_size_) return -3;
  if (!blocks_used_) return -4;

  MemoryBuffer data_buffer(data_block_size_);

  for (uint64 b = 0; b < blocks_used_; ++b) {
    encoder_->Extract(&buffer_[b * codeword_block_size_],
        data_buffer.GetRawPointer());

    for (uint64 i = 0; i < data_block_size_; ++i) {
      //TODO:            #warning doriesit try catch!
      try {
        virtual_storage_->WriteByte(virtual_storage_offset_ +
                                    (b * data_block_size_) + i, data_buffer[i]);
      } catch (std::out_of_range&) {
        LOG_DEBUG("CarrierFile::extractBufferUsingEncoder: "
                  "virtualStorage->writeByte failed: block: " << (b + 1) << "/"
                  << blocks_used_ << ", byte: " << (i + 1) << "/" <<
                  data_block_size_);
        //TODO: poriesit
        //LOG_TRACE("CarrierFile::extractBufferUsingEncoder: virtualStorage->writeByte failed: error code: " << errc << ", block: " << (b+1) << "/" << blocks_used_ << ", byte: " << (i+1) << "/" << data_block_size_);
        // TODO: poriesit ako zistit kedy je to error a kedy koniec uloziska -> to je ok
        // TODO: virtual_storage_->raw_capacity_ testovat
        //LOG_ERROR("ERROR WRITING BYTE! error code: " << errc << ", block: " << (b+1) << "/" << blocks_used_ << ", byte: " << (i+1) << "/" << data_block_size_);
      }
    }
  }

  return 0;
}


int CarrierFile::EmbedBufferUsingEncoder() {
  if (buffer_.GetSize() == 0) throw std::length_error("Buffer is empty!");
  if (!encoder_)
    throw exception::InvalidState{exception::Operation::embedBufferUsingEncoder,
		                          exception::Component::encoder,
								  exception::ComponentState::notSetted};
  if (!codeword_block_size_) throw exception::EmptyMember{"codeword_block_size_"};
  if (!blocks_used_) throw exception::EmptyMember{"codeword_block_size_"};

  MemoryBuffer data_buffer(data_block_size_);

  for (uint64 b = 0; b < blocks_used_; ++b) {
    for (uint64 i = 0; i < data_block_size_; ++i) {
      //TODO:            #warning doriesit try catch!
      try {
        data_buffer[i] = virtual_storage_->ReadByte(virtual_storage_offset_ +
                                                    (b * data_block_size_) + i);
      } catch (std::out_of_range&) {
        LOG_DEBUG("CarrierFile::embedBufferUsingEncoder: virtualStorage->"
                  "readByte failed: block: " << (b + 1) << "/" << blocks_used_
                  << ", byte: " << (i + 1) << "/" << data_block_size_);
        //TODO: poriesit ako zistit kedy je to error a kedy koniec uloziska -> to je ok
        //LOG_ERROR("ERROR READING BYTE! error code: " << errc << ", block: " << (b+1) << "/" << blocks_used_ << ", byte: " << (i+1) << "/" << data_block_size_);
        data_buffer[i] = 0; // TODO: not sure if random data would be better here
      }
    }
    encoder_->Embed(&buffer_[b * codeword_block_size_],
        data_buffer.GetConstRawPointer());
  }

  return 0;
}


uint32 CarrierFile::GetWidth() {
  return width_;
}

uint32 CarrierFile::GetHeight() {
  return height_;
}

bool CarrierFile::IsGrayscale() {
  return is_grayscale_;
}
} // stego_disk
