#include "file.h"

#if _WIN32
#define PATH_SEPARATOR	'\\'
#include "FileWin.cpp"
#elif __unix__
#define PATH_SEPARATOR	'/'
//#include "file_unix.cc" //TODO(matus) toto je nejako divne previazane a triedu File
#else
#error Unsupported OS
#endif

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <stdexcept>

#include "stego_header.h"

using namespace std;

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

//TODO(Matus) toto je viac na linux asi treba prerobit
//uint64 File::GetSize()
//{
//  struct stat file_stat;
//  if (stat(GetAbsolutePath().c_str(), &file_stat) < 0) {
//    //TODO: throw exception
//    return 0;
//  }
//  return file_stat.st_size;
//}

File::File(std::string base_path, std::string relative_path) {
  string base_path_safe = base_path;

  if (base_path_safe.empty())
    base_path_safe = ".";

  if (base_path_safe[base_path.length() - 1] != PATH_SEPARATOR)
    base_path_safe.push_back(PATH_SEPARATOR);

  base_path_ = base_path_safe;

  string relative_path_safe = relative_path;
  if (relative_path_safe.length() > 0)
    if (relative_path_safe[0] == PATH_SEPARATOR)
      relative_path_safe = relative_path_safe.substr(1);

  relative_path_ = relative_path_safe;
}

FilePtr File::Open() {
  return FilePtr(*this);
}

std::string File::GetExtension(bool convert_to_lowercase) const {
  unsigned long ext_pos = static_cast<unsigned long>(
                            relative_path_.find_last_of("."));

  if (ext_pos == string::npos) return "";
  if ((relative_path_.length() - ext_pos) > 5) return "";

  string ext = relative_path_.substr(ext_pos);
  if (convert_to_lowercase)
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  return ext;
}

std::string File::GetFileName() {
  //TODO: implement this
  throw std::exception();
  return "";
}



// TODO: add automatic modification date handling (open/close methods)
//     struct utimbuf new_times;

//    new_times.actime = _stat.st_atime;
//    new_times.modtime = _stat.st_mtime;
//    if (utime(_fileName.c_str(), &new_times) < 0) {
//        return -1;
//    }

//    return NO_ERROR;


FilePtr::FilePtr(const File& file) {
  int ret = 0;
#ifdef STEGO_OS_WIN
  fileHandle = nullptr;
  ret = fopen_s(&fileHandle, file.getAbsolutePath().c_str(), "r+b");
#else
  if ((file_handle_ = fopen(file.GetAbsolutePath().c_str(), "r+b")) == nullptr)
    ret = errno;
#endif
  if ( ret != 0 ) {
    LOG_ERROR("FilePtr::FilePtr: cannot open file '" <<
              file.GetAbsolutePath() << "': " << strerror(errno));
    throw std::runtime_error("FilePtr::FilePtr: cannot open file '" +
                             file.GetAbsolutePath() + "': " + strerror(errno));
  }
}

FilePtr::~FilePtr() {
  if (file_handle_ != nullptr) {
    fclose(file_handle_);
    //TODO(matus) fflush a overenie??
    file_handle_ = nullptr;
  }
}

} // stego_disk
