//
//  HammingEncoder.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/21/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__HammingEncoder__
#define __StegoFS__HammingEncoder__

#include "Encoders/Encoder.h"

#define ENCODER_HAMMING_CODE_NAME_DEF "Hamming"

namespace StegoDisk {

class HammingEncoder : public Encoder {
    
public:
    HammingEncoder();
    HammingEncoder(uint32 parityBits);

    int embed(uint8 *codeword, const uint8 *data);
    int extract(const uint8 *codeword, uint8 *data);

    void setArgByName(const string &arg, const string &val);

    static const string getName();
    const string getNameInstance() const;
    static shared_ptr<Encoder> getNew();
    shared_ptr<Encoder> getNewInstance();

    static int getParityBitsMin();
    static int getParityBitsMax();

private:
    void init(uint32 parityBits);
    uint64 computeH(const uint8 *buffer);
    void swapBitInCodeword(uint8 *buffer, int bitIn);
    void writeBitsToBuffer(uint64 value, int offset, uint8 *buffer);
    uint64 readBitsFromBuffer(const uint8 *buffer, int offset, int length);
    
    uint32 _parityBits;
    uint32 _totalBits;
    
    uint32 _codewordsInBlock;
    uint32 _codewordBufferSize;

    static const uint32 ENCODER_HAMMING_PARITY_BITS_MIN = 3;
    static const uint32 ENCODER_HAMMING_PARITY_BITS_MAX = 8;
    static const string ENCODER_HAMMING_CODE_NAME;    
    // default block size
    static const uint32 ENCODER_HAMMING_DEFAULT_PARITY_BITS = 5;
};
    
}

#endif /* defined(__StegoFS__HammingEncoder__) */
