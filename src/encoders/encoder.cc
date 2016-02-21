/**
* @file encoder.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Implementation of interface class for encoders
*
*/

#include "encoder.h"


namespace stego_disk {

/**
 * @brief Constructor
 *
 * Initialize member variables. It is invoked from constructors of derived classes.
 */
Encoder::Encoder() {
  codeword_block_size_ = 0;
  data_block_size_ = 0;
}

/**
 * @brief Destructor
 *
 * Nothing to do.
 */
Encoder::~Encoder() {}

/**
 * @brief Get the codeword block size
 *
 * Get the codeword block size (encoded information, data embedded in carrier file) for instance of some encoder.
 *
 * @return Codeword block size
 */
uint32 Encoder::GetCodewordBlockSize() {
  return codeword_block_size_;
}

/**
 * @brief Get the data block size
 *
 * Get the data block size (decoded information, user data) for instance of some encoder.
 *
 * @return Data block size
 */
uint32 Encoder::GetDataBlockSize() {
  return data_block_size_;
}

/**
 * @brief Throws an exception because derived class has no implemented this method
 *
 * Method GetName is obligue for all of the derived classes.
 * If one calls this method for class Encoder, there is emitted an exception std::domain_error.
 * Purpose of this method is return code name of some derived class.
 *
 * @return std::domain_error exception
 */
/*
const string Encoder::GetName()
{
    throw std::domain_error("Encoder::GetName: call for abstract class");
}
*/

/**
 * @brief Throws an exception because derived class has no implemented this method
 *
 * Method GetNew is oblique for all of the derived classes.
 * If one calls this method for class Encoder, there is emitted an exception std::domain_error.
 * Purpose of this method is return smart pointer (shared_ptr) to instance of some derived class.
 *
 * @return std::domain_error exception
 */
/*
shared_ptr<Encoder> Encoder::GetNew()
{
  throw std::domain_error("Encoder::GetNew: call for abstract class");
}
*/

} // stego_disk
