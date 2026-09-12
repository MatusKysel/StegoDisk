/**
* @file fuse_service.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief FUSE service implementation
*
*/

#include "fuse_service.h"

#include <cerrno>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/wait.h>

#include <ctime>
#include <string>
#include <thread>

#include "api_mask.h"
#include "encoders/encoder.h"
#include "encoders/hamming_encoder.h"
#include "file_management/carrier_files_manager.h"
#include "permutations/affine_permutation.h"
#include "permutations/feistel_num_permutation.h"
#include "permutations/permutation.h"
#include "utils/config.h"
#include "utils/stego_math.h"

namespace stego_disk {

#ifdef __APPLE__
static const char *file_path = "/virtualdisc.dmg";
const char *FuseService::virtual_file_name_ = "virtualdisc.dmg";
#else
static const char *file_path = "/virtualdisc.iso";
const char *FuseService::virtual_file_name_ = "virtualdisc.iso";
#endif

static struct fuse_operations stegofs_ops;

static FuseContext *get_ctx() {
  return static_cast<FuseContext *>(fuse_get_context()->private_data);
}

// =============================================================================
//      FUSE CALLBACK METHODS
// =============================================================================

static int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t, struct fuse_file_info *) {
  if (strcmp(path, "/") != 0) /* We only recognize the root directory. */
    return -ENOENT;

  filler(buf, ".", NULL, 0);           /* Current directory (.)  */
  filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
  filler(buf, file_path + 1, NULL, 0); /* The only file we have. */

  return 0;
}

static int sfs_getattr(const char *path, struct stat *stbuf) {
  FuseContext *ctx = get_ctx();
  memset(stbuf, 0, sizeof(struct stat));

  if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 3;
  } else if (strcmp(path, file_path) == 0) { /* The only file we have. */
    stbuf->st_nlink = 1;
    stbuf->st_size = ctx->capacity;
    stbuf->st_atim = ctx->tv[0];
    stbuf->st_mtim = ctx->tv[1];
    stbuf->st_uid = ctx->uid;
    stbuf->st_gid = ctx->gid;
    stbuf->st_mode = ctx->mode;
  } else /* We reject everything else. */
    return -ENOENT;

  return 0;
}

static int sfs_access(const char *, int) {
  return 0;
}

static int sfs_readlink(const char *, char *, size_t) {
  return -ENOSYS;
}

static int sfs_mkdir(const char *, mode_t) {
  return -EROFS;
}

static int sfs_unlink(const char *) {
  return 0;
}

static int sfs_rmdir(const char *) {
  return -EROFS;
}

static int sfs_rename(const char *, const char *) {
  return -EROFS;
}

static int sfs_chmod(const char *, mode_t mode) {
  get_ctx()->mode = S_IFREG | (mode & 07777);
  return 0;
}

static int sfs_chown(const char *, uid_t uid, gid_t gid) {
  FuseContext *ctx = get_ctx();
  ctx->uid = uid;
  ctx->gid = gid;
  return 0;
}

static int sfs_open(const char *, struct fuse_file_info *) {
  return 0;
}

static int sfs_create(const char *path, mode_t, struct fuse_file_info *fi) {
  return sfs_open(path, fi);
}

static int sfs_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *) {
  if (strcmp(path, file_path) != 0)
    return -ENOENT;

  FuseContext *ctx = get_ctx();
  uint64 offset64 = offset;
  uint64 size64 = size;

  if (offset64 >= ctx->capacity) /* Trying to read past the end of file. */ {
    LOG_ERROR("fuse service: offset+size > capacity_... offset:"
              << offset << ", size:" << size << ", cap:" << ctx->capacity);
    return 0;
  }

  if (offset64 + size64 > ctx->capacity) /* Trim the read to the file size. */ {
    LOG_ERROR("fuse service: offset+size > capacity_... offset:"
              << offset << ", size:" << size << ", cap:" << ctx->capacity);
    size64 = ctx->capacity - offset64;
  }

  try {
    ctx->stego_storage->Read(buf, offset64, size64);
  } catch (...) {
    return -EIO;
  }
  return static_cast<int>(size64);
}

static int sfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *) {
  if (strcmp(path, file_path) != 0)
    return -ENOENT;

  FuseContext *ctx = get_ctx();
  uint64 offset64 = offset;
  uint64 size64 = size;

  if (offset64 >= ctx->capacity) /* Trying to read past the end of file. */ {
    LOG_ERROR("fuse service: offset > capacity_... offset:"
              << offset << ", size:" << size << ", cap:" << ctx->capacity);
    return 0;
  }

  if (offset64 + size64 > ctx->capacity) /* Trim the read to the file size. */ {
    LOG_ERROR("fuse service: offset+size > capacity_... offset:"
              << offset << ", size:" << size << ", cap:" << ctx->capacity);
    size64 = ctx->capacity - offset64;
  }

  try {
    ctx->stego_storage->Write(buf, offset64, size64);
  } catch (...) {
    return -EIO;
  }

  ctx->writes = true;
  return static_cast<int>(size64);
}

