/**
* @file file_manager.h
* @author Matus Kysel
* @date 2016
* @brief File manager for unit tests
*
*/

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>


class FileManager {
public:
  FileManager() {}


  ~FileManager() {}

  inline static void CopyFile(const std::string &input,
                              const std::string &output) {
    std::ifstream src(input, std::ios::binary);
    std::ofstream dst(output, std::ios::binary);
    if (src.is_open() && dst.is_open())
      dst << src.rdbuf();
  }

  // These used to shell out to "cp -rf" and "rm -rf" with the paths pasted
  // into the command string, which CodeQL reports as command injection: a
  // directory name containing shell metacharacters would be executed. The
  // file's own TODO asked for this. std::filesystem needs no shell and the
  // project already requires C++17.
  inline static void CopyDirectory(const std::string &src,
                                   const std::string &dst) {
    std::cout << "copy '" << src << "' to '" << dst << "'" << std::endl;
    if (std::filesystem::exists(src))
      std::filesystem::copy(src, dst,
                            std::filesystem::copy_options::overwrite_existing |
                                std::filesystem::copy_options::recursive);
  }

  inline static void RemoveDirectory(const std::string &path) {
    std::cout << "Remove '" << path << "'" << std::endl;
    if (std::filesystem::exists(path))
      std::filesystem::remove_all(path);
  }
};

#endif // FILE_MANAGER_H
