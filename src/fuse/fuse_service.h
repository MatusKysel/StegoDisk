/**
* @file fuse_service.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File FUSE support for this library
*
*/

#ifndef STEGODISK_FUSE_FUSESERVICE_H_
#define STEGODISK_FUSE_FUSESERVICE_H_

#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#ifdef __APPLE__
#include <osxfuse/fuse.h>
#else
#include <fuse.h>
#endif

#include <string>

#include "api_mask.h"
#include "logging/logger.h"
#include "stego-disk_export.h"
#include "stego_storage.h"

namespace stego_disk {

struct FuseContext {
  StegoStorage* stego_storage;
  uint64 capacity;
  uid_t uid;
  gid_t gid;
  mode_t mode;
  struct timespec tv[2];
  bool writes;
};

class STEGO_DISK_EXPORT FuseService {

public:
  static const char* virtual_file_name_;

  int Init(StegoStorage *stego_storage);
  std::string MountFuse();
  int MountFuse(const std::string &mount_point);
  static int UnmountFuse(const std::string &mount_point);

private:
  FuseContext* ctx_;
};
}

#endif // STEGODISK_FUSE_FUSESERVICE_H_
