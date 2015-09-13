//
//  Encoder.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/21/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include "Encoders/Encoder.h"


namespace StegoDisk {
    
/**
 * @brief Constructor
 *
 * Initialize member variables. It is invoked from constructors of derived classes.
 */
Encoder::Encoder()
{
    _codewordBlockSize = 0;
    _dataBlockSize = 0;
}

/**
 * @brief Destructor
 *
 * Nothing to do.
 */
Encoder::~Encoder()
{
    // nothing to do
}

/**
 * @brief Get the codeword block size
 *
 * Get the codeword block size (encoded information, data embedded in carrier file) for instance of some encoder.
 * 
 * @return Codeword block size
 */
uint32 Encoder::getCodewordBlockSize()
{
    return _codewordBlockSize;
}

/**
 * @brief Get the data block size
 *
 * Get the data block size (decoded information, user data) for instance of some encoder.
 * 
 * @return Data block size
 */
uint32 Encoder::getDataBlockSize()
{
    return _dataBlockSize;
}

/**
 * @brief Throws an exception because derived class has no implemented this method
 *
 * Method getName is obligue for all of the derived classes.
 * If one calls this method for class Encoder, there is emitted an exception std::domain_error.
 * Purpose of this method is return code name of some derived class.
 *
 * @return std::domain_error exception
 */
/*
const string Encoder::getName()
{
    throw std::domain_error("Encoder::getName: call for abstract class");
}
*/

/**
 * @brief Throws an exception because derived class has no implemented this method
 *
 * Method getNew is oblique for all of the derived classes.
 * If one calls this method for class Encoder, there is emitted an exception std::domain_error.
 * Purpose of this method is return smart pointer (shared_ptr) to instance of some derived class.
 *
 * @return std::domain_error exception
 */
/*
shared_ptr<Encoder> Encoder::getNew()
{
	throw std::domain_error("Encoder::getNew: call for abstract class");
}
*/

}
