/**
* @file file_manager.h
* @author Matus Kysel
* @date 2016
* @brief File manager for unit tests
*
*/

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdlib.h>

#include <string>
#include <algorithm>
#include <fstream>
#include <iostream>

#include "utils/exceptions.h"
#include "utils/include_fs_library.h"


class FileManager {
public:
  FileManager() {}


  ~FileManager() {}

  inline static void CopyFile(const std::string &input, const std::string &output) {
    std::ifstream  src(input, std::ios::binary);
    std::ofstream  dst(output, std::ios::binary);
    dst << src.rdbuf();
  }

  inline static std::string GetWinPath(const std::string &input) {
	  std::string out = input;
	  std::replace(out.begin(), out.end(), '/', '\\');
	  return out;
  }

#ifndef HAS_FILESYSTEM_LIBRARY
  //PSTODO change to filesystem
  inline static void CopyDirectory(const std::string &src, const std::string &dst) {
    //TODO(Matus) rewrite to secure form
#ifdef _WIN32
    std::string cmd = "xcopy " + GetWinPath(src) + " " + GetWinPath(dst) + " /s /e /h /i";
#else
    std::string cmd = "cp -rf " + src + " " + dst;
#endif
    std::cout << cmd << std::endl;

    if(system(cmd.c_str()) < 0)
      throw stego_disk::exception::ExecFailed{cmd};
  }

  //PSTODO change to filesystem
  inline static void RemoveDirectory(const std::string &path) {
    //TODO(Matus) rewrite to secure form
#ifdef _WIN32
    std::string cmd = "rd /s /q " + GetWinPath(path);
#else
    std::string cmd = "rm -rf " + path;
#endif
    std::cout << cmd << std::endl;

    if(system(cmd.c_str()) < 0)
      throw stego_disk::exception::ExecFailed{cmd};
  }
#else //HAS_FILESYSTEM_LIBRARY
  inline static void CopyDirectory(const std::string &src, const std::string &dst) {
    std::cout << "copy '" << src << "' to '" << dst << "'" << std::endl;
	if (fs::exists(src))
      fs::copy(src, dst, fs::copy_options::overwrite_existing|fs::copy_options::recursive);
  }

  inline static void RemoveDirectory(const std::string &path) {
    std::cout << "Remove '" << path << "'" << std::endl;
	if (fs::exists(path))
      fs::remove_all(path);
  }
#endif //HAS_FILESYSTEM_LIBRARY

};

#endif // FILE_MANAGER_H
