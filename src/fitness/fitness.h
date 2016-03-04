/**
* @file fitness.h
* @author Matus Kysel
* @date 2016
* @brief Fitness functions interface
*
*/

#ifndef STEGODISK_FITNESS_FITNESS_H_
#define STEGODISK_FITNESS_FITNESS_H_

#include <string>
#include <memory>
#include <stdexcept>

#include "utils/stego_types.h"
#include "logging/logger.h"
#include "utils/memory_buffer.h"

namespace stego_disk {

class CarrierFile;

class Fitness {

public:
  explicit Fitness(std::shared_ptr<CarrierFile> file) { file_ = file; }

  Fitness(const Fitness&) = delete;
  Fitness& operator=(const Fitness&) = delete;

  virtual ~Fitness();

  virtual uint64 SelectBytes(MemoryBuffer &in, MemoryBuffer *out) = 0;

  virtual void InsertBytes(MemoryBuffer &in, MemoryBuffer *out) const = 0;

protected:
  std::shared_ptr<CarrierFile> file_;

};

} // stego_disk

#endif // STEGODISK_FITNESS_FITNESS_H_
