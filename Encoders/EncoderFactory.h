#ifndef ENCODERFACTORY_H
#define ENCODERFACTORY_H

#include <memory>
#include <vector>
#include "Encoders/Encoder.h"
#include "Utils/Cpp11Compat.h"
using namespace std;

namespace StegoDisk {
// no inheritance from this class
class EncoderFactory KEYWORD_FINAL
{
private:
    EncoderFactory();
public:
    // get vector of all encoders (each encoder with all possible settings)
    static vector<std::shared_ptr<Encoder>> getAllEncoders();
    // get vector of all encoders (each encoder once with default settings)
    static vector<std::shared_ptr<Encoder>> getEncoders();
    // get names of all encoders supported by this library
    static vector<string> getEncoderNames();
    // get instance of encoder based on the his code name
    static std::shared_ptr<Encoder> getEncoderByName(const string &name);
    // set encoder param by name
    static void setEncoderArgByName(std::shared_ptr<Encoder> encoder, const string &param, const string &val);
    // get instance of the default encoder
    static std::shared_ptr<Encoder> getDefaultEncoder();
};

}

#endif // ENCODERFACTORY_H
