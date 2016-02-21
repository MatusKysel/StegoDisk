/**
* @file stego_storage.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File containing StegoStorage interface.
*
*/

#ifndef STEGODISK_STEGOSTORAGE_H_
#define STEGODISK_STEGOSTORAGE_H_

#include <memory>
#include <string>
#include <stdexcept>

#include "encoders/encoder_factory.h"
#include "permutations/permutation_factory.h"

#define CONFIG_NAME ".config.json"

namespace stego_disk {

class CarrierFilesManager;
class VirtualStorage;

class StegoStorage {

public:
  StegoStorage();
  ~StegoStorage();

  void Open(const std::string &storage_base_path, const std::string &password);
  void Load();
  void Save();

  void Read(void* destination, std::size_t offset, std::size_t length) const;
  void Write(void* source, std::size_t offset, std::size_t length);

  void Configure(const std::string &config_path) const;
  void Configure() const;
  void Configure(const EncoderFactory::EncoderType encoder,
                 const PermutationFactory::PermutationType global_perm,
                 const PermutationFactory::PermutationType local_perm) const;

  std::size_t GetSize() const;

private:

  std::unique_ptr<CarrierFilesManager> carrier_files_manager_;

  //TODO: after some refactoring unique_ptr could be enough here:
  std::shared_ptr<VirtualStorage> virtual_storage_;
  bool opened_;

};

} // stego_disk

#endif // STEGODISK_STEGOSTORAGE_H_
