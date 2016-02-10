//
//  StegoStorage.h
//  StegoDiskCore
//
//  Created by Martin Kosdy on 15/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_STEGOSTORAGE_H_
#define STEGODISK_STEGOSTORAGE_H_

#include <memory>
#include <string>
#include <stdexcept>

#include "encoders/encoder_factory.h"
#include "permutations/permutation_factory.h"

namespace stego_disk {

class CarrierFilesManager;
class VirtualStorage;

class StegoStorage {

public:
  StegoStorage();
  ~StegoStorage();

  void Open(std::string storage_base_path, std::string password);
  void Load(EncoderFactory::EncoderType encoder,
            PermutationFactory::PermutationType global_perm,
            PermutationFactory::PermutationType local_perm);
  void Load();
  void Save();

  void Read(void* destination, std::size_t offset, std::size_t length) const;
  void Write(void* source, std::size_t offset, std::size_t length);

  std::size_t GetSize() const;

private:
  std::unique_ptr<CarrierFilesManager> carrier_files_manager_;

  //TODO: after some refactoring unique_ptr could be enough here:
  std::shared_ptr<VirtualStorage> virtual_storage_;

  bool opened_;

};

} // stego_disk

#endif // STEGODISK_STEGOSTORAGE_H_
