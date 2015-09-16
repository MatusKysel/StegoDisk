#ifndef STEGODISK_ENCODERS_ENCODERFACTORY_H_
#define STEGODISK_ENCODERS_ENCODERFACTORY_H_

#include <memory>
#include <vector>

#include "encoder.h"
#include "utils/cpp11_compat.h"

using namespace std;

namespace stego_disk {
// no inheritance from this class
class EncoderFactory KEYWORD_FINAL
{
private:
  EncoderFactory();
public:
  // Get vector of all encoders (each encoder with all possible settings)
  static vector<std::shared_ptr<Encoder>> GetAllEncoders();
  // Get vector of all encoders (each encoder once with default settings)
  static vector<std::shared_ptr<Encoder>> GetEncoders();
  // Get names of all encoders supported by this library
  static vector<string> GetEncoderNames();
  // Get instance of encoder based on the his code name
  static std::shared_ptr<Encoder> GetEncoderByName(const string &encoder_name);
  // set encoder param by name
  static void SetEncoderArgByName(std::shared_ptr<Encoder> encoder,
                                  const string &param, const string &val);
  // Get instance of the default encoder
  static std::shared_ptr<Encoder> GetDefaultEncoder();
};

} // stego_disk

#endif // STEGODISK_ENCODERS_ENCODERFACTORY_H_
