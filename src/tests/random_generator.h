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
#include <random>

#include <string>
#include <fstream>
#include <iostream>

class RandomGenerator {
 public:
  RandomGenerator() : random_file_("/dev/urandom", std::ios_base::binary | std::ios_base::in), generator_(device_()), distribution_(0, 256) {
    if (random_file_.is_open()) std::cout << "Will seed random binary strings from /dev/urandom" << std::endl;
    else std::cout << "Failed to open /dev/urandom, will use rand() to generate random binary strings" << std::endl;
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
    return static_cast<char>(Instance().distribution_(Instance().generator_));
  }

  inline static bool RandomOpen() { return Instance().random_file_.is_open(); }
 private:
  std::fstream random_file_;
  std::random_device device_;
  std::mt19937 generator_;
  std::uniform_int_distribution<int> distribution_;
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
