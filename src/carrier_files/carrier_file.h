/**
* @file carrier_file.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File with interfce for loading and saving files
*
*/

#ifndef STEGODISK_CARRIERFILES_CARRIERFILE_H_
#define STEGODISK_CARRIERFILES_CARRIERFILE_H_

#include <sys/stat.h>
#include <errno.h>

#include <iostream>
#include <string>
#include <typeinfo>

#include "utils/config.h"
#include "utils/stego_types.h"
#include "utils/file.h"
#include "utils/memory_buffer.h"
#include "logging/logger.h"
#include "encoders/encoder_factory.h"
#include "permutations/permutation_factory.h"
#include "permutations/permutation.h"
#include "virtual_storage/virtual_storage.h"
#include "keys/key.h"
#include "fitness/fitness.h"


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

  void SetPermutation(std::shared_ptr<Permutation> permutation);
  void UnSetPermutation();
  void SetEncoder(std::shared_ptr<Encoder> encoder);
  void UnSetEncoder();
  uint64 GetCapacityUsingEncoder(std::shared_ptr<Encoder> encoder);

  virtual bool IsFileLoaded();
  virtual int LoadFile() = 0;
  virtual int SaveFile() = 0;

  void SetSubkey(const Key& subkey_);
  int AddToVirtualStorage(std::shared_ptr<VirtualStorage> storage, uint64 offSet,
                          uint64 bytes_used);

  Key GetPermKey();
  uint32 GetWidth();
  uint32 GetHeight();
  bool IsGrayscale();

  bool operator< (const CarrierFile& val) const;

  static bool CompareByPointers(CarrierFile* a, CarrierFile* b);
  static bool CompareBySharedPointers(std::shared_ptr<CarrierFile> a,
                                      std::shared_ptr<CarrierFile> b);

protected:
  int SetDatesBack();

  void SetBitInBufferPermuted(uint64 index);
  uint8 GetBitInBufferPermuted(uint64 index);

  int ExtractBufferUsingEncoder();
  int EmbedBufferUsingEncoder();

  MemoryBuffer buffer_;
  uint32 width_;
  uint32 height_;
  bool is_grayscale_;
  uint32 codeword_block_size_;
  uint32 data_block_size_;
  uint32 block_count_;
  uint64 capacity_;
  uint64 raw_capacity_;
  uint32 blocks_used_;
  uint64 virtual_storage_offset_;
  bool file_loaded_;
  Key subkey_;
  File file_;
  std::shared_ptr<Encoder> encoder_;
  std::shared_ptr<Permutation> permutation_;
  std::unique_ptr<Fitness> fitness_;
  std::shared_ptr<VirtualStorage> virtual_storage_;
};

} // stego_disk

#endif // STEGODISK_CARRIERFILES_CARRIERFILE_H_
