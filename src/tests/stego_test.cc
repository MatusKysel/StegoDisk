
#include <iostream>
#include <algorithm>
#include <string>

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

enum EnocoderType {
  LSB,
  HAMMING
};

std::string EncoderType(char *encoder_) {
  std::string encoder;
  encoder.assign(encoder_);
  std::transform(encoder.begin(), encoder.end(), encoder.begin(), ::tolower);
  //  EnocoderType type;
  //  if (encoder == "lsb") type = EnocoderType::LSB;
  //  else if (encoder == "hamming") type = EnocoderType::HAMMING;
  //  else throw std::invalid_argument("EncoderType: "
  //                                   "unknown type of encoder");

  return encoder;
}

static void PrintHelp(char *name) {
  std::cerr << "Usage: " << name << " <option(s)> SOURCES"
            << "Options:\n"
            << "\t-h,--help\t\tShow this help message\n"
            << "\t-d,--destination DESTINATION\tSpecify the destination path"
            << std::endl;
}

int main(int argc, char *argv[]) {
  bool error = false;

  //! Disables output truncating for ctest xml
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;

  if (!TransferEngineInit()) return -1;

  std::string encoder;
  std::string permutation;
  bool password;
  std::string file_type;
  std::string dir;
  bool test_directory = false;
  size_t gen_file_size;

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
        encoder = EncoderType(argv[i++]);
      } else {
        LOG_ERROR("--encoder option requires one argument.");
        return -1;
      }
    } else if ((arg == "-p") || (arg == "--permutation")) {
      if (++i < argc) {
        permutation = argv[i++];
      } else {
        LOG_ERROR("--permutation option requires one argument.");
        return -1;
      }
    } else if ((arg == "-d") || (arg == "--directory")) {
      if (++i < argc) {
        dir = argv[i++];
      } else {
        LOG_ERROR("--directory option requires one argument.");
        return -1;
      }
    } else if ((arg == "-t") || (arg == "--test_directory")) {
        test_directory = true;

    } else if ((arg == "-f") || (arg == "--file_type")) {
      if (++i < argc) {
        file_type = argv[i++];
      } else {
        LOG_ERROR("--file_type option requires one argument.");
        return -1;
      }
    } else if ((arg == "-p") || (arg == "--password")) {
      if (++i < argc) {
        password = static_cast<bool>(argv[i++]);
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

  if(!dir.empty()) {
    std::cout << dir << std::endl;
    dir = DST_DIRECTORY + dir;
    std::cout << dir << std::endl;
    FileManager::RemoveDirecotry(dir);
    FileManager::CopyDirecotry(SRC_DIRECTORY, dir);
  } else {
    LOG_ERROR("directory was not set");
    return false;
  }
  LOG_DEBUG("Opening storage");
  stego_storage->Open(dir, PASSWORD);
  LOG_DEBUG("Loading storage");
  stego_storage->Load();
  size = stego_storage->GetSize();
  std::cout << "size = " << size << "b" << std::endl;
  std::string input;
  std::string output;
  LOG_DEBUG("Generating random string");
  RandomGenerator::GetRandomString(&input, size/sizeof(std::string) - 1);
  LOG_DEBUG("Writing to the storage");
  stego_storage->Write(&input, 0, input.size());
  LOG_DEBUG("Saving storage");
  stego_storage->Save();
  LOG_DEBUG("Opening storage");
  stego_storage->Open(dir, PASSWORD);
  LOG_DEBUG("Loading storage");
  stego_storage->Load();
  output.reserve(input.size());
  LOG_DEBUG("Reading the storage");
  stego_storage->Read(&output, 0, output.size());
  LOG_DEBUG("Saving storage");
  stego_storage->Save();

  FileManager::RemoveDirecotry(dir);

  LOG_ERROR(input.size());
  LOG_ERROR(output.size());

  if (input != output) {
    LOG_DEBUG("Not equal");
    error = true;
  }

  return error;
}

