/**
* @file stego_test.cc
* @author Matus Kysel
* @date 2016
* @brief Helper for execution of unit tests for this library
*
*/

#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>

#include "stego_storage.h"
#include "logging/logger.h"

#include "test_assert_helper.h"
#include "random_generator.h"
#include "file_manager.h"

#include "tests/test_config.h"


bool TransferEngineInit() {

  std::string logging_level("INFO");

  char *env_logging_level = NULL;
  if ((env_logging_level = getenv("LOGGING_LEVEL"))) {
    logging_level.assign(env_logging_level);
  }

  Logger::SetVerbosityLevel(logging_level, std::string("cout"));

  return true;
}

static void PrintHelp(char *name) {
  std::cerr << "Usage: " << name << " <option(s)> \n"
            << "Options:\n"
            << "\t-h,--help\t\tShow this help message\n"
            << "\t-e,--encoder ENCODER\tSpecify the encoder\n"
            << "\t-p,--permutation PERMUTATION\tSpecify the permutation\n"
            << "\t-g,--gen_file_size GEN_SIZE\tSpecify size of generated data\n"
            << "\t-d,--directory DIRECTORY\tSpecify the source directory\n"
            << "\t-t,--test_directory \tSpecify that this directory is only for"
               " testing and it will create copy of it\n"
            << "\t-p,--password \tSpecify if the password sould be used\n"
            << std::endl;
}

stego_disk::EncoderFactory::EncoderType StrToEncoder(std::string &encoder){
  std::transform(encoder.begin(), encoder.end(), encoder.begin(), ::tolower);

  if (encoder == "lsb") {
    return stego_disk::EncoderFactory::EncoderType::LSB;
  } else if (encoder == "hamming"){
    return stego_disk::EncoderFactory::EncoderType::HAMMING;
  } else {
    return stego_disk::EncoderFactory::GetDefaultEncoderType();
  }
}

stego_disk::PermutationFactory::PermutationType StrToPermutation(std::string &permutation){
  std::transform(permutation.begin(), permutation.end(), permutation.begin(), ::tolower);

  if (permutation == "identity") {
    return stego_disk::PermutationFactory::PermutationType::IDENTITY;
  } else if (permutation == "affine") {
    return stego_disk::PermutationFactory::PermutationType::AFFINE;
  } else if (permutation == "affine64") {
    return stego_disk::PermutationFactory::PermutationType::AFFINE64;
  } else if (permutation == "num_feistel") {
    return stego_disk::PermutationFactory::PermutationType::FEISTEL_NUM;
  } else if (permutation == "mix_feistel"){
    return stego_disk::PermutationFactory::PermutationType::FEISTEL_MIX;
  } else {
    return stego_disk::PermutationFactory::GetDefaultPermutationType();
  }
}

int main(int argc, char *argv[]) {
  bool error = false;

  //! Disables output truncating for ctest xml
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;

  if (!TransferEngineInit()) return -1;

  std::string encoder;
  std::string permutation;
  std::string file_type;
  std::string dir;
  bool test_directory = false;
  bool password = false;
  size_t gen_file_size = 0;

  if (argc < 3) {
    PrintHelp(argv[0]);
    return -1;
  }
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-h") || (arg == "--help")) {
      PrintHelp(argv[0]);
      return 0;
    } else if ((arg == "-e") || (arg == "--encoder")) {
      if (++i < argc) {
        encoder = argv[i];
      } else {
        LOG_ERROR("--encoder option requires one argument.");
        return -1;
      }
    } else if ((arg == "-p") || (arg == "--permutation")) {
      if (++i < argc) {
        permutation = argv[i];
      } else {
        LOG_ERROR("--permutation option requires one argument.");
        return -1;
      }
    } else if ((arg == "-g") || (arg == "--gen_file_size")) {
      if (++i < argc) {
        gen_file_size = atoi(argv[i]);
      } else {
        LOG_ERROR("--gen_file_size option requires one argument.");
        return -1;
      }
    } else if ((arg == "-d") || (arg == "--directory")) {
      if (++i < argc) {
        dir = argv[i];
      } else {
        LOG_ERROR("--directory option requires one argument.");
        return -1;
      }
    } else if ((arg == "-t") || (arg == "--test_directory")) {
      test_directory = true;
    } else if ((arg == "-f") || (arg == "--file_type")) {
      if (++i < argc) {
        file_type = argv[i];
      } else {
        LOG_ERROR("--file_type option requires one argument.");
        return -1;
      }
    } else if ((arg == "-p") || (arg == "--password")) {
      if (++i < argc) {
        password = (atoi(argv[i]) == 1) ? true : false;
      } else {
        LOG_ERROR("--password option requires one argument.");
        return -1;
      }
    } else {
      LOG_ERROR("Unknown argument: " << argv[i]);
    }
  }

  size_t size;
  std::unique_ptr<stego_disk::StegoStorage>
      stego_storage(new stego_disk::StegoStorage());

  if(!dir.empty() && test_directory) {
    std::cout << dir << std::endl;
    dir = DST_DIRECTORY + dir;
    std::cout << dir << std::endl;
    FileManager::RemoveDirecotry(dir);
    FileManager::CopyDirecotry(SRC_DIRECTORY, dir);
  }
  if (dir.empty()) {
    LOG_ERROR("directory was not set");
    return false;
  }
  stego_storage->Configure(StrToEncoder(encoder), StrToPermutation(permutation),
                           StrToPermutation(permutation));
  LOG_DEBUG("Opening storage");
#ifdef _WIN32
  stego_storage->Open(FileManager::GetWinPath(dir), (password) ? PASSWORD : "");
#else
  stego_storage->Open(dir, (password) ? PASSWORD : "");
#endif
  LOG_DEBUG("Loading storage");
  stego_storage->Load();
  size = stego_storage->GetSize();
  std::cout << "Storage size = " << size << "B" << std::endl;
  if( gen_file_size == 0) gen_file_size = size;
  std::string input;
  std::string output;
  LOG_DEBUG("Generating random string");
  GenerateRandomString(&input, gen_file_size);
  LOG_DEBUG("Writing to the storage");
  stego_storage->Write(&(input[0]), 0, input.size());
  LOG_DEBUG("Saving storage");
  stego_storage->Save();
  LOG_DEBUG("Opening storage");
  stego_storage->Configure(StrToEncoder(encoder), StrToPermutation(permutation),
                           StrToPermutation(permutation));
  stego_storage->Open(dir, (password) ? PASSWORD : "");
  LOG_DEBUG("Loading storage");
  stego_storage->Load();
  output.resize(input.size());
  LOG_DEBUG("Reading from the storage");
  stego_storage->Read(&(output[0]), 0, input.size());
  LOG_DEBUG("Saving storage");
  stego_storage->Save();

  if(test_directory) FileManager::RemoveDirecotry(dir);

  if (input != output) {
    LOG_ERROR("Not equal! Input size: " << input.size() <<
              " output size: " << output.size());
    error = true;
  }

  return error;
}

