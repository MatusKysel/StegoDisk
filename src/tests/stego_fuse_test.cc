/**
* @file stego_fuse_test.cc
* @author Matus Kysel
* @date 2016
* @brief FUSE unit test executor
*
*/

#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>

#include "stego_storage.h"
#include "logging/logger.h"
#include "fuse/fuse_service.h"

#include "test_assert_helper.h"
#include "random_generator.h"
#include "file_manager.h"

#include "tests/test_config.h"

struct StegoPair{
  std::unique_ptr<stego_disk::StegoStorage> stego_storage;
  std::string fuse_mount;
};


StegoPair mount_stego(const std::string& dir, bool password) {
  StegoPair pair;

  pair.stego_storage.reset(new stego_disk::StegoStorage());
  std::unique_ptr<stego_disk::FuseService> fuse_service(new stego_disk::FuseService());


  pair.stego_storage->Configure();
  LOG_DEBUG("Opening storage");
  pair.stego_storage->Open(dir, (password) ? PASSWORD : "");
  LOG_DEBUG("Loading storage");
  pair.stego_storage->Load();

  if (fuse_service->Init(pair.stego_storage.get()) != 0) {
    throw std::runtime_error("FuseService::Init failed");
  }

  std::string fuse_mount = fuse_service->MountFuse();
  pair.fuse_mount = fuse_mount;

  if (fuse_mount.empty()) {
    throw std::runtime_error("FuseService::MountFuse failed");
  }


  return pair;
}

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

  if(!dir.empty() && test_directory) {
    std::cout << dir << std::endl;
    dir = DST_DIRECTORY + dir;
    std::cout << dir << std::endl;
    FileManager::RemoveDirectory(dir);
    FileManager::CopyDirectory(SRC_DIRECTORY, dir);
  }

  if (dir.empty()) {
    LOG_ERROR("directory was not set");
    return -1;
  }

  StegoPair pair1 = mount_stego(dir, password);

  std::cout << "Storage size = " << pair1.stego_storage->GetSize() << "B" << std::endl;
  if( gen_file_size == 0) gen_file_size = pair1.stego_storage->GetSize();
  std::string input;
  std::string output;
  std::string input_file = pair1.fuse_mount + "/" + 
                           std::string(stego_disk::FuseService::virtual_file_name_);

  LOG_DEBUG("Generating random string");
  GenerateRandomString(&input, gen_file_size);
  LOG_DEBUG("Writing to the storage");
  std::ofstream ofs(input_file.c_str());
  if (!ofs.is_open()) {
    return -1;
  }
  ofs << input;
  ofs.close();
  stego_disk::FuseService::UnmountFuse(pair1.fuse_mount);
  FileManager::RemoveDirectory(pair1.fuse_mount);

  StegoPair pair2 = mount_stego(dir, password);

  std::string input_file2 = pair2.fuse_mount + "/" + 
                           std::string(stego_disk::FuseService::virtual_file_name_);

  std::ifstream ifs(input_file2.c_str());
  if (!ifs.is_open()) {
    return -1;
  }
  output.resize(input.size());
  ifs.read(&output[0], input.size());
  ifs.close();

  stego_disk::FuseService::UnmountFuse(pair2.fuse_mount);
  FileManager::RemoveDirectory(pair2.fuse_mount);

  if(test_directory) FileManager::RemoveDirectory(dir);

  if (input != output) {
    LOG_ERROR("Not equal! Input size: " << input.size() <<
              " output size: " << output.size());
    error = true;
  }

  return error;
}

