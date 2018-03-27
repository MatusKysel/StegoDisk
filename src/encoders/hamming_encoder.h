/**
* @file hamming_encoder.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Hamming encoder class
*
*/

#ifndef STEGODISK_ENCODERS_HAMMINGENCODER_H_
#define STEGODISK_ENCODERS_HAMMINGENCODER_H_

#include "api_mask.h"
#include "encoder.h"

#define ENCODER_HAMMING_CODE_NAME_DEF "Hamming"

namespace stego_disk {

class HammingEncoder : public Encoder {

public:
  HammingEncoder();
  HammingEncoder(uint32 parity_bits);

  int Embed(uint8 *codeword, const uint8 *data);
  int Extract(const uint8 *codeword, uint8 *data);

  void SetArgByName(const string &arg, const string &val);

  static const string GetName();
  const string GetNameInstance() const;
  static shared_ptr<Encoder> GetNew();
  shared_ptr<Encoder> GetNewInstance();

  static int GetParityBitsMin();
  static int GetParityBitsMax();

private:
  void Init(uint32 parity_bits);
  uint64 ComputeH(const uint8 *buffer);
  void SwapBitInCodeword(uint8 *buffer, int bit_in);
  void WriteBitsToBuffer(uint64 value, int offset, uint8 *buffer);
  uint64 ReadBitsFromBuffer(const uint8 *buffer, int offset, int length);

  uint32 parity_bits_;
  uint32 total_bits_;

  uint32 codewords_in_block_;
  uint32 codeword_buffer_size_;

  static const uint32 kEncoderHammingParityBitsMin = 3;
  static const uint32 kEncoderHammingParityBitsMax = 8;
  static const string kEncoderHammingCodeName;
  // default block size
  static const uint32 kEncoderHammingDefaultParityBits = 5;
};

} // stego_disk

#endif // STEGODISK_ENCODERS_HAMMINGENCODER_H_
