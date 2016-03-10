/**
* @file stego_fuse_test.cc
* @author Matus Kysel
* @date 2016
* @brief FUSE unit test executor
*
*/

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <cstring>

#include "stego_storage.h"
#include "logging/logger.h"
#include "fuse/fuse_service.h"

#include "test_assert_helper.h"
#include "random_generator.h"
#include "file_manager.h"

#include "tests/test_config.h"


bool LoggerInit() {

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
            << "\t-g,--gen_file_size GEN_SIZE\tSpecify size of generated data\n"
            << "\t-d,--directory DIRECTORY\tSpecify the source directory\n"
            << "\t-t,--test_directory \tSpecify that this directory is only for"
               " testing and it will create copy of it\n"
            << "\t-p,--password \tSpecify if the password sould be used\n"
            << std::endl;
}

int main(int argc, char *argv[]) {
  bool error = false;

  //! Disables output truncating for ctest xml
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;

  if (!LoggerInit()) return -1;

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
    FileManager::RemoveDirectory(dir);
    FileManager::CopyDirectory(SRC_DIRECTORY, dir);
  }
  if (dir.empty()) {
    LOG_ERROR("directory was not set");
    return false;
  }
  stego_storage->Configure();
  LOG_DEBUG("Opening storage");
  stego_storage->Open(dir, (password) ? PASSWORD : "");
  LOG_DEBUG("Loading storage");
  stego_storage->Load();
  size = stego_storage->GetSize();

  if (FuseService::Init(stego_storage.get()) != 0) {
    return false;
  }

  if (FuseService::MountFuse(DST_DIRECTORY) != 0) {
    return false;
  }

  std::cout << "Storage size = " << size << "B" << std::endl;
  if( gen_file_size == 0) gen_file_size = size;
  std::string input;
  std::string output;
  std::string input_file = std::string(DST_DIRECTORY) +
                           std::string(FuseService::virtual_file_name_,
                                       std::strlen(FuseService::virtual_file_name_));
//  LOG_DEBUG("Generating random string");
//  GenerateRandomString(&input, gen_file_size);
//  LOG_DEBUG("Writing to the storage");
//  std::ofstream ofs(input_file.c_str());
//  if (!ofs.is_open()) {
//    return true;
//  }
//  ofs << input;
//  ofs.close();
//  LOG_DEBUG("Saving storage");
  stego_storage->Save();

  LOG_DEBUG("Loading storage");
  stego_storage->Load();
  output.resize(input.size());
  LOG_DEBUG("Reading from the storage");
  stego_storage->Read(&(output[0]), 0, input.size());
  stego_storage->Save();
  FuseService::UnmountFuse(DST_DIRECTORY);

  if(test_directory) FileManager::RemoveDirectory(dir);

  FileManager::RemoveDirectory(input_file);

  if (input != output) {
    LOG_ERROR("Not equal! Input size: " << input.size() <<
              " output size: " << output.size());
    error = true;
  }

  return error;
}

