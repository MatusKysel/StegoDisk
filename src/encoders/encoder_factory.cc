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
  // TODO after testing set HammingEncoder(5)
  //return std::shared_ptr<Encoder>(new HammingEncoder(5));
  return std::shared_ptr<Encoder>(new LsbEncoder(1));
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
void EncoderFactory::SetEncoderArgByName(std::shared_ptr<Encoder> encoder,
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
 * @brief Get instance of the encoder by the code name
 *
 * This method Get instance of the encoder by the encoder's code name.
 * If those encoder exists, it is initialized with default parameters.
 * If there is no encoder with the 'encoder_name', throws an exception.
 * Exception is raised if 'encoder_name' has zero length, too.
 *
 * @param[in] encoder_name Name of encoder to be created
 * @return On success instance of encoder, exception std::invalid_argument otherwise
 */
std::shared_ptr<Encoder> EncoderFactory::GetEncoderByName(
    const string &encoder_name) {

  vector<std::shared_ptr<Encoder>> list;
  std::shared_ptr<Encoder> encoder(nullptr);
  string str, name;

  if (encoder_name.length() == 0)
    throw std::invalid_argument("EncoderFactory::GetEncoderByName: "
                                "'encoder_name' is empty");
  name = encoder_name;

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  list = GetEncoders();
  for (unsigned int i = 0; i < list.size(); ++i) {
    str = list[i]->GetNameInstance();
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (name.compare(str) == 0) {
      encoder = list[i];
      break;
    }
  }

  if (encoder)
    return encoder;
  throw std::invalid_argument("EncoderFactory::GetEncoderByName: "
                              "encoder with name '"
                              + encoder_name + "' doesn't exist");
}

} // stego_disk
