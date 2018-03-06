/**
* @file context_fitness.h
* @author Matus Kysel
* @date 2016
* @brief Context fitness function
*
*/

#ifndef STEGODISK_FITNESS_CONTEXTFITNESS_H_
#define STEGODISK_FITNESS_CONTEXTFITNESS_H_

#include <array>
#include <forward_list>

#include "fitness.h"

namespace stego_disk {

//PSTODO nemalo by toto byt niekde pouzite?
class ContextFitness : public Fitness {

public:
  ContextFitness(std::shared_ptr<CarrierFile> file);
  ~ContextFitness();

  uint64 SelectBytes(const MemoryBuffer &in, MemoryBuffer *out);
  void InsertBytes(const MemoryBuffer &in, MemoryBuffer *out) const;

private:
  bool CheckSubboxValidity(const std::array<unsigned char, 4> &subbox) const;
  bool CheckValidity(const std::array<unsigned char, 9>& box) const;

  std::forward_list<uint64> selected_bits_;

};

} // stego_disk

#endif // STEGODISK_FITNESS_CONTEXTFITNESS_H_
