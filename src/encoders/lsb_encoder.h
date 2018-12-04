/**
* @file lsb_encoder.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief LSB encoder class
*
*/

#pragma once

#include "encoder.h"

#define ENCODER_LSB_CODE_NAME_DEF "Lsb"

namespace stego_disk {

class LsbEncoder : public Encoder {

public:
    LsbEncoder();
    LsbEncoder(uint32 block_size);

	virtual int Embed(uint8 *codeword, const uint8 *data) override;
	virtual int Extract(const uint8 *codeword, uint8 *data) override;

	virtual void SetArgByName(const std::string &arg, const std::string &val) override;

    static std::shared_ptr<Encoder> GetNew();
	virtual std::shared_ptr<Encoder> GetNewInstance() override;
    static const std::string GetName();
	virtual const std::string GetNameInstance() const override;

    static uint32 GetBlockSizeMin();
    static uint32 GetBlockSizeMax();

private:
    void Init(uint32 block_size);

    static const uint32 kEncoderLsbBlockSizeMin = 1;
    static const uint32 kEncoderLsbBlockSizeMax = 1024;
    static const uint32 kEncoderLsbDefaultBlockSize = 1;
    static const std::string kEncoderLsbCodeName;
};

} // stego_disk
