/**
* @file file.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Interface for file manipulation
*
*/

#ifndef STEGODISK_UTILS_FILE_H_
#define STEGODISK_UTILS_FILE_H_

#include <string>
#include <vector>
#include <stdexcept>

#include "stego_types.h"

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

class File {
public:
  File(std::string base_path, std::string relative_path);
  std::string GetAbsolutePath() const;
  std::string GetRelativePath() const;
  std::string GetBasePath() const;
  std::string GetNormalizedPath() const;

  std::string GetExtension(bool conver_to_lowercase = true) const;
  std::string GetFileName();

  uint64 GetSize();

  static std::string NormalizePath(std::string platform_specific_path);

  static std::vector<File> GetFilesInDir(std::string directory,
                                         std::string mask);

  FilePtr Open();

private:
  std::string base_path_;
  std::string relative_path_;
};

} // stego_disk

#endif // STEGODISK_UTILS_FILE_H_
