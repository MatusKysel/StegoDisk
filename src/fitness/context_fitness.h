/**
* @file context_fitness.h
* @author Matus Kysel
* @date 2016
* @brief Context fitness function
*
*/

#pragma once

#include <array>
#include <forward_list>

#include "api_mask.h"
#include "fitness.h"

namespace stego_disk {

//PSTODO nemalo by toto byt niekde pouzite?
class ContextFitness : public Fitness {

public:
  ContextFitness(std::shared_ptr<CarrierFile> file);
  ~ContextFitness();

  virtual uint64 SelectBytes(const MemoryBuffer &in, MemoryBuffer *out) override;
  virtual void InsertBytes(const MemoryBuffer &in, MemoryBuffer *out) const override;

private:
  bool CheckSubboxValidity(const std::array<unsigned char, 4> &subbox) const;
  bool CheckValidity(const std::array<unsigned char, 9>& box) const;

  std::forward_list<uint64> selected_bits_;

};

} // stego_disk
