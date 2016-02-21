/**
* @file lsb_encoder.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief LSB encoder class
*
*/

#ifndef STEGODISK_ENCODERS_LSBENCODER_H_
#define STEGODISK_ENCODERS_LSBENCODER_H_

#include "encoder.h"

#define ENCODER_LSB_CODE_NAME_DEF "Lsb"

namespace stego_disk {

class LsbEncoder : public Encoder {

public:
    LsbEncoder();
    LsbEncoder(uint32 block_size);

    int Embed(uint8 *codeword, const uint8 *data);
    int Extract(const uint8 *codeword, uint8 *data);

    void SetArgByName(const string &arg, const string &val);

    static shared_ptr<Encoder> GetNew();
    shared_ptr<Encoder> GetNewInstance();
    static const string GetName();
    const string GetNameInstance() const;

    static uint32 GetBlockSizeMin();
    static uint32 GetBlockSizeMax();

private:
    void Init(uint32 block_size);

    static const uint32 kEncoderLsbBlockSizeMin = 1;
    static const uint32 kEncoderLsbBlockSizeMax = 1024;
    static const uint32 kEncoderLsbDefaultBlockSize = 1;
    static const string kEncoderLsbCodeName;
};

} // stego_disk

#endif // STEGODISK_ENCODERS_LSBENCODER_H_
