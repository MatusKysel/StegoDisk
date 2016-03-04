/**
* @file context_fitness.cc
* @author Matus Kysel
* @date 2016
* @brief Implementation of context fitness function
*
*/

#include "context_fitness.h"
#include "carrier_files/carrier_file.h"

namespace stego_disk {

ContextFitness::ContextFitness(std::shared_ptr<CarrierFile> file) : Fitness(file)
{}

ContextFitness::~ContextFitness() {}

bool ContextFitness::CheckSubboxValidity(const std::array<unsigned char, 4> &subbox) const {
  unsigned char validity = 1;

  validity <<= !(subbox[0] - subbox[1]);
  validity <<= !(subbox[0] - subbox[2]);
  validity <<= !(subbox[0] - subbox[3]);
  validity <<= !(subbox[1] - subbox[2]);
  validity <<= !(subbox[1] - subbox[3]);
  validity <<= !(subbox[2] - subbox[3]);

  if (validity > 2)
    return false;

  return true;
}


bool ContextFitness::CheckValidity(const std::array<unsigned char, 9> &box) const {
  bool zero, one;
  unsigned char central_pixel;

  central_pixel = box[4] & 0xFE;
  zero = (CheckSubboxValidity(std::array<unsigned char, 4>{{box[0], box[1], box[3], central_pixel}}) &&
          CheckSubboxValidity(std::array<unsigned char, 4>{{box[1], box[2], central_pixel, box[5]}}) &&
          CheckSubboxValidity(std::array<unsigned char, 4>{{box[3], central_pixel, box[6], box[7]}}) &&
          CheckSubboxValidity(std::array<unsigned char, 4>{{central_pixel, box[5], box[7], box[8]}}));

  central_pixel |= 0x01;

  one = (CheckSubboxValidity(std::array<unsigned char, 4>{{box[0], box[1], box[3], central_pixel}}) &&
         CheckSubboxValidity(std::array<unsigned char, 4>{{box[1], box[2], central_pixel, box[5]}}) &&
         CheckSubboxValidity(std::array<unsigned char, 4>{{box[3], central_pixel, box[6], box[7]}}) &&
         CheckSubboxValidity(std::array<unsigned char, 4>{{central_pixel, box[5], box[7], box[8]}}));

  return zero && one;
}


uint64 ContextFitness::SelectBytes(MemoryBuffer &in, MemoryBuffer *out) {
  if(file_->IsGrayscale()) {
    uint64 counter  = 0, i_max, j_max, i_ind, j_ind, index;

    i_max = file_->GetHeight() / 3;
    j_max = file_->GetWidth() / 3;
    i_ind = 1;

    for (uint64 i = 0; i < i_max; ++i) {
      j_ind = 2;
      for (uint64 j = 0; j < j_max; ++j) {

        index = i_ind * file_->GetWidth() + j_ind - 1;
        if (CheckValidity(std::array<unsigned char, 9>{{in[index - file_->GetWidth() - 1],
                          in[index - file_->GetWidth()], in[index - file_->GetWidth() + 1],
                          in[index - 1], in[index], in[index + 1],
                          in[index + file_->GetWidth() - 1], in[index + file_->GetWidth()],
                          in[index + file_->GetWidth() + 1]}})) {
          ++counter;

          if(selected_bits_.empty())
            selected_bits_.push_front(index);
          else
            selected_bits_.insert_after(selected_bits_.end(), index);
        }
        j_ind += 3;
      }
      i_ind += 3;
    }
    out->Resize(counter);

    for(auto & element : selected_bits_) {
      *out = in[element];
      ++out;
    }
    return counter;

  } else {
    out->Resize(in.GetSize());
    out = &in;
    return in.GetSize();
  }
}
void ContextFitness::InsertBytes(MemoryBuffer &in, MemoryBuffer *out) const {
  if(file_->IsGrayscale()) {
    out->Resize(in.GetSize());
    uint64 index = 0;
    for(auto & element : selected_bits_) {
      (*out)[element] = in[index];
      ++index;
    }

  } else {
    out = &in;
  }
}

} // stego_disk
