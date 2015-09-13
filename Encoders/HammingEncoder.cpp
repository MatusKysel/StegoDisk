//
//  HammingEncoder.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/21/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include <string.h>
#include <algorithm>

#include "Encoders/HammingEncoder.h"
#include "Utils/StegoMath.h"
#include "Utils/StegoErrors.h"
#include "Logging/logger.h"

namespace StegoDisk {

const string HammingEncoder::ENCODER_HAMMING_CODE_NAME = ENCODER_HAMMING_CODE_NAME_DEF;

/**
 * @brief Get minimal value for parameter 'parityBits'
 *
 * Get the minimal value for parameter 'parityBits' used by Hamming coder instance.
 * Minimal value is 3.
 * 
 * @return Minimal value required for parameter 'parityBits'
 */
int HammingEncoder::getParityBitsMax()
{
    return ENCODER_HAMMING_PARITY_BITS_MAX;
}

/**
 * @brief Get maximal value for parameter 'parityBits'
 *
 * Get the maximal value for parameter 'parityBits' used by Hamming coder instance.
 * Maximal value is 8.
 * 
 * @return Maximal value required for parameter 'parityBits'
 */
int HammingEncoder::getParityBitsMin()
{
    return ENCODER_HAMMING_PARITY_BITS_MIN;
}

/**
 * @brief Return new instance of Hamming encoder
 *
 * Return New instance of Hamming encoder,
 * the return value is smart pointer.
 * Instance is created with default values of class parameters.
 * Static method!
 *
 * @return New instance of Hamming encoder class
 */
shared_ptr<Encoder> HammingEncoder::getNew() 
{
    return shared_ptr<Encoder>(new HammingEncoder());
}

/**
 * @brief Return new instance of Hamming encoder
 *
 * Return New instance of Hamming encoder,
 * the return value is smart pointer.
 * Instance is created with default values of class parameters.
 * Instance method!
 *
 * @return New instance of Hamming encoder class
 */
shared_ptr<Encoder> HammingEncoder::getNewInstance() 
{
    return getNew();
}

/**
 * @brief Initialize data structures of Hamming encoder instance
 *
 * This is private function, used by member functions.
 * Requires one argument - parityBits.
 * This argument must be in range <getParityBitsMin(); getParityBitsMax()>
 * If no, throws exception std::out_of_range
 * 
 * With Hamming encoder we can insert k bits of information into 2^k-1 bits of the stego-carrier.
 * The parameter 'parityBits' is 'k'.
 * So, i.e. for k=3 we insert 3 bits of information into 7 bits of cover, for k=8 we need 255 bits of cover.
 * Thus the embedding efficiency is k/(2^k-1), and for values in <3;8> is (in % of usable capacity of the cover medium):
 *    3 --> 42%, 4 --> 27%, 5 --> 16%, 6 --> 9.5%, 7 --> 5.5%, 8 --> 3.1%
 * 
 * But one great advantage of Hamming encoder is, that for insertion of k bits into 2^k-1 bits we need modify at most 1 bit.
 * So for values of k in <3;8> we change at most (in % of usable capacity of the cover medium):
 *    3 --> 14%, 4 --> 6.7%, 5 --> 3.2%, 6 --> 1.6%, 7 --> 0.8%, 8 --> 0.4%
 * We see, that increasing parameter k we increase the security, in terms of possibility of the statistical stegoanalysis attacks.
 * 
 * @param[in] parityBits Must be in range <getParityBitsMin(); getParityBitsMax()>
 * @return Throws exception std::out_of_range, if 'parityBits' is out of the range
 */
void HammingEncoder::init(uint32 parityBits) {
    if ((parityBits < ENCODER_HAMMING_PARITY_BITS_MIN) || (parityBits > ENCODER_HAMMING_PARITY_BITS_MAX)) {
        string err = "HammingEncoder::init: 'parityBits' is " + std::to_string((uint64)parityBits);
        err += ", should be in <"+ENCODER_HAMMING_PARITY_BITS_MIN;
        err += "," + std::to_string((uint64)ENCODER_HAMMING_PARITY_BITS_MAX);
        err += ">";
        throw std::out_of_range(err);
    }

    // parity bits = k
    // total bits = 2^k - 1
    // data bits = 2^k - 1 - k
    // note: this embedder uses parity bits as data storage!
    
    // Hamming code params
    _parityBits = parityBits;
    /*
    _totalBits = 4;
    for (int i=2;i<_parityBits;i++) 
        _totalBits *= 2;
    _totalBits--;
    */
    _totalBits = (uint32)(1L << _parityBits) - 1;
    
    // in/out block params
    _dataBlockSize = (uint32)StegoMath::lcm(_parityBits, 8) / 8;
    _codewordsInBlock = (_dataBlockSize*8) / _parityBits;
    _codewordBufferSize = (_totalBits-1) / 8 + 1;
    _codewordBlockSize = _codewordBufferSize*_codewordsInBlock;

    LOG_DEBUG("HammingEncoder::init: parity bits: " << _parityBits << ", " <<
              "total bits: " << _totalBits << ", " <<
              "dataBlockSize: " << _dataBlockSize << ", " <<
              "codewordsInBlock: " << _codewordsInBlock << ", " <<
              "codeword buffer size: " << _codewordBufferSize);
}

/**
 * @brief Constructor of the Hamming encoder instance with default parameter
 *
 * Construct new instance of Hamming encoder with default parameter 'parityBits'.
 * This parameter is set to the default value = 5.
 * Why 5? See description of parametric constructor...
 * Yes, if we use parityBits=5, we can use only 16% of the cover media capacity,
 * but we modify at most 3.2% of this media, which is below 5% border of the significance
 * of statistical hypothesis testing in case of some steganalysis attack.
 * 
 * @return Instance of the Hamming encoder initialized with default values
 */
HammingEncoder::HammingEncoder()
{
	init(ENCODER_HAMMING_DEFAULT_PARITY_BITS);
}

/**
 * @brief Constructor of the Hamming encoder instance
 *
 * Requires one argument - parityBits.
 * This argument must be in range <getParityBitsMin(); getParityBitsMax()>
 * If no, throws exception std::out_of_range
 *
 * With Hamming encoder we can insert k bits of information into 2^k-1 bits of the stego-carrier.
 * The parameter 'parityBits' is 'k'.
 * So, i.e. for k=3 we insert 3 bits of information into 7 bits of cover, for k=8 we need 255 bits of cover.
 * Thus the embedding efficiency is k/(2^k-1), and for values in <3;8> is (in % of usable capacity of the cover media):
 *    3 --> 42%, 4 --> 27%, 5 --> 16%, 6 --> 9.5%, 7 --> 5.5%, 8 --> 3.1%
 * 
 * But one great advantage of Hamming encoder is, that for insertion of k bits we need modify at most 1 bit of 2^k-1 bits.
 * So for values of k in <3;8> we change at most (in % of usable capacity of the cover media):
 *    3 --> 14%, 4 --> 6.7%, 5 --> 3.2%, 6 --> 1.6%, 7 --> 0.8%, 8 --> 0.4%
 * We see, that increasing parameter k we increase the security, in terms of impossibility of the statistical stegoanalysis attacks.
 * 
 * @param[in] parityBits Must be in range <getParityBitsMin(); getParityBitsMax()>
 * @return Instance of the Hamming encoder initialized with parameter 'parityBits'
 */
HammingEncoder::HammingEncoder(uint32 parityBits)
{
    try { init(parityBits); }
    catch (const std::out_of_range &) { throw; }
}

/**
 * @brief Set parameter param of the encoder with value val
 *
 * This method set value of an encoder's parameter.
 * Both of the arguments must be of the string type.
 * 
 * List of supported parameters for Hamming encoder (purpose of each parameter see constructor of the class):
 *     "parityBits", value (as string) from range <getParityBitsMin(); getParityBitsMax()>
 *
 * @param[in] param Name of the parameter to be set
 * @param[in] val Value of the parameter to be set; must be of the string type
 * @return For all inputs throws exception std::invalid_argument if length of 'param' or 'val' is zero
 * @return For all parameters throws exception std::out_of_range if 'param' is not valid parameter for this class
 * @return For parameter "parityBits" throws exceptions:
 *     - std::invalid_argument, if 'val' is not a number
 *     - std::out_of_range, if 'val' is not from range <getParityBitsMin(); getParityBitsMax()>
 */
void HammingEncoder::setArgByName(const string &param, const string &val) {
    bool isValidParam = false;
    string p = param;

    if (!param.size())
        throw std::invalid_argument("HammingEncoder::setArgByName: 'param' is empty");
    if (!val.size())
        throw std::invalid_argument("HammingEncoder::setArgByName: 'val' is empty");

    std::transform(p.begin(), p.end(), p.begin(), ::tolower);

    // parameter "parityBits"
    if (p.compare("paritybits") == 0) {
        int parityBits;

        isValidParam = true;
        try { parityBits = stoi(val); }
        catch (const std::invalid_argument& ) { throw; }
        catch (const std::out_of_range& ) { throw; }

        if (parityBits<0)
            throw std::out_of_range("HammingEncoder::setArgByName: 'parityBits' should be positive, is " + std::to_string((uint64)parityBits));

        try { init((uint32)parityBits); }
        catch (const std::out_of_range &) { throw; }
    }

    // other parameters..

    // if is not valid parameter
    if (isValidParam == false)
        throw std::invalid_argument("HammingEncoder::setArgByName: unknown class parameter '"+param+"'");
}

/**
 * @brief Get the code name of this encoder
 *
 * Get code name of Hamming encoder. The code name is "Hamming".
 * Static function, does not required any instance of the class.
 * 
 * @return Code name of Hamming encoder class
 */
const string HammingEncoder::getName()
{   
    return ENCODER_HAMMING_CODE_NAME;
}

/**
 * @brief Get the code name of this encoder
 *
 * Get code name of Hamming encoder. The code name is "Hamming".
 * member method, calls static function of the class.
 * 
 * @return Code name of Hamming encoder class
 */
const string HammingEncoder::getNameInstance() const
{
    return HammingEncoder::getName();
}

/**
 * @brief Transform user data with Hamming encoder
 *
 * Encode user data (steganographic information) stored in buffer 'data' with Hamming encoder.
 * Warning: This function doesn't be called directly!
 * The size of input/output buffers is given intrinsic based on the configuration of the encoder instance.
 * Function is called from CarrierFile->embedBufferUsingEncoder when translating user stego data into carrier file.
 * 
 * @param[in] data Initialized buffer with user data
 * @param[out] codeword Buffer with encoded user data
 * @return Exception std::invalid_argument, if 'codeword' or 'data' are NULL pointer
 */
int HammingEncoder::embed(uint8 *codeword, const uint8 *data)
{
    uint64 h;
    int bitOffset = 0;

    if ( !codeword )
        throw std::invalid_argument("HammingEncoder::embed: 'codeword' is NULL");
    if ( !data )
        throw std::invalid_argument("HammingEncoder::embed: 'data' is NULL");

    for (uint32 i=0;i<_codewordsInBlock;i++) {
        h = computeH(&codeword[i*_codewordBufferSize]);
        h ^= readBitsFromBuffer(data, bitOffset, _parityBits); // h = h xor m

        if (h != 0) {
            swapBitInCodeword(&codeword[i*_codewordBufferSize], (int)h);
        }

        bitOffset += _parityBits;
    }

    return 0;
}

/**
 * @brief Transform data with Hamming decoder
 *
 * Decode data previously obtained from carrier file (stored in 'codework') into user data (steganographic information),
 * stored in buffer 'data', with Hamming decoder.
 * Warning: This function doesn't be called directly!
 * The size of input/output buffers is given intrinsic based on the configuration of the encoder instance.
 * Function is called from CarrierFile->extractBufferUsingEncoder when translating data from carrier file into user data.
 * 
 * @param[in] codeword Initialized buffer with encoded user data
 * @param[out] data Buffer with decoded user data
 * @return Exception std::invalid_argument, if 'codeword' or 'data' are NULL pointer
 */
int HammingEncoder::extract(const uint8 *codeword, uint8 *data)
{
    if ( !codeword )
        throw std::invalid_argument("HammingEncoder::extract: 'codeword' is NULL");
    if ( !data )
        throw std::invalid_argument("HammingEncoder::extract: 'data' is NULL");

    memset(data, 0, _dataBlockSize);

    uint64 h;
    int bitOffset = 0;

    for (uint32 i=0;i<_codewordsInBlock;i++) {
        h = computeH(&codeword[i*_codewordBufferSize]);
        writeBitsToBuffer(h, bitOffset, data);
        bitOffset += _parityBits;
    }

    return STEGO_NO_ERROR;
}

/**
 * @brief Writes value (bit representation) to offset in buffer
 * 
 * Writes bit representation of 'value' into 'buffer' starting at 'offset' position.
 * This helper function is invoked internally from HammingEncoder::extract method.
 * Method doesn't make any variables validity test.
 *
 * @param[in] value Value to be written into buffer
 * @param[in] offset Starting position from beginning of the buffer in bits
 * @param[out] buffer Initialized memory storage
 *
 * @return Nothing
 */
void HammingEncoder::writeBitsToBuffer(uint64 value, int offset, uint8 *buffer)
{
    int shrValue;
    uint8 tmpValue;
    int writtenBits = 0;
    while (writtenBits < (int) _parityBits) {
        int byteOffset = offset / 8;
        int bitOffset = offset % 8;
        shrValue = _parityBits - 8 + bitOffset - writtenBits;
        if (shrValue < 0) {
            tmpValue = (value << -shrValue) & ((0xFF << -shrValue) & 0xFF);
        } else {
            tmpValue = (value >> shrValue) & 0xFF;
        }
        buffer[byteOffset] ^= tmpValue;
        
        if (shrValue <= 0) {
            writtenBits += (_parityBits - writtenBits);
        } else {
            writtenBits += ((_parityBits - writtenBits) - shrValue);
        }
        
        if (shrValue <= 0) {
            offset += _parityBits;
        } else {
            offset += (_parityBits - shrValue);
        }
    }
}

/**
 * @brief Reads value (bit representation) at offset from buffer
 * 
 * Reads bit representation of 'result' from 'buffer' starting at 'offset' position.
 * This helper function is invoked internally from HammingEncoder::embed method.
 * Method doesn't make any variables validity test.
 *
 * @param[in] buffer Initialized memory storage with valid data
 * @param[in] offset Starting position for reading from beginning of the buffer in bits
 * @param[in] length Number of bits to be read from buffer
 *
 * @return Readed bits from buffer represented as number
 */
uint64 HammingEncoder::readBitsFromBuffer(const uint8 *buffer, int offset, int length)
{
    uint8 tmpValue;
    uint64 tmpLongValue;
    uint64 result = 0;
    int writtenBits = 0;
    int tmpWrittenBits;
    while (writtenBits < length) {
        int byteOffset = offset / 8;
        int bitOffset = offset % 8;
        tmpValue = (buffer[byteOffset] << bitOffset) & 0xFF;
        tmpWrittenBits = 8 - bitOffset;
        if (length-writtenBits < tmpWrittenBits) {
            tmpValue >>= tmpWrittenBits - (length-writtenBits);
            tmpWrittenBits -= (tmpWrittenBits-(length-writtenBits));
        }
        tmpValue = tmpValue >> bitOffset;
        tmpLongValue = tmpValue;
        result ^= tmpLongValue << (length-writtenBits-tmpWrittenBits);
        offset += tmpWrittenBits;
        writtenBits += tmpWrittenBits;
    }
    return result;
}

/**
 * @brief Multiply input vector by control matrix H
 *
 * Multiply input vector stored in 'buffer' by control matrix of Hamming code H.
 * This function is used from HammingEncoder::extract and HammingEncoder::embed.
 * There is no control of validity of input argument.
 *
 * @param[in] buffer Input vector to be multipled
 * 
 * @return Multiplication result
 */
uint64 HammingEncoder::computeH(const uint8 *buffer)
{
    uint64 tmpHline = 1;
    uint64 result = 0;
    for (uint32 i=0;i<_codewordBufferSize;i++)
    {
        for (int j=7;j>=0;j--) {
            if (tmpHline <= (uint64)_totalBits) {
                if ((buffer[i] >> j) & 0x01) result ^= tmpHline;
                tmpHline++;
            }
        }
    }
    return result;
}

/**
 * @brief Swap value of the bit in codeword
 *
 * Swap value of the 'bitIn'-th bit in codeword buffer.
 * Function is used internally from HammingEncoder::embed.
 * There is no check of validity of the input args.
 *
 * @param[in] bitIn Position (in bits) of the bit to be changed in input 'buffer'
 * @param buffer Initialized buffer with valid data (codewords)
 *
 * @return Nothing
 */
void HammingEncoder::swapBitInCodeword(uint8 *buffer, int bitIn)
{
    if (!bitIn) 
        return;
    buffer[(bitIn-1) / 8] ^= 1 << (7 - ((bitIn-1) % 8));
}

}