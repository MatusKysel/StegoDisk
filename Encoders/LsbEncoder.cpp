//
//  LsbEncoder.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/21/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include <algorithm>
#include <string.h>

#include "Encoders/LsbEncoder.h"
#include "Utils/StegoMath.h"
#include "Logging/logger.h"

namespace StegoDisk {

const string LsbEncoder::ENCODER_LSB_CODE_NAME = ENCODER_LSB_CODE_NAME_DEF;

/**
 * @brief Get minimal value for parameter 'blockSize'
 *
 * Get the minimal value for parameter 'blockSize' used by Lsb coder instance.
 * Minimal value is 1.
 * 
 * @return Minimal value required for parameter 'blockSize'
 */
uint32 LsbEncoder::getBlockSizeMin()
{
    return ENCODER_LSB_BLOCK_SIZE_MIN;
}

/**
 * @brief Get maximal value for parameter 'blockSize'
 *
 * Get the maximal value for parameter 'blockSize' used by Lsb coder instance.
 * Maximal value is 1024.
 * 
 * @return Maximal value required for parameter 'blockSize'
 */
uint32 LsbEncoder::getBlockSizeMax()
{
    return ENCODER_LSB_BLOCK_SIZE_MAX;
}

/**
 * @brief Return new instance of Lsb encoder
 *
 * Return New instance of Lsb encoder,
 * the return value is smart pointer.
 * Instance is created with default values of class parameters.
 * Static method!
 *
 * @return New instance of Lsb encoder class
 */
shared_ptr<Encoder> LsbEncoder::getNew() 
{
    return shared_ptr<Encoder>(new LsbEncoder());
}

/**
 * @brief Return new instance of Lsb encoder
 *
 * Return New instance of Lsb encoder,
 * the return value is smart pointer.
 * Instance is created with default values of class parameters.
 * Instance method! 
 *
 * @return New instance of Lsb encoder class
 */
shared_ptr<Encoder> LsbEncoder::getNewInstance() 
{
    return getNew();
}

/**
 * @brief Initialize data structures of Lsb encoder instance
 *
 * This is private function, used by class member functions.
 * Requires one argument - blockSize.
 * This argument must be in range <getBlockSizeMin(); getBlockSizeMax()>
 * If no, throws exception std::out_of_range.
 * This exception is involved in case if 'blockSize' is not power of two, too.
 * 
 * With Lsb encoder we can insert k bits of information into k bits of the stego-carrier.
 * 
 * @param[in] blockSize Must be in range <getBlockSizeMin(); getBlockSizeMax()> and power of two.
 * @return Throws exception std::out_of_range, if 'blockSize' is out of the range or isn't power of two.
 */
void LsbEncoder::init(uint32 blockSize)
{
    // is blockSize power  2?
    if (StegoMath::popcount(blockSize) != 1)
        throw std::out_of_range("LsbEncoder::init: 'blockSize' should be power of two, is " + std::to_string((uint64)blockSize));
    // is blockSize in range
    if ((blockSize < ENCODER_LSB_BLOCK_SIZE_MIN) || (blockSize > ENCODER_LSB_BLOCK_SIZE_MAX)) {
        string err = "LsbEncoder::init: 'blockSize' is " + std::to_string((uint64)blockSize);
        err += ", should be in <"+ENCODER_LSB_BLOCK_SIZE_MIN;
        err += "," + std::to_string((uint64)ENCODER_LSB_BLOCK_SIZE_MAX);
        err += ">";
        throw std::out_of_range(err);
    }
    _dataBlockSize = blockSize;
    _codewordBlockSize = blockSize;

    LOG_DEBUG("LsbEncoder::init: blockSize: " << blockSize << ", " <<
              "dataBlockSize: " << _dataBlockSize << ", " <<
              "codewordBlockSize: " << _codewordBlockSize);
}

/**
 * @brief Constructor of the Lsb encoder instance with default parameter
 *
 * Construct new instance of Lsb encoder with default parameter 'blockSize'.
 * This parameter is set to the default value = 1.
 * 
 * @return Instance of the Lsb encoder initialized with default values
 */
LsbEncoder::LsbEncoder()
{
	init(ENCODER_LSB_DEFAULT_BLOCK_SIZE);
}

/**
 * @brief Constructor of the Lsb encoder instance
 *
 * Requires one argument - blockSize.
 * This argument must be in range <getBlockSizeMin(); getBlockSizeMax()>.
 * If no, throws exception std::out_of_range.
 * This exception is involved in case if 'blockSize' is not power of two, too.
 *
 * With Lsb encoder we can insert k bits of information into k bits of the stego-carrier.
 * 
 * @param[in] blockSize Must be in range <getBlockSizeMin(); getBlockSizeMax()> and power of two.
 * @return Instance of the Lsb encoder initialized with parameter 'blockSize'
 */
LsbEncoder::LsbEncoder(uint32 blockSize)
{
    init(blockSize);
}

/**
 * @brief Get the code name of this encoder
 *
 * Get code name of Lsb encoder. The code name is "Lsb".
 * Static function, does not required any instance of the class.
 * 
 * @return Code name of Lsb encoder class
 */
const string LsbEncoder::getName()
{   
    return ENCODER_LSB_CODE_NAME;
}

/**
 * @brief Get the code name of this encoder
 *
 * Get code name of Lsb encoder. The code name is "Lsb".
 * member method, calls static function of the class.
 * 
 * @return Code name of Lsb encoder class
 */
const string LsbEncoder::getNameInstance() const
{   
    return LsbEncoder::getName();
}

/**
 * @brief Transform user data with Lsb encoder
 *
 * Encode user data (steganographic information) stored in buffer 'data' with Lsb encoder.
 * Warning: This function doesn't be called directly!
 * The size of input/output buffers is given intrinsic based on the configuration of the encoder instance.
 * Function is called from CarrierFile->embedBufferUsingEncoder when translating user stego data into carrier file.
 * 
 * @param[in] data Initialized buffer with user data
 * @param[out] codeword Buffer with encoded user data
 * @return Exception std::invalid_argument, if 'codeword' or 'data' are NULL pointer, otherwise 0
 */
int LsbEncoder::embed(uint8 *codeword, const uint8 *data)
{
    if ( !codeword )
        throw std::invalid_argument("LsbEncoder::embed: 'codeword' is NULL");
    if ( !data )
        throw std::invalid_argument("LsbEncoder::embed: 'data' is NULL");
    memcpy(codeword,data,_dataBlockSize);
    return 0;

}

/**
 * @brief Transform data with Lsb decoder
 *
 * Decode data previously obtained from carrier file (stored in 'codework') into user data (steganographic information),
 * stored in buffer 'data', with Lsb decoder.
 * Warning: This function doesn't be called directly!
 * The size of input/output buffers is given intrinsic based on the configuration of the encoder instance.
 * Function is called from CarrierFile->extractBufferUsingEncoder when translating data from carrier file into user data.
 * 
 * @param[in] codeword Initialized buffer with encoded user data
 * @param[out] data Buffer with decoded user data
 * @return Exception std::invalid_argument, if 'codeword' or 'data' are NULL pointer, otherwise 0
 */
int LsbEncoder::extract(const uint8 *codeword, uint8 *data)
{
    if ( !codeword )
        throw std::invalid_argument("LsbEncoder::embed: 'codeword' is NULL");
    if ( !data )
        throw std::invalid_argument("LsbEncoder::embed: 'data' is NULL");
    memcpy(data,codeword,_codewordBlockSize);
    return 0;
}

/**
 * @brief Set parameter param of the encoder with value val
 *
 * This method set value of an encoder's parameter.
 * Both of the arguments must be of the string type.
 * 
 * List of supported parameters for Lsb encoder (purpose of each parameter see constructor of the class):
 *     "blockSize", value (as string) from range <getBlockSizeMin(); getBlockSizeMax()>
 *
 * @param[in] param Name of the parameter to be set
 * @param[in] val Value of the parameter to be set; must be of the string type
 * @return For all inputs throws exception std::invalid_argument if length of 'param' or 'val' is zero
 * @return For all parameters throws exception std::out_of_range if 'param' is not valid parameter for this class
 * @return For parameter "blockSize" throws exceptions:
 *     - std::invalid_argument, if 'val' is not a number
 *     - std::out_of_range, if 'val' is not from range <getBlockSizeMin(); getBlockSizeMax()>, or is not a power of two
 */
void LsbEncoder::setArgByName(const string &param, const string &val) {
    bool isValidParam = false;
    string p = param;

    if (!param.size())
        throw std::invalid_argument("LsbEncoder::setArgByName: 'param' is empty");
    if (!val.size())
        throw std::invalid_argument("LsbEncoder::setArgByName: 'val' is empty");

    std::transform(p.begin(), p.end(), p.begin(), ::tolower);

    // parameter "blockSize"
    if (p.compare("blocksize") == 0) {
        int blockSize;

        isValidParam = true;
        try { blockSize = stoi(val); }
        catch (const std::invalid_argument& ) { throw; }
        catch (const std::out_of_range& ) { throw; }

        if (blockSize<0)
            throw std::out_of_range("LsbEncoder::setArgByName: 'blockSize' should be positive, is "+to_string((uint64)blockSize));

        try { init(blockSize); }
        catch (const std::out_of_range& ) { throw; }
    }

    // other parameters...

    // if is not valid parameter
    if (isValidParam == false)
        throw std::invalid_argument("LsbEncoder::setArgByName: unknown class parameter '"+param+"'");
}

}
