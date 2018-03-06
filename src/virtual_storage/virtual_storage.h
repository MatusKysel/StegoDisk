/**
* @file virtual_storage.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Main class for virtual storage
*
*/

#ifndef STEGODISK_VIRTUALSTORAGE_VIRTUALSTORAGE_H_
#define STEGODISK_VIRTUALSTORAGE_VIRTUALSTORAGE_H_

#include <memory>

#include "utils/stego_types.h"
#include "permutations/permutation_factory.h"
#include "keys/key.h"


namespace stego_disk {

/**
 * Main storage buffer that utilizes global permutation in readByte/writeByte ops
 *
 * [ STORAGE (len: usable capacity) | CHECKSUM/HASH (len: hash length) ]
*/

class VirtualStorage {
private:
  void Init();

public:
  VirtualStorage();
  ~VirtualStorage();

  static std::shared_ptr<VirtualStorage> GetNewInstance();
  static std::shared_ptr<VirtualStorage> GetNewInstance(string permutation);

  // Initialization of the VirtualStorage depends on permutation
  void SetPermutation(std::shared_ptr<Permutation> permutation);
  void UnSetPermutation();
  void ApplyPermutation(uint64 requested_size, Key key);

  //PSTODO ak tomu spravne chapem, *Byte robi na spermutovanimi, read/write nad nespermutovanimi? ak ano, su to trochu neintuitivne nazvy...
  // Accessed by CarrierFile during save/load operation
  void WriteByte(uint64 position, uint8 value);
  uint8 ReadByte(uint64 position);

  // Accessed by main I/O layer (Fuse, VirtualDisc driver..)
  void Read(uint64 offSet, std::size_t length, uint8* buffer) const;
  void Write(uint64 offSet, std::size_t length, const uint8* buffer);

  uint64 GetRawCapacity();
  uint64 GetUsableCapacity();

  void RandomizeBuffer();
  void ClearBuffer();
  void FillBuffer(uint8 value);

  bool IsValidChecksum();
  void WriteChecksum();

private:
  std::shared_ptr<Permutation> global_permutation_;
  bool   is_set_global_permutation_;
  uint64 raw_capacity_;                // raw capacity (hash + storage)
  uint64 usable_capacity_;             // usable capacity (storage only)
  MemoryBuffer data_;
};

} // stego_disk

#endif // STEGODISK_VIRTUALSTORAGE_VIRTUALSTORAGE_H_
