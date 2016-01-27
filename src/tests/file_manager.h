#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdlib.h>

#include <string>
#include <fstream>
#include <iostream>


class FileManager {
public:
  FileManager() {}


  ~FileManager() {}

  inline static void CopyFile(const std::string &input, const std::string &output) {
    std::ifstream  src(input, std::ios::binary);
    std::ofstream  dst(output, std::ios::binary);
    dst << src.rdbuf();
  }

  inline static void CopyDirecotry(const std::string &src, const std::string &dst) {
    //TODO(Matus) rewrite to secure form
#ifdef _WIN32
   std::string cmd = "xcopy " + src + " " + dst + " /s /e /h /i";
#else
    std::string cmd = "cp -rf " + src + " " + dst;
#endif
    system(cmd.c_str());
  }

  inline static void RemoveDirecotry(const std::string &path) {
    //TODO(Matus) rewrite to secure form
#ifdef _WIN32
   std::string cmd = "rd /s /q " + path;
#else
    std::string cmd = "rm -rf " + path;
#endif
    system(cmd.c_str());
  }
};

#endif // FILE_MANAGER_H
