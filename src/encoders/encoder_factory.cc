#include "encoder_factory.h"

#include <algorithm>

#include "encoder.h"
#include "hamming_encoder.h"
#include "lsb_encoder.h"

namespace stego_disk {

/**
 * @brief Get default encoder instance
 *
 * Method Get default encoder instance.
 * This is Hamming encoder with parameter 5.
 * For default settings choice see class HammingEncoder.
 *
 * @return Instance of default encoder.
 */
std::shared_ptr<Encoder> EncoderFactory::GetDefaultEncoder() {
  return GetEncoder(kDefaultEncoder);
}


EncoderFactory::EncoderType EncoderFactory::GetDefaultEncoderType() {
  return kDefaultEncoder;
}

/**
 * @brief Get vector of instances of all the encoders using all possible settings
 *
 * This method Get vector of instances of all the encoder supported by this library.
 * For each encoder are used all possible settings:
 *     - HammingEncoder: 6 instances with parity from 3 to 8
 *     - LsbEncoder: 11 instances with data block size 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, and 1024 Bytes.
 *
 * @return Vector of all supported encoders
 */
vector<std::shared_ptr<Encoder>> EncoderFactory::GetAllEncoders() {
  vector<std::shared_ptr<Encoder>> list;

  // HammingEncoder
  // Create instances of HammingEncoder using all possible settings
  for (int i = HammingEncoder::GetParityBitsMin();
       i <= HammingEncoder::GetParityBitsMax(); ++i) {
    list.push_back(make_shared<HammingEncoder>(i));
  }
  // LsbEncoder
  // Create instance of LsbEncoder, blockLength = 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
  for (unsigned int i = LsbEncoder::GetBlockSizeMin();
       i <= LsbEncoder::GetBlockSizeMax(); i <<= 1) {
    list.push_back(make_shared<LsbEncoder>(i));
  }

  // Other encoders ...

  return list;

}

/**
 * @brief Set parameter of an encoder by arg name
 *
 * This method set parameter 'param' of 'encoder' based on the name of this parameter.
 * Invokes method encoder->setArgByName(string,string), see documentation of class Encoder
 * If an exception occured, it's passed throw
 *
 * @return A std::exception if some error occurs in encoder->setArgByName()
 */
void EncoderFactory::SetEncoderArg(std::shared_ptr<Encoder> encoder,
                                         const string &param,
                                         const string &val) {
  if (!encoder)
    throw std::invalid_argument("EncoderFactory::setEncoderArgByName: "
                                "arg 'encoder' is null");
  if (!param.size())
    throw std::invalid_argument("EncoderFactory::setEncoderArgByName: "
                                "arg 'param' is empty");
  if (!val.size())
    throw std::invalid_argument("EncoderFactory::setEncoderArgByName: "
                                "arg 'val' is empty");

  try { encoder->SetArgByName(param,val); }
  catch (...) { throw; }
}

/**
 * @brief Get vector of instances of all the encoders
 *
 * This method Get vector of instances of all the encoder supported by this library.
 * Each encoder is in the vector only once,
 * initialized with default values of parameters (i.e. block length).
 *
 * @return Vector of supported encoders
 */
vector<std::shared_ptr<Encoder>> EncoderFactory::GetEncoders() {
  vector<std::shared_ptr<Encoder>> list;
  list.push_back(make_shared<LsbEncoder>());
  list.push_back(make_shared<HammingEncoder>());
  return list;
}

/**
 * @brief Get names of all encoders supported by this library
 *
 * Returns in vector names of all encoders supported by this library.
 * Method not returns constant strings, because it is possible make some transformation of names,
 * e.g. to lower-case or to upper-case.
 *
 * For now, there are supported two encoders:
 * 1. LsbEncoder with code name "Lsb"
 * 2. HammingEncoder with code name "Hamming"
 *
 * @return vector of names
 */
vector<string> EncoderFactory::GetEncoderNames() {
  vector<string> list;

  list.push_back(HammingEncoder::GetName());
  list.push_back(LsbEncoder::GetName());

  return list;
}

/**
 * @brief Get instance of the encoder by type
 *
 * This method Get instance of the encoder by the encoder's type.
 * If those encoder exists, it is initialized with default parameters.
 * If there is no encoder with the encoder's type, returns a nullptr.
 *
 * @param[in] encoder Type of encoder to be created
 * @return On success instance of encoder, nullptr otherwise
 */

std::shared_ptr<Encoder> EncoderFactory::GetEncoder(const EncoderType encoder) {

  switch(encoder) {
    case EncoderType::LSB:
      return std::make_shared<LsbEncoder>();
    case EncoderType::HAMMING:
      return std::make_shared<HammingEncoder>();
    default:
      return nullptr;
  }
}


EncoderFactory::EncoderType EncoderFactory::GetEncoderType(const std::string &encoder) {
  std::string l_encoder(encoder.size(), '\0');
  std::transform(encoder.begin(), encoder.end(), l_encoder.begin(), ::tolower);

  if (l_encoder == "lsb") {
    return EncoderType::LSB;
  } else if (l_encoder == "hamming"){
    return EncoderType::HAMMING;
  } else {
    return GetDefaultEncoderType();
  }
}

const std::string EncoderFactory::GetEncoderName(const EncoderType encoder) {
  if (encoder == EncoderType::LSB) {
    return "lsb";
  } else {
    return "hamming";
  }
}

} // stego_disk