static void *sfs_init(struct fuse_conn_info *) {
  return fuse_get_context()->private_data;
}

static void sfs_destroy(void *) {
  delete get_ctx();
}

static int sfs_flush(const char *, struct fuse_file_info *) {
  FuseContext *ctx = get_ctx();
  if (ctx->writes) {
    try {
      ctx->stego_storage->Save();
    } catch (...) {
      return -EIO;
    }
    ctx->writes = false;
  }
  return 0;
}

static int sfs_setxattr(const char *, const char *, const char *, size_t, int) {
  return 0;
}

static int sfs_truncate(const char *, off_t) {
  return 0;
}

static int sfs_utimens(const char *, const struct timespec tv[2]) {
  FuseContext *ctx = get_ctx();
  ctx->tv[0] = tv[0];
  ctx->tv[1] = tv[1];
  return 0;
}


int FuseService::Init(StegoStorage *stego_storage) {
  ctx_ = new FuseContext();
  ctx_->stego_storage = stego_storage;
  ctx_->capacity = stego_storage->GetSize();
  ctx_->uid = getuid();
  ctx_->gid = getgid();
  ctx_->mode = S_IFREG | 0600;
  ctx_->tv[0] = {};
  ctx_->tv[1] = {};
  ctx_->writes = false;

  stegofs_ops.init = sfs_init;
  stegofs_ops.getattr = sfs_getattr;
  stegofs_ops.access = sfs_access;
  stegofs_ops.readlink = sfs_readlink;
  stegofs_ops.readdir = sfs_readdir;
  stegofs_ops.mkdir = sfs_mkdir;
  stegofs_ops.unlink = sfs_unlink;
  stegofs_ops.rmdir = sfs_rmdir;
  stegofs_ops.rename = sfs_rename;
  stegofs_ops.chmod = sfs_chmod;
  stegofs_ops.chown = sfs_chown;
  stegofs_ops.truncate = sfs_truncate;
  stegofs_ops.utimens = sfs_utimens;
  stegofs_ops.create = sfs_create;
  stegofs_ops.open = sfs_open;
  stegofs_ops.read = sfs_read;
  stegofs_ops.write = sfs_write;
  stegofs_ops.destroy = sfs_destroy;
  stegofs_ops.flush = sfs_flush;
  stegofs_ops.setxattr = sfs_setxattr;

  return 0;
}

std::string FuseService::MountFuse() {
  if (!ctx_)
    return "";

  char temp[] = "/tmp/fuseXXXXXX";
  char *p = mkdtemp(temp);

  if (!p)
    return "";

  std::string path{p};

  if (MountFuse(path) == 0)
    return path;

  rmdir(path.c_str());
  return "";
}

int FuseService::MountFuse(const std::string &mount_point) {
  if (!ctx_)
    return -1;

  LOG_INFO("mount point: " << mount_point);

  FuseContext *ctx = ctx_;
  ctx_ = nullptr;
  std::thread([mount_point, ctx]() {
    char mnt_pt[PATH_MAX];
    strncpy(mnt_pt, mount_point.c_str(), sizeof(mnt_pt) - 1);
    mnt_pt[sizeof(mnt_pt) - 1] = '\0';

    char *argv[10];
    memset(argv, 0, 10 * sizeof(char *));
    argv[0] = (char *)"fuse";
    argv[1] = mnt_pt;
    argv[2] = (char *)"-f";
    argv[3] = (char *)"-s";
    argv[4] = (char *)"-o";
    argv[5] = (char *)"allow_other";
    argv[6] = NULL;

    fuse_main(6, argv, &stegofs_ops, ctx);
  }).detach();

  // Poll for FUSE filesystem readiness
  std::string virtual_file = mount_point + "/" + virtual_file_name_;
  struct stat st;
  for (int i = 0; i < 100; i++) {
    usleep(50000);
    if (stat(virtual_file.c_str(), &st) == 0)
      return 0;
  }

  pid_t pid = fork();

  if (pid == 0) {
    execl("/bin/umount", "umount", mount_point.c_str(), nullptr);
    _exit(127);
  }

  if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
  }

  return -1;
}

int FuseService::UnmountFuse(const std::string &mount_point) {
  LOG_INFO("unmounting: " << mount_point);
  std::string virtual_file = mount_point + "/" + virtual_file_name_;
  struct stat st;

  for (int i = 0; i < 300; ++i) {
    pid_t pid = fork();

    if (pid == 0) {
      execl("/bin/umount", "umount", mount_point.c_str(), nullptr);
      _exit(127);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
      break;

    usleep(1000000);
  }

  for (int i = 0; i < 300; ++i) {
    if (stat(virtual_file.c_str(), &st) != 0)
      return 0;

    usleep(1000000);
  }

  return -1;
}

}
