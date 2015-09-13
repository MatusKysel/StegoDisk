//
//  Encoder.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/21/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__Encoder__
#define __StegoFS__Encoder__

#include <memory>
#include <string>

#include "Utils/StegoTypes.h"

using namespace std;

namespace StegoDisk {

class Encoder {

public:
    Encoder();
    virtual ~Encoder();
    virtual int embed(uint8* codeword, const uint8* data) = 0;
    virtual int extract(const uint8* codeword, uint8* data) = 0;

    virtual uint32 getDataBlockSize();
    virtual uint32 getCodewordBlockSize();
    virtual void setArgByName(const string &arg, const string &val) = 0;

    virtual const string getNameInstance() const = 0;
    //static const string getName();
    virtual shared_ptr<Encoder> getNewInstance() = 0;
    //static shared_ptr<Encoder> getNew();

protected:
    uint32 _codewordBlockSize;
    uint32 _dataBlockSize;
};

}

#endif /* defined(__StegoFS__Encoder__) */
