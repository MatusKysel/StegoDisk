//
//  CarrierFileJPEG.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 1/9/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include <errno.h>
//#include <turbojpeg.h>
#include <iostream>
#include "Utils/StegoErrors.h"
#include "Utils/StegoMath.h"
#include "CarrierFiles/CarrierFileJPEG.h"

#define EXIT_SUCCESS 0

namespace StegoDisk {

CarrierFileJPEG::CarrierFileJPEG(File file, std::shared_ptr<Encoder> encoder) : CarrierFile(file, encoder)
{
    computeCapacity();
}

int CarrierFileJPEG::computeCapacity()
{
	// TODO: test if file is opened OK
    auto filePtr = _file.open();

    LOG_TRACE("Computing capacity of file " << _file.getSize());

    struct jpeg_decompress_struct cinfo_decompress;
    struct jpeg_error_mgr jerr_decompress;
    jvirt_barray_ptr* coeff_arrays;

    // Create decompression object.
    cinfo_decompress.err = jpeg_std_error(&jerr_decompress);
    jpeg_create_decompress(&cinfo_decompress);

    fseek(filePtr.get(), 0, SEEK_SET);
    jpeg_stdio_src(&cinfo_decompress, filePtr.get());

    // Get all compression parameters.
    jpeg_read_header(&cinfo_decompress, TRUE);

    // Read coefficients.
    coeff_arrays = jpeg_read_coefficients(&cinfo_decompress);


    JBLOCKARRAY jpegBlockBuffer;
    JCOEFPTR blockptr;
    jpeg_component_info* compptr;

    JDIMENSION ci, by, bx, bi;

    uint32 capacityInBits = 0;

    for (ci=0; ci<3; ci++)
    {
        compptr = cinfo_decompress.comp_info + ci;
        for (by=0; by<compptr->height_in_blocks; by++)
        {
            jpegBlockBuffer = (cinfo_decompress.mem->access_virt_barray)((j_common_ptr)&cinfo_decompress, coeff_arrays[ci], by, (JDIMENSION)1, FALSE);
            for (bx=0; bx<compptr->width_in_blocks; bx++)
            {
                blockptr = jpegBlockBuffer[0][bx];
                for (bi=1; bi<64; bi++) // skip the first coeff (DC) - first index is 1 (first AC coeff)
                {
                    if (blockptr[bi] & 0xFFFE) capacityInBits++; // ignore coeffs 0,1
                }
            }
        }
    }

    _rawCapacity = (capacityInBits / 8);
    //setencoder(_encoder); // sets _blockCount & _capacity using encoder settings & _rawCapacity

    jpeg_finish_decompress(&cinfo_decompress);
    jpeg_destroy_decompress(&cinfo_decompress);

    return 0;

}

int CarrierFileJPEG::loadFile()
{
    if (_fileLoaded) return STEGO_NO_ERROR;
    
    auto filePtr = _file.open();

    //LOG_TRACE("CarrierFileJPEG::loadFile: Loading file " << _file.getRelativePath());
    
    if (_permutation->getSize() == 0) {
        _permutation->init(_rawCapacity*8, subkey);
    }

    _buffer.resize(_rawCapacity);
    _buffer.clear();
    
    struct jpeg_decompress_struct cinfo_decompress;
    struct jpeg_error_mgr jerr_decompress;
    jvirt_barray_ptr* coeff_arrays;

    // Create decompression object.
    cinfo_decompress.err = jpeg_std_error(&jerr_decompress);
    jpeg_create_decompress(&cinfo_decompress);
    
    fseek(filePtr.get(), 0, SEEK_SET);
    jpeg_stdio_src(&cinfo_decompress, filePtr.get());
    
    // Get all compression parameters.
    jpeg_read_header(&cinfo_decompress, TRUE);
    
    // Read coefficients.
    coeff_arrays = jpeg_read_coefficients(&cinfo_decompress);
    
    
    JBLOCKARRAY jpegBlockBuffer;
    JCOEFPTR blockptr;
    jpeg_component_info* compptr;
    
    JDIMENSION ci, by, bx, bi;

    // read data
    
    uint32 coeffCounter = 0;

    boolean shouldRead = true;

    uint64 bitsToModify = _permutation->getSize();//_blocksUsed*_codewordBlockSize*8;

    LOG_TRACE("CarrierFileJPEG::loadFile: file " << _file.getRelativePath() << ", bits to modify: " << bitsToModify);

    for (ci=0; ci<3; ci++)
    {
        compptr = cinfo_decompress.comp_info + ci;
        for (by=0; (by<compptr->height_in_blocks) && shouldRead; by++)
        {
            jpegBlockBuffer = (cinfo_decompress.mem->access_virt_barray)((j_common_ptr)&cinfo_decompress, coeff_arrays[ci], by, (JDIMENSION)1, FALSE);
            for (bx=0; (bx<compptr->width_in_blocks) && shouldRead; bx++)
            {
                blockptr = jpegBlockBuffer[0][bx];
                for (bi=1; (bi<64) && shouldRead; bi++) // skip the first coeff (DC) - first index is 1 (first AC coeff)
                {
                    if (blockptr[bi] & 0xFFFE) { // ignore 0, 1 coeffs

                        if ((blockptr[bi] & 0x1)) {
                            setBitInBufferPermuted(coeffCounter);
                        }
                        coeffCounter++;
                        if (coeffCounter >= bitsToModify) shouldRead = false;

                    }
                }
            }
        }
    }

    LOG_TRACE(_file.getRelativePath() << ", coeff_counter:" << coeffCounter);
    
    LOG_TRACE(_file.getRelativePath() << ", unpacked buffer: " << StegoMath::hexBufferToStr(_buffer.getRawPointer(), 10));

    if (extractBufferUsingEncoder()) {
        LOG_ERROR("CarrierFileJPEG::loadFile: file " << _file.getRelativePath() << ", saving: buffer extracting failed!");
    }

    _fileLoaded = true;
    
    jpeg_finish_decompress(&cinfo_decompress);
    jpeg_destroy_decompress(&cinfo_decompress);
    
    LOG_TRACE("CarrierFileJPEG::loadFile: file " << _file.getRelativePath() << " loaded");
    
    return STEGO_NO_ERROR;
}

int CarrierFileJPEG::saveFile()
{
    
    auto filePtr = _file.open();
    //if (!_fileLoaded) return SE_LOAD_FILE;
    
    LOG_TRACE("Saving file " << _file.getRelativePath());

    if (_permutation->getSize() == 0) {
        _permutation->init(_rawCapacity*8, subkey);
    }

    _buffer.resize(_rawCapacity);
    _buffer.clear();

    // JPEG LOADING PHASE -------------------------------------------
    
    struct jpeg_decompress_struct cinfo_decompress;
    struct jpeg_error_mgr jerr_decompress;
    jvirt_barray_ptr* coeff_arrays;
    
    // Create decompression object.
    cinfo_decompress.err = jpeg_std_error(&jerr_decompress);
    jpeg_create_decompress(&cinfo_decompress);
    
    fseek(filePtr.get(), 0, SEEK_SET);
    jpeg_stdio_src(&cinfo_decompress, filePtr.get());
    
    // save markers (exif data etc)
    jpeg_save_markers (&cinfo_decompress, JPEG_COM, 0xffff);
    for (int i=0;i<16;i++) {
        jpeg_save_markers (&cinfo_decompress, JPEG_APP0+i, 0xffff);
    }
    // Get all compression parameters.
    jpeg_read_header(&cinfo_decompress, TRUE);
    
    // Read coefficients.
    coeff_arrays = jpeg_read_coefficients(&cinfo_decompress);

    
    // COEF MODIFICATION PHASE ------------------------------------
    
    JBLOCKARRAY jpegBlockBuffer;
    JCOEFPTR blockptr;
    jpeg_component_info* compptr;
    
    JDIMENSION ci, by, bx, bi;
    
    
    uint32 coeffCounter = 0;
    
    boolean shouldWrite = true;
    boolean shouldRead = true;

    
    uint32 bitsToModify = (uint32)_permutation->getSize();//_blocksUsed*_codewordBlockSize*8;

    // LOG_INFO(_relativePath << ", bits to modify: " << bitsToModify);

    // read LSBs from DCT coefficient and store them in temporary buffer in "locally" permuted order

    for (ci=0; ci<3; ci++)
    {
        compptr = cinfo_decompress.comp_info + ci;
        for (by=0; (by<compptr->height_in_blocks) && shouldRead; by++)
        {
            jpegBlockBuffer = (cinfo_decompress.mem->access_virt_barray)((j_common_ptr)&cinfo_decompress, coeff_arrays[ci], by, (JDIMENSION)1, FALSE);
            for (bx=0; (bx<compptr->width_in_blocks) && shouldRead; bx++)
            {
                blockptr = jpegBlockBuffer[0][bx];
                for (bi=1; (bi<64) && shouldRead; bi++) // skip the first coeff (DC) - first index is 1 (first AC coeff)
                {
                    if (blockptr[bi] & 0xFFFE) { // ignore 0, 1 coeffs

                        if ((blockptr[bi] & 0x1)) {
                            setBitInBufferPermuted(coeffCounter);
                        }
                        coeffCounter++;
                        if (coeffCounter >= bitsToModify) shouldRead = false;

                    }
                }
            }
        }
    }

    //LOG_INFO(_relativePath << ", reading finished coeff_counter:" << coeffCounter);

    // use encoder to embed "globally" permuted bytes of hidden storage to "locally" permuted LSBbits stored in temporary buffer

    //LOG_INFO(_relativePath << ", unpacked buffer: " << StegoMath::hexBufferToStr(_buffer, 10));

	// TODO: what with this error?
    if (embedBufferUsingEncoder()) {
        LOG_ERROR("CarrierFileJPEG::saveFile: '" << _file.getRelativePath() << "': buffer embedding failed!");
    }

    //LOG_INFO(_relativePath << ", embedded buffer: " << StegoMath::hexBufferToStr(_buffer, 10));


    // write down permuted and encoded LSBs into DCT coefficients

    coeffCounter = 0;
    uint8 tmpLSB = 0;

    for (ci=0; ci<3; ci++)
    {
        compptr = cinfo_decompress.comp_info + ci;
        for (by=0; (by<compptr->height_in_blocks) && shouldWrite; by++)
        {
            jpegBlockBuffer = (cinfo_decompress.mem->access_virt_barray)((j_common_ptr)&cinfo_decompress, coeff_arrays[ci], by, (JDIMENSION)1, FALSE);
            for (bx=0; (bx<compptr->width_in_blocks) && shouldWrite; bx++)
            {
                blockptr = jpegBlockBuffer[0][bx];
                for (bi=1; (bi<64) && shouldWrite; bi++)
                {
                    if (blockptr[bi] & 0xFFFE) { // ignore 0, 1 coeffs
                        


                        tmpLSB = getBitInBufferPermuted(coeffCounter);

                        blockptr[bi] = blockptr[bi] & 0xFFFE;
                        blockptr[bi] |= (tmpLSB & 0x01);

                        coeffCounter++;
                        if (coeffCounter >= bitsToModify) shouldWrite = false;

                        
                    }
                }
            }
        }
    }

    // LOG_INFO(_relativePath << ", writing finished coeff_counter:" << coeffCounter);
    
    // JPEG SAVING PHASE -------------------------------------------

    struct jpeg_compress_struct cinfo_compress;
    struct jpeg_error_mgr jerr_compress;
    jpeg_create_compress(&cinfo_compress);
    cinfo_compress.err = jpeg_std_error(&jerr_compress);
    fseek(filePtr.get(), 0, SEEK_SET);
    jpeg_stdio_dest(&cinfo_compress, filePtr.get());
    
    // set jpeg params
    jpeg_copy_critical_parameters(&cinfo_decompress, &cinfo_compress);
    
    // write coeffs
    jpeg_write_coefficients(&cinfo_compress, coeff_arrays);
    
    // write markers
    jpeg_saved_marker_ptr marker;
    for(marker = cinfo_decompress.marker_list; marker != NULL; marker = marker->next) {
        jpeg_write_marker(&cinfo_compress, marker->marker, marker->data, marker->data_length);
    }
    
    
    // CLEANUP PHASE -----------------------------------------------
    
    jpeg_finish_compress(&cinfo_compress);
    jpeg_destroy_compress(&cinfo_compress);
    
    jpeg_finish_decompress(&cinfo_decompress);
    jpeg_destroy_decompress(&cinfo_decompress);
    
    LOG_TRACE("CarrierFileJPEG::saveFile: file " << _file.getRelativePath() << " saved");
    
    //_fileLoaded = false;
    
    return STEGO_NO_ERROR;
}

int CarrierFileJPEG::getHistogram()
{
    /*
    if (loadFile()) return -1;
    
    JBLOCKARRAY jpegBlockBuffer;
    JCOEFPTR blockptr;
    jpeg_component_info* compptr;
    
    int ci, by, bx, bi;
    
    int histogram[0xFFFF];
    memset(histogram, 0, 4*0xFFFF);
    
    if (_cinfo_decompress.comp_info == NULL) return -1;
    
    int coefCount = 0;
    
    for (ci = 0; ci < 3; ci++)
    {
        compptr = _cinfo_decompress.comp_info + ci;
        for (by = 0; by < compptr->height_in_blocks; by++)
        {
            jpegBlockBuffer = (_cinfo_decompress.mem->access_virt_barray)((j_common_ptr)&_cinfo_decompress, _coeff_arrays_one[ci], by, (JDIMENSION)1, FALSE);
            for (bx = 0; bx < compptr->width_in_blocks; bx++)
            {
                blockptr = jpegBlockBuffer[0][bx];
                for (bi = 0; bi < 64; bi++)
                {
                    histogram[(uint16_t)blockptr[bi] ^ (rand() % 2)]++;
                    coefCount++;
                }
            }
        }
    }
    
    for (short i=-20;i<=20;i++) {
        printf("%7d, %7.3lf\n", (short)i,(double)histogram[(unsigned short)i]*100/(double)coefCount);
    }
    */
    return 0;
    
}

}
