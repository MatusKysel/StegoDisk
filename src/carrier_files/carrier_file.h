/**
* @file carrier_file.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File with interfce for loading and saving files
*
*/

#pragma once

#include <errno.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <typeinfo>

#include "encoders/encoder_factory.h"
#include "fitness/fitness.h"
#include "keys/key.h"
#include "logging/logger.h"
#include "permutations/permutation.h"
#include "permutations/permutation_factory.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/memory_buffer.h"
#include "utils/stego_types.h"
#include "virtual_storage/virtual_storage.h"


namespace stego_disk {



/**
 * The CarrierFile class.
 *
 * Abstract base class CarrierFile (cannot be instantiated)
 * Provides an interface for CarrierFiles subclasses
 */
class CarrierFile {

public:
  CarrierFile(File file,
              std::shared_ptr<Encoder> encoder,
              std::shared_ptr<Permutation> permutation,
              std::unique_ptr<Fitness> fitness);

  virtual ~CarrierFile();

  uint64 GetCapacity();
  uint64 GetRawCapacity();
  uint32 GetBlockCount();

  File GetFile();

  void SetEncoder(std::shared_ptr<Encoder> encoder);
  void UnSetEncoder();
  uint64 GetCapacityUsingEncoder(std::shared_ptr<Encoder> encoder);

  virtual bool IsFileLoaded();
  virtual void LoadFile() = 0;
  virtual void SaveFile() = 0;

  void SetSubkey(const Key& subkey_);
  int AddToVirtualStorage(std::shared_ptr<VirtualStorage> storage, uint64 offSet,
                          uint64 bytes_used);

  Key GetPermKey() const;
  uint32 GetWidth() const;
  uint32 GetHeight() const;
  bool IsGrayscale() const;

  bool operator< (const CarrierFile& val) const;

  static bool CompareByPointers(CarrierFile* a, CarrierFile* b);
  static bool CompareBySharedPointers(std::shared_ptr<CarrierFile> a,
                                      std::shared_ptr<CarrierFile> b);

protected:
  void SetBitInBufferPermuted(uint64 index);
  uint8 GetBitInBufferPermuted(uint64 index);

  int ExtractBufferUsingEncoder();
  int EmbedBufferUsingEncoder();

  MemoryBuffer buffer_;
  uint32 width_{ 0 };
  uint32 height_{ 0 };
  bool is_grayscale_{ false };
  uint32 codeword_block_size_{ 0 };
  uint32 data_block_size_{ 0 };
  uint32 block_count_{ 0 };
  uint64 capacity_{ 0 };
  uint64 raw_capacity_{ 0 };
  uint32 blocks_used_{ 0 };
  uint64 virtual_storage_offset_{ 0 };
  bool file_loaded_{ false };
  Key subkey_;
  File file_;
  std::shared_ptr<Encoder> encoder_{ nullptr };
  std::shared_ptr<Permutation> permutation_{ nullptr };
  std::unique_ptr<Fitness> fitness_{ nullptr };
  std::shared_ptr<VirtualStorage> virtual_storage_{ nullptr };
};

} // stego_disk
