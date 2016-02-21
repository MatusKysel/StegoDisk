/**
* @file random_generator.h
* @author Matus Kysel
* @date 2016
* @brief Random generator for unit tests
*
*/

#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <ctime>
#include <cstdlib>

#include <string>
#include <fstream>
#include <iostream>

class RandomGenerator {
 public:
  RandomGenerator() : random_file_("/dev/urandom", std::ios_base::binary | std::ios_base::in) {
    if (random_file_.is_open()) std::cout << "Will seed random binary strings from /dev/urandom" << std::endl;
    else std::cout << "Failed to open /dev/urandom, will use rand() to generate random binary strings" << std::endl;
    std::srand(std::time(0));
  }

  ~RandomGenerator() {
    if (random_file_.is_open()) random_file_.close();
  }

  inline static RandomGenerator &Instance() {
    static RandomGenerator random_generator;
    return random_generator;
  }

  inline static void GetRandomString(std::string *string, const size_t size) {
    string->resize(size);

    Instance().random_file_.read(&((*string)[0]), size);

    size_t bytes_read = Instance().random_file_.gcount();

    if (bytes_read != size) string->resize(bytes_read);
  }

  static char RandomChar() {
    return rand() % 256;
  }

  inline static bool RandomOpen() { return Instance().random_file_.is_open(); }
 private:
  std::fstream random_file_;
};

void GenerateRandomString(std::string *s, size_t size) {
  if (RandomGenerator::RandomOpen()) {
    RandomGenerator::GetRandomString(s, size);
  } else {
    s->resize(size);

    for (size_t i = 0; i < size; ++i) {
      (*s)[i] = RandomGenerator::RandomChar();
    }
  }
}

#endif // RANDOM_GENERATOR_H
