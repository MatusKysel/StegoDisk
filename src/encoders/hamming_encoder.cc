//
//  HammingEncoder.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/21/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "hamming_encoder.h"

#include <string.h>
#include <algorithm>

#include "utils/stego_math.h"
#include "utils/stego_errors.h"
#include "logging/logger.h"

namespace stego_disk {

const string HammingEncoder::kEncoderHammingCodeName = ENCODER_HAMMING_CODE_NAME_DEF;

/**
 * @brief Get minimal value for parameter 'parity_bits'
 *
 * Get the minimal value for parameter 'parity_bits' used by Hamming coder instance.
 * Minimal value is 3.
 *
 * @return Minimal value required for parameter 'parity_bits'
 */
int HammingEncoder::GetParityBitsMax() {
  return kEncoderHammingParityBitsMax;
}

/**
 * @brief Get maximal value for parameter 'parity_bits'
 *
 * Get the maximal value for parameter 'parity_bits' used by Hamming coder instance.
 * Maximal value is 8.
 *
 * @return Maximal value required for parameter 'parity_bits'
 */
int HammingEncoder::GetParityBitsMin() {
  return kEncoderHammingParityBitsMin;
}

/**
 * @brief Return new instance of Hamming encoder
 *
 * Return New instance of Hamming encoder,
 * the return value is smart pointer.
 * Instance is created with default values of class parameters.
 * Static method!
 *
 * @return New instance of Hamming encoder class
 */
shared_ptr<Encoder> HammingEncoder::GetNew() {
  return shared_ptr<Encoder>(new HammingEncoder());
}

/**
 * @brief Return new instance of Hamming encoder
 *
 * Return New instance of Hamming encoder,
 * the return value is smart pointer.
 * Instance is created with default values of class parameters.
 * Instance method!
 *
 * @return New instance of Hamming encoder class
 */
shared_ptr<Encoder> HammingEncoder::GetNewInstance() {
  return GetNew();
}

/**
 * @brief Initialize data structures of Hamming encoder instance
 *
 * This is private function, used by member functions.
 * Requires one argument - parity_bits.
 * This argument must be in range <GetParityBitsMin(); GetParityBitsMax()>
 * If no, throws exception std::out_of_range
 *
 * With Hamming encoder we can insert k bits of information into 2^k-1 bits of the stego-carrier.
 * The parameter 'parity_bits' is 'k'.
 * So, i.e. for k=3 we insert 3 bits of information into 7 bits of cover, for k=8 we need 255 bits of cover.
 * Thus the embedding efficiency is k/(2^k-1), and for values in <3;8> is (in % of usable capacity of the cover medium):
 *    3 --> 42%, 4 --> 27%, 5 --> 16%, 6 --> 9.5%, 7 --> 5.5%, 8 --> 3.1%
 *
 * But one great advantage of Hamming encoder is, that for insertion of k bits into 2^k-1 bits we need modify at most 1 bit.
 * So for values of k in <3;8> we change at most (in % of usable capacity of the cover medium):
 *    3 --> 14%, 4 --> 6.7%, 5 --> 3.2%, 6 --> 1.6%, 7 --> 0.8%, 8 --> 0.4%
 * We see, that increasing parameter k we increase the security, in terms of possibility of the statistical stegoanalysis attacks.
 *
 * @param[in] parity_bits Must be in range <GetParityBitsMin(); GetParityBitsMax()>
 * @return Throws exception std::out_of_range, if 'parity_bits' is out of the range
 */
void HammingEncoder::Init(uint32 parity_bits) {
  if ((parity_bits < kEncoderHammingParityBitsMin) ||
      (parity_bits > kEncoderHammingParityBitsMax)) {
    string err = "HammingEncoder::init: 'parity_bits' is " +
                 std::to_string(static_cast<uint64>(parity_bits));
    err += ", should be in <" + kEncoderHammingParityBitsMin;
    err += "," + std::to_string(
             static_cast<uint64>(kEncoderHammingParityBitsMax));
    err += ">";
    throw std::out_of_range(err);
  }

  // parity bits = k
  // total bits = 2^k - 1
  // data bits = 2^k - 1 - k
  // note: this embedder uses parity bits as data storage!

  // Hamming code params
  parity_bits_ = parity_bits;
  /*
    total_bits_ = 4;
    for (int i=2;i<parity_bits_;i++)
        total_bits_ *= 2;
    total_bits_--;
    */
  total_bits_ = static_cast<uint64>(1L << parity_bits_) - 1;

  // in/out block params
  data_block_size_ = static_cast<uint32>(StegoMath::Lcm(parity_bits_, 8)) / 8;
  codewords_in_block_ = (data_block_size_ * 8) / parity_bits_;
  codeword_buffer_size_ = (total_bits_ - 1) / 8 + 1;
  codeword_block_size_ = codeword_buffer_size_ * codewords_in_block_;

  LOG_DEBUG("HammingEncoder::init: parity bits: " << parity_bits_ << ", " <<
            "total bits: " << total_bits_ << ", " <<
            "dataBlockSize: " << data_block_size_ << ", " <<
            "codewordsInBlock: " << codewords_in_block_ << ", " <<
            "codeword buffer size: " << codeword_buffer_size_);
}

/**
 * @brief Constructor of the Hamming encoder instance with default parameter
 *
 * Construct new instance of Hamming encoder with default parameter 'parity_bits'.
 * This parameter is set to the default value = 5.
 * Why 5? See description of parametric constructor...
 * Yes, if we use parity_bits=5, we can use only 16% of the cover media capacity,
 * but we modify at most 3.2% of this media, which is below 5% border of the significance
 * of statistical hypothesis testing in case of some steganalysis attack.
 *
 * @return Instance of the Hamming encoder initialized with default values
 */
HammingEncoder::HammingEncoder() {
  Init(kEncoderHammingDefaultParityBits);
}

/**
 * @brief Constructor of the Hamming encoder instance
 *
 * Requires one argument - parity_bits.
 * This argument must be in range <GetParityBitsMin(); GetParityBitsMax()>
 * If no, throws exception std::out_of_range
 *
 * With Hamming encoder we can insert k bits of information into 2^k-1 bits of the stego-carrier.
 * The parameter 'parity_bits' is 'k'.
 * So, i.e. for k=3 we insert 3 bits of information into 7 bits of cover, for k=8 we need 255 bits of cover.
 * Thus the embedding efficiency is k/(2^k-1), and for values in <3;8> is (in % of usable capacity of the cover media):
 *    3 --> 42%, 4 --> 27%, 5 --> 16%, 6 --> 9.5%, 7 --> 5.5%, 8 --> 3.1%
 *
 * But one great advantage of Hamming encoder is, that for insertion of k bits we need modify at most 1 bit of 2^k-1 bits.
 * So for values of k in <3;8> we change at most (in % of usable capacity of the cover media):
 *    3 --> 14%, 4 --> 6.7%, 5 --> 3.2%, 6 --> 1.6%, 7 --> 0.8%, 8 --> 0.4%
 * We see, that increasing parameter k we increase the security, in terms of impossibility of the statistical stegoanalysis attacks.
 *
 * @param[in] parity_bits Must be in range <GetParityBitsMin(); GetParityBitsMax()>
 * @return Instance of the Hamming encoder initialized with parameter 'parity_bits'
 */
HammingEncoder::HammingEncoder(uint32 parity_bits) {
  try { Init(parity_bits); }
  catch (const std::out_of_range &) { throw; }
}

/**
 * @brief Set parameter param of the encoder with value val
 *
 * This method set value of an encoder's parameter.
 * Both of the arguments must be of the string type.
 *
 * List of supported parameters for Hamming encoder (purpose of each parameter see constructor of the class):
 *     "parity_bits", value (as string) from range <GetParityBitsMin(); GetParityBitsMax()>
 *
 * @param[in] param Name of the parameter to be set
 * @param[in] val Value of the parameter to be set; must be of the string type
 * @return For all inputs throws exception std::invalid_argument if length of 'param' or 'val' is zero
 * @return For all parameters throws exception std::out_of_range if 'param' is not valid parameter for this class
 * @return For parameter "parity_bits" throws exceptions:
 *     - std::invalid_argument, if 'val' is not a number
 *     - std::out_of_range, if 'val' is not from range <GetParityBitsMin(); GetParityBitsMax()>
 */
void HammingEncoder::SetArgByName(const string &param, const string &val) {
  bool is_valid_param = false;
  string p = param;

  if (!param.size())
    throw std::invalid_argument("HammingEncoder::setArgByName: "
                                "'param' is empty");
  if (!val.size())
    throw std::invalid_argument("HammingEncoder::setArgByName: 'val' is empty");

  std::transform(p.begin(), p.end(), p.begin(), ::tolower);

  // parameter "parity_bits"
  if (p.compare("paritybits") == 0) {
    int parity_bits;

    is_valid_param = true;
    try { parity_bits = stoi(val); }
    catch (const std::invalid_argument& ) { throw; }
    catch (const std::out_of_range& ) { throw; }

    if (parity_bits < 0)
      throw std::out_of_range("HammingEncoder::setArgByName: "
                              "'parity_bits' should be positive, is "
                              + std::to_string(
                                static_cast<uint64>(parity_bits)));

    try { Init(static_cast<uint32>(parity_bits)); }
    catch (const std::out_of_range &) { throw; }
  }

  // other parameters..

  // if is not valid parameter
  if (is_valid_param == false)
    throw std::invalid_argument("HammingEncoder::setArgByName: "
                                "unknown class parameter '" + param + "'");
}

/**
 * @brief Get the code name of this encoder
 *
 * Get code name of Hamming encoder. The code name is "Hamming".
 * Static function, does not required any instance of the class.
 *
 * @return Code name of Hamming encoder class
 */
const string HammingEncoder::GetName() {
  return kEncoderHammingCodeName;
}

/**
 * @brief Get the code name of this encoder
 *
 * Get code name of Hamming encoder. The code name is "Hamming".
 * member method, calls static function of the class.
 *
 * @return Code name of Hamming encoder class
 */
const string HammingEncoder::GetNameInstance() const {
  return HammingEncoder::GetName();
}

/**
 * @brief Transform user data with Hamming encoder
 *
 * Encode user data (steganographic information) stored in buffer 'data' with Hamming encoder.
 * Warning: This function doesn't be called directly!
 * The size of input/output buffers is given intrinsic based on the configuration of the encoder instance.
 * Function is called from CarrierFile->embedBufferUsingEncoder when translating user stego data into carrier file.
 *
 * @param[in] data Initialized buffer with user data
 * @param[out] codeword Buffer with encoded user data
 * @return Exception std::invalid_argument, if 'codeword' or 'data' are NULL pointer
 */
int HammingEncoder::Embed(uint8 *codeword, const uint8 *data) {
  uint64 h;
  int bit_offset = 0;

  if ( !codeword )
    throw std::invalid_argument("HammingEncoder::embed: 'codeword' is NULL");
  if ( !data )
    throw std::invalid_argument("HammingEncoder::embed: 'data' is NULL");

  for (uint32 i = 0; i < codewords_in_block_; ++i) {
    h = ComputeH(&codeword[i * codeword_buffer_size_]);
    h ^= ReadBitsFromBuffer(data, bit_offset, parity_bits_); // h = h xor m

    if (h != 0) {
      SwapBitInCodeword(&codeword[i*codeword_buffer_size_],
          static_cast<int>(h));
    }

    bit_offset += parity_bits_;
  }

  return 0;
}

/**
 * @brief Transform data with Hamming decoder
 *
 * Decode data previously obtained from carrier file (stored in 'codework') into user data (steganographic information),
 * stored in buffer 'data', with Hamming decoder.
 * Warning: This function doesn't be called directly!
 * The size of input/output buffers is given intrinsic based on the configuration of the encoder instance.
 * Function is called from CarrierFile->extractBufferUsingEncoder when translating data from carrier file into user data.
 *
 * @param[in] codeword Initialized buffer with encoded user data
 * @param[out] data Buffer with decoded user data
 * @return Exception std::invalid_argument, if 'codeword' or 'data' are NULL pointer
 */
int HammingEncoder::Extract(const uint8 *codeword, uint8 *data) {
  if ( !codeword )
    throw std::invalid_argument("HammingEncoder::extract: 'codeword' is NULL");
  if ( !data )
    throw std::invalid_argument("HammingEncoder::extract: 'data' is NULL");

  memset(data, 0, data_block_size_);

  uint64 h;
  int bit_offset = 0;

  for (uint32 i = 0; i < codewords_in_block_; ++i) {
    h = ComputeH(&codeword[i * codeword_buffer_size_]);
    WriteBitsToBuffer(h, bit_offset, data);
    bit_offset += parity_bits_;
  }

  return STEGO_NO_ERROR;
}

/**
 * @brief Writes value (bit representation) to offset in buffer
 *
 * Writes bit representation of 'value' into 'buffer' starting at 'offset' position.
 * This helper function is invoked internally from HammingEncoder::extract method.
 * Method doesn't make any variables validity test.
 *
 * @param[in] value Value to be written into buffer
 * @param[in] offset Starting position from beginning of the buffer in bits
 * @param[out] buffer Initialized memory storage
 *
 * @return Nothing
 */
void HammingEncoder::WriteBitsToBuffer(uint64 value, int offset,
                                       uint8 *buffer) {
  int shr_value;
  uint8 tmp_value;
  int written_bts = 0;

  while (written_bts < static_cast<int>(parity_bits_)) {
    int byte_offset = offset / 8;
    int bit_offset = offset % 8;
    shr_value = parity_bits_ - 8 + bit_offset - written_bts;
    if (shr_value < 0) {
      tmp_value = (value << -shr_value) & ((0xFF << -shr_value) & 0xFF);
    } else {
      tmp_value = (value >> shr_value) & 0xFF;
    }
    buffer[byte_offset] ^= tmp_value;

    if (shr_value <= 0) {
      written_bts += (parity_bits_ - written_bts);
    } else {
      written_bts += ((parity_bits_ - written_bts) - shr_value);
    }

    if (shr_value <= 0) {
      offset += parity_bits_;
    } else {
      offset += (parity_bits_ - shr_value);
    }
  }
}

/**
 * @brief Reads value (bit representation) at offset from buffer
 *
 * Reads bit representation of 'result' from 'buffer' starting at 'offset' position.
 * This helper function is invoked internally from HammingEncoder::embed method.
 * Method doesn't make any variables validity test.
 *
 * @param[in] buffer Initialized memory storage with valid data
 * @param[in] offset Starting position for reading from beginning of the buffer in bits
 * @param[in] length Number of bits to be read from buffer
 *
 * @return Readed bits from buffer represented as number
 */
uint64 HammingEncoder::ReadBitsFromBuffer(const uint8 *buffer, int offset,
                                          int length) {
  uint8 tmp_value;
  uint64 tmp_long_value;
  uint64 result = 0;
  int written_bts = 0;
  int tmp_written_bits;

  while (written_bts < length) {
    int byte_offset = offset / 8;
    int bit_offset = offset % 8;
    tmp_value = (buffer[byte_offset] << bit_offset) & 0xFF;
    tmp_written_bits = 8 - bit_offset;

    if (length - written_bts < tmp_written_bits) {
      tmp_value >>= tmp_written_bits - (length - written_bts);
      tmp_written_bits -= (tmp_written_bits - (length - written_bts));
    }
    tmp_value = tmp_value >> bit_offset;
    tmp_long_value = tmp_value;
    result ^= tmp_long_value << (length - written_bts - tmp_written_bits);
    offset += tmp_written_bits;
    written_bts += tmp_written_bits;
  }
  return result;
}

/**
 * @brief Multiply input vector by control matrix H
 *
 * Multiply input vector stored in 'buffer' by control matrix of Hamming code H.
 * This function is used from HammingEncoder::extract and HammingEncoder::embed.
 * There is no control of validity of input argument.
 *
 * @param[in] buffer Input vector to be multipled
 *
 * @return Multiplication result
 */
uint64 HammingEncoder::ComputeH(const uint8 *buffer) {
  uint64 tmp_hline = 1;
  uint64 result = 0;

  for (uint32 i = 0; i < codeword_buffer_size_; ++i) {
    for (int j = 7; j >= 0; --j) {
      if (tmp_hline <= static_cast<uint64>(total_bits_)) {
        if ((buffer[i] >> j) & 0x01) result ^= tmp_hline;
        ++tmp_hline;
      }
    }
  }
  return result;
}

/**
 * @brief Swap value of the bit in codeword
 *
 * Swap value of the 'bit_in'-th bit in codeword buffer.
 * Function is used internally from HammingEncoder::embed.
 * There is no check of validity of the input args.
 *
 * @param[in] bit_in Position (in bits) of the bit to be changed in input 'buffer'
 * @param buffer Initialized buffer with valid data (codewords)
 *
 * @return Nothing
 */
void HammingEncoder::SwapBitInCodeword(uint8 *buffer, int bit_in){
  if (!bit_in)
    return;
  buffer[(bit_in - 1) / 8] ^= 1 << (7 - ((bit_in - 1) % 8));
}

} // stego_disk
