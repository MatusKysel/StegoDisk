//
//  CarrierFileBMP.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 1/6/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include "CarrierFiles/CarrierFileBMP.h"
#include "Utils/StegoErrors.h"

//TODO: Prepisat nacitavanie a ukladanie -> osetrit pripad ak sa medzi naloadovanim a saveovanim zmeni subor (velkost, hlavicka..)

namespace StegoDisk {

CarrierFileBMP::CarrierFileBMP(File file, std::shared_ptr<Encoder> encoder) : CarrierFile(file, encoder)
{
    auto filePtr = file.open();
    
    char bmpheader[14];
    char bmpinfo[40];
    
    fseek(filePtr.get(), 0, SEEK_SET);
    int read_cnt = (int)fread(&bmpheader, 1, 14, filePtr.get());
    read_cnt += (int)fread(&bmpinfo, 1, 40, filePtr.get());
    
    if (read_cnt < 54) {
        //TODO: throw exception
        return;
    }
    
    uint32_t bmpFileSize = *((uint32_t*)&bmpheader[2]);
    if (bmpFileSize != _file.getSize()) { // wrong file size
        //TODO: throw exception
        return;
    }
    
    _bmpOffset = *((uint32_t*)&bmpheader[10]); // should be = 54
    uint16_t bmpResolution = *((uint16_t*)&bmpinfo[14]); // 24
    uint32_t bmpCompression = *((uint32_t*)&bmpinfo[16]); // should be 0

    if (bmpCompression != 0) {
        //TODO: throw exception
        return;
    }
    
    if (bmpResolution != 24) {
        //TODO: throw exception
        return;
    }

    //_bmpSize = *((uint32_t*)&bmpinfo[20]); // byva = 0 pri vypnutej kompresii
    
    int32_t bmpWidth = *((int32_t*)&bmpinfo[4]);
    int32_t bmpHeight = *((int32_t*)&bmpinfo[8]);
    
    _bmpSize = (((24*bmpWidth+31)/32) * 4) * abs(bmpHeight);
    
    if ((_bmpSize+54) > bmpFileSize) {
        //TODO: throw exception
        return;
    }
    
    _rawCapacity = (_bmpSize / 8);
}



int CarrierFileBMP::loadFile()
{

    if (_fileLoaded) return STEGO_NO_ERROR;

    auto filePtr = _file.open();

    LOG_INFO("Loading file " << _file.getRelativePath());

    if (_permutation->getSize() == 0) {
        _permutation->init(_rawCapacity*8, subkey);
    }

    _buffer.resize(_rawCapacity);
    _buffer.clear();

    // TODO: rewrite using memory buffer & exceptions!
    MemoryBuffer bitmapBuffer(_rawCapacity*8);

    uint64 bitsToModify = _blocksUsed*_codewordBlockSize*8;

    fseek(filePtr.get(), _bmpOffset, SEEK_SET);
    uint32 read_cnt = (uint32)fread(bitmapBuffer.getRawPointer(), 1, _rawCapacity*8, filePtr.get());
    
    if (read_cnt != _rawCapacity*8) {
        LOG_ERROR("Unable to read file.");
        //TODO: throw exception
        return SE_READ_FILE;
    }
    
    // copy LSB data to content buffer

    for (uint64 i=0;i<bitsToModify;i++)
    {
        if (bitmapBuffer[i] & 0x01) setBitInBufferPermuted(i);
    }

//    bitmapBuffer.destroy();

    extractBufferUsingEncoder();
    
    _fileLoaded = true;
    
    LOG_INFO("File " << _file.getRelativePath() << " loaded");
    
    return STEGO_NO_ERROR;
}


int CarrierFileBMP::saveFile()
{
    auto filePtr = _file.open();
    //if (!_fileLoaded) return SE_LOAD_FILE;

    LOG_INFO("Saving file " << _file.getRelativePath());

    if (_permutation->getSize() == 0) {
        _permutation->init(_rawCapacity*8, subkey);
    }

    _buffer.resize(_rawCapacity);
    _buffer.clear();

    MemoryBuffer bitmapBuffer(_rawCapacity*8);

    uint64 bitsToModify = _blocksUsed*_codewordBlockSize*8;

    fseek(filePtr.get(), _bmpOffset, SEEK_SET);
    uint32 read_cnt = (uint32)fread(bitmapBuffer.getRawPointer(), 1, _rawCapacity*8, filePtr.get());

    if (read_cnt != _rawCapacity*8) {
        LOG_ERROR("Unable to read file.");
        //TODO: throw exception
        return SE_READ_FILE;
    }
    // copy LSB data to content buffer

    for (int i=0; i<bitsToModify; i++)
    {
        if (bitmapBuffer[i] & 0x01) setBitInBufferPermuted(i);
    }

    embedBufferUsingEncoder();

    uint8 tmpLSB = 0;
    for (int i=0; i<bitsToModify ;i++)
    {
        tmpLSB = getBitInBufferPermuted(i);
        bitmapBuffer[i] = (bitmapBuffer[i] & 0xFE) | tmpLSB;
    }


    // write data
    
    fseek(filePtr.get(), _bmpOffset, SEEK_SET);
    uint32 write_cnt = (uint32)fwrite(bitmapBuffer.getRawPointer(), 1, _rawCapacity*8, filePtr.get());

//    bitmapBuffer.destroy();
    
    if (write_cnt != _rawCapacity*8) {
        LOG_ERROR("Writing content to file failed.");
        //TODO: throw exception
        return SE_WRITE_FILE;
    }
    
    LOG_INFO("File " << _file.getRelativePath() << " saved");
    
    return STEGO_NO_ERROR;
}

}
