//
//  LsbEncoder.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/21/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__LsbEncoder__
#define __StegoFS__LsbEncoder__

#include "Encoders/Encoder.h"

#define ENCODER_LSB_CODE_NAME_DEF "Lsb"

namespace StegoDisk {

class LsbEncoder : public Encoder {
    
public:
    LsbEncoder();
    LsbEncoder(uint32 blockSize);

    int embed(uint8 *codeword, const uint8 *data);
    int extract(const uint8 *codeword, uint8 *data);

    void setArgByName(const string &arg, const string &val);

    static shared_ptr<Encoder> getNew();
    shared_ptr<Encoder> getNewInstance();
    static const string getName();
    const string getNameInstance() const;

    static uint32 getBlockSizeMin();
    static uint32 getBlockSizeMax();

private:
    void init(uint32 blockSize);

    static const uint32 ENCODER_LSB_BLOCK_SIZE_MIN = 1;
    static const uint32 ENCODER_LSB_BLOCK_SIZE_MAX = 1024;
    static const uint32 ENCODER_LSB_DEFAULT_BLOCK_SIZE = 1;
    static const string ENCODER_LSB_CODE_NAME; 
};

}

#endif /* defined(__StegoFS__LsbEncoder__) */
