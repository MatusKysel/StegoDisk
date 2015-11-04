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
  }

  ~RandomGenerator() {
    if (random_file_.is_open()) random_file_.close();
  }

  inline static RandomGenerator &Instance() {
    static RandomGenerator random_generator;
    return random_generator;
  }

  inline static void GetRandomString(std::string *string, size_t size) {
    string->resize(size);

    Instance().random_file_.read(&((*string)[0]), size);

    size_t bytes_read = Instance().random_file_.gcount();

    if (bytes_read != size) string->resize(bytes_read);
  }

  inline static bool RandomOpen() { return Instance().random_file_.is_open(); }
 private:
  std::fstream random_file_;
};

#endif // RANDOM_GENERATOR_H
