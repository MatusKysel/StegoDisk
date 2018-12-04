/**
* @file encoder.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Interface class for encoders
*
*/

#pragma once

#include <memory>
#include <string>
#include <stdexcept>

#include "api_mask.h"
#include "utils/stego_types.h"

namespace stego_disk {

class Encoder {

public:
  Encoder();
  virtual ~Encoder();
  virtual int Embed(uint8* codeword, const uint8* data) = 0;
  virtual int Extract(const uint8* codeword, uint8* data) = 0;

  virtual uint32 GetDataBlockSize();
  virtual uint32 GetCodewordBlockSize();
  virtual void SetArgByName(const std::string &arg, const std::string &val) = 0;

  virtual const std::string GetNameInstance() const = 0;
  //static const string getName();
  virtual std::shared_ptr<Encoder> GetNewInstance() = 0;
  //static shared_ptr<Encoder> getNew();

protected:
	uint32 codeword_block_size_{ 0 };
	uint32 data_block_size_{ 0 };
};

} // stego_disk
