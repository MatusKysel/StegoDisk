/**
* @file file.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Interface for file manipulation
*
*/

#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <regex>

#include "api_mask.h"
#include "stego_types.h"
#include "include_fs_library.h"

namespace stego_disk {

class File;

class FilePtr {

  friend class File;

public:
  ~FilePtr();
  FILE* Get() { return file_handle_; }

private:
  FilePtr(const File& file);
  FILE* file_handle_;
};

//PSTODO je celkom bordel s timi pathami, cize idealne sa toho zbavit najviac ako sa da, len je otazka ci nie skor ked bude podporovany iba std::filesystem, kedze podla mna by sa to dalo nahradit za fs::path
class File {
public:
#ifndef HAS_FILESYSTEM_LIBRARY
  File(std::string base_path, std::string relative_path);
#else
  File(const fs::path &path);
#endif
  std::string GetAbsolutePath() const;
  std::string GetRelativePath() const;
  std::string GetBasePath() const;
  std::string GetNormalizedPath() const;

  std::string GetExtension(bool conver_to_lowercase = true) const;
  std::string GetFileName() const;

  uint64 GetSize() const;

  static std::string NormalizePath(std::string platform_specific_path);

  static std::vector<File> GetFilesInDir(const std::string &directory, const std::string &filter = "");

  FilePtr Open();

private:
  std::string base_path_;
  std::string relative_path_;
#ifdef HAS_FILESYSTEM_LIBRARY
  fs::path path_;
#endif
};

} // stego_disk