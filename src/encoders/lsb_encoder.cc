//
//  LsbEncoder.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/21/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "lsb_encoder.h"

#include <algorithm>
#include <string.h>

#include "utils/stego_math.h"
#include "logging/logger.h"

namespace stego_disk {

const string LsbEncoder::kEncoderLsbCodeName = ENCODER_LSB_CODE_NAME_DEF;

/**
 * @brief Get minimal value for parameter 'block_size'
 *
 * Get the minimal value for parameter 'block_size' used by Lsb coder instance.
 * Minimal value is 1.
 *
 * @return Minimal value required for parameter 'block_size'
 */
uint32 LsbEncoder::GetBlockSizeMin() {
  return kEncoderLsbBlockSizeMin;
}

/**
 * @brief Get maximal value for parameter 'block_size'
 *
 * Get the maximal value for parameter 'block_size' used by Lsb coder instance.
 * Maximal value is 1024.
 *
 * @return Maximal value required for parameter 'block_size'
 */
uint32 LsbEncoder::GetBlockSizeMax() {
  return kEncoderLsbBlockSizeMax;
}

/**
 * @brief Return new instance of Lsb encoder
 *
 * Return New instance of Lsb encoder,
 * the return value is smart pointer.
 * Instance is created with default values of class parameters.
 * Static method!
 *
 * @return New instance of Lsb encoder class
 */
shared_ptr<Encoder> LsbEncoder::GetNew() {
  return shared_ptr<Encoder>(new LsbEncoder());
}

/**
 * @brief Return new instance of Lsb encoder
 *
 * Return New instance of Lsb encoder,
 * the return value is smart pointer.
 * Instance is created with default values of class parameters.
 * Instance method!
 *
 * @return New instance of Lsb encoder class
 */
shared_ptr<Encoder> LsbEncoder::GetNewInstance() {
  return GetNew();
}

/**
 * @brief Initialize data structures of Lsb encoder instance
 *
 * This is private function, used by class member functions.
 * Requires one argument - block_size.
 * This argument must be in range <GetBlockSizeMin(); GetBlockSizeMax()>
 * If no, throws exception std::out_of_range.
 * This exception is involved in case if 'block_size' is not power of two, too.
 *
 * With Lsb encoder we can insert k bits of information into k bits of the stego-carrier.
 *
 * @param[in] block_size Must be in range <GetBlockSizeMin(); GetBlockSizeMax()> and power of two.
 * @return Throws exception std::out_of_range, if 'block_size' is out of the range or isn't power of two.
 */
void LsbEncoder::Init(uint32 block_size) {
  // is block_size power  2?
  if (StegoMath::Popcount(block_size) != 1)
    throw std::out_of_range("LsbEncoder::init: 'block_size' should be power of "
                            "two, is " +
                            std::to_string(static_cast<uint64>(block_size)));
  // is block_size in range
  if ((block_size < kEncoderLsbBlockSizeMin) ||
      (block_size > kEncoderLsbBlockSizeMax)) {
    string err = "LsbEncoder::init: 'block_size' is "
                 + std::to_string(static_cast<uint64>(block_size));
    err += ", should be in <" + kEncoderLsbBlockSizeMin;
    err += "," + std::to_string(static_cast<uint64>(kEncoderLsbBlockSizeMax));
    err += ">";
    throw std::out_of_range(err);
  }
  data_block_size_ = block_size;
  codeword_block_size_ = block_size;

  LOG_DEBUG("LsbEncoder::init: block_size: " << block_size << ", " <<
            "dataBlockSize: " << data_block_size_ << ", " <<
            "codewordBlockSize: " << codeword_block_size_);
}

/**
 * @brief Constructor of the Lsb encoder instance with default parameter
 *
 * Construct new instance of Lsb encoder with default parameter 'block_size'.
 * This parameter is set to the default value = 1.
 *
 * @return Instance of the Lsb encoder initialized with default values
 */
LsbEncoder::LsbEncoder() {
  Init(kEncoderLsbDefaultBlockSize);
}

/**
 * @brief Constructor of the Lsb encoder instance
 *
 * Requires one argument - block_size.
 * This argument must be in range <GetBlockSizeMin(); GetBlockSizeMax()>.
 * If no, throws exception std::out_of_range.
 * This exception is involved in case if 'block_size' is not power of two, too.
 *
 * With Lsb encoder we can insert k bits of information into k bits of the stego-carrier.
 *
 * @param[in] block_size Must be in range <GetBlockSizeMin(); GetBlockSizeMax()> and power of two.
 * @return Instance of the Lsb encoder initialized with parameter 'block_size'
 */
LsbEncoder::LsbEncoder(uint32 block_size) {
  Init(block_size);
}

/**
 * @brief Get the code name of this encoder
 *
 * Get code name of Lsb encoder. The code name is "Lsb".
 * Static function, does not required any instance of the class.
 *
 * @return Code name of Lsb encoder class
 */
const string LsbEncoder::GetName() {
  return kEncoderLsbCodeName;
}

/**
 * @brief Get the code name of this encoder
 *
 * Get code name of Lsb encoder. The code name is "Lsb".
 * member method, calls static function of the class.
 *
 * @return Code name of Lsb encoder class
 */
const string LsbEncoder::GetNameInstance() const {
  return LsbEncoder::GetName();
}

/**
 * @brief Transform user data with Lsb encoder
 *
 * Encode user data (steganographic information) stored in buffer 'data' with Lsb encoder.
 * Warning: This function doesn't be called directly!
 * The size of input/output buffers is given intrinsic based on the configuration of the encoder instance.
 * Function is called from CarrierFile->embedBufferUsingEncoder when translating user stego data into carrier file.
 *
 * @param[in] data Initialized buffer with user data
 * @param[out] codeword Buffer with encoded user data
 * @return Exception std::invalid_argument, if 'codeword' or 'data' are NULL pointer, otherwise 0
 */
int LsbEncoder::Embed(uint8 *codeword, const uint8 *data) {
  if ( !codeword )
    throw std::invalid_argument("LsbEncoder::embed: 'codeword' is NULL");
  if ( !data )
    throw std::invalid_argument("LsbEncoder::embed: 'data' is NULL");
  memcpy(codeword, data, data_block_size_);
  return 0;

}

/**
 * @brief Transform data with Lsb decoder
 *
 * Decode data previously obtained from carrier file (stored in 'codework') into user data (steganographic information),
 * stored in buffer 'data', with Lsb decoder.
 * Warning: This function doesn't be called directly!
 * The size of input/output buffers is given intrinsic based on the configuration of the encoder instance.
 * Function is called from CarrierFile->extractBufferUsingEncoder when translating data from carrier file into user data.
 *
 * @param[in] codeword Initialized buffer with encoded user data
 * @param[out] data Buffer with decoded user data
 * @return Exception std::invalid_argument, if 'codeword' or 'data' are NULL pointer, otherwise 0
 */
int LsbEncoder::Extract(const uint8 *codeword, uint8 *data) {
  if ( !codeword )
    throw std::invalid_argument("LsbEncoder::embed: 'codeword' is NULL");
  if ( !data )
    throw std::invalid_argument("LsbEncoder::embed: 'data' is NULL");
  memcpy(data, codeword, codeword_block_size_);
  return 0;
}

/**
 * @brief Set parameter param of the encoder with value val
 *
 * This method set value of an encoder's parameter.
 * Both of the arguments must be of the string type.
 *
 * List of supported parameters for Lsb encoder (purpose of each parameter see constructor of the class):
 *     "block_size", value (as string) from range <GetBlockSizeMin(); GetBlockSizeMax()>
 *
 * @param[in] param Name of the parameter to be set
 * @param[in] val Value of the parameter to be set; must be of the string type
 * @return For all inputs throws exception std::invalid_argument if length of 'param' or 'val' is zero
 * @return For all parameters throws exception std::out_of_range if 'param' is not valid parameter for this class
 * @return For parameter "block_size" throws exceptions:
 *     - std::invalid_argument, if 'val' is not a number
 *     - std::out_of_range, if 'val' is not from range <GetBlockSizeMin(); GetBlockSizeMax()>, or is not a power of two
 */
void LsbEncoder::SetArgByName(const string &param, const string &val) {
  bool is_valid_param = false;
  string p = param;

  if (!param.size())
    throw std::invalid_argument("LsbEncoder::setArgByName: 'param' is empty");
  if (!val.size())
    throw std::invalid_argument("LsbEncoder::setArgByName: 'val' is empty");

  std::transform(p.begin(), p.end(), p.begin(), ::tolower);

  // parameter "block_size"
  if (p.compare("blocksize") == 0) {
    int block_size;

    is_valid_param = true;
    try { block_size = stoi(val); }
    catch (const std::invalid_argument& ) { throw; }
    catch (const std::out_of_range& ) { throw; }

    if (block_size < 0)
      throw std::out_of_range("LsbEncoder::setArgByName: 'block_size' "
                              "should be positive, is " +
                              to_string(static_cast<uint64>(block_size)));

    try { Init(block_size); }
    catch (const std::out_of_range& ) { throw; }
  }

  // other parameters...

  // if is not valid parameter
  if (is_valid_param == false)
    throw std::invalid_argument("LsbEncoder::setArgByName: "
                                "unknown class parameter '" +
                                param + "'");
}

} // stego_disk
