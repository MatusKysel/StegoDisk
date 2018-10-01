/**
* @file file.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Interface for file manipulation
*
*/

#include "file.h"

#if _WIN32
#define PATH_SEPARATOR	'\\'
#elif defined(__unix__) || defined(__APPLE__)
#define PATH_SEPARATOR	'/'
#else
#error Unsupported OS
#endif

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <algorithm>
#include <stdexcept>
#include <fstream>

#include "api_mask.h"
#include "stego_header.h"
#include "utils/exceptions.h"

namespace stego_disk {

std::string File::GetAbsolutePath() const {
  return base_path_ + relative_path_;
}

std::string File::GetRelativePath() const {
  return relative_path_;
}

std::string File::GetBasePath() const {
  return base_path_;
}

std::string File::GetNormalizedPath() const {
  return NormalizePath(relative_path_);
}

std::string File::NormalizePath(std::string platform_specific_path) {
  std::transform(platform_specific_path.begin(), platform_specific_path.end(),
                 platform_specific_path.begin(), ::tolower);
  std::replace(platform_specific_path.begin(), platform_specific_path.end(),
               PATH_SEPARATOR, '/');
  return platform_specific_path;
}

uint64 File::GetSize() const
{
  struct stat stat_buf;
  int rc = stat(GetAbsolutePath().c_str(), &stat_buf);
  return rc == 0 ? static_cast<uint64>(stat_buf.st_size) : 0;
}

File::File(std::string base_path, std::string relative_path) {
  std::string base_path_safe = base_path;

  if (base_path_safe.empty())
    base_path_safe = ".";

  if (base_path_safe.back() != PATH_SEPARATOR)
    base_path_safe.push_back(PATH_SEPARATOR);

  base_path_ = base_path_safe;

  std::string relative_path_safe = relative_path;
  if (relative_path_safe.length() > 0)
    if (relative_path_safe.front() == PATH_SEPARATOR)
      relative_path_safe = relative_path_safe.substr(1);

  relative_path_ = relative_path_safe;
}

FilePtr File::Open() {
  return FilePtr(*this);
}

std::string File::GetExtension(bool convert_to_lowercase) const {
  unsigned long ext_pos = static_cast<unsigned long>(
                            relative_path_.find_last_of("."));

  if (ext_pos == std::string::npos) return "";
  if ((relative_path_.length() - ext_pos) > 5) return "";

  std::string ext = relative_path_.substr(ext_pos + 1);
  if (convert_to_lowercase)
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  return ext;
}

std::string File::GetFileName() const
{
  //TODO: implement this
  throw exception::FuctionNotImplementad{};
  return "";
}


FilePtr::FilePtr(const File& file) {
  int ret = 0;
#ifdef STEGO_OS_WIN
  file_handle_ = nullptr;
  ret = fopen_s(&file_handle_, file.GetAbsolutePath().c_str(), "r+b");
#else
  if ((file_handle_ = fopen(file.GetAbsolutePath().c_str(), "r+b")) == nullptr)
    ret = errno;
#endif
  if (ret != 0) {
    LOG_ERROR("FilePtr::FilePtr: cannot open file '" <<
              file.GetAbsolutePath() << "': " << strerror(ret));
    throw exception::ErrorOpenFIle(file.GetAbsolutePath(), strerror(ret));
  }
}

FilePtr::~FilePtr() {
  if (file_handle_ != nullptr) {
    if (fclose(file_handle_) != 0 ) {
      LOG_ERROR("FilePtr::FilePtr: cannot close file " << strerror(errno));
    }
    file_handle_ = nullptr;
  }
}

} // stego_disk
