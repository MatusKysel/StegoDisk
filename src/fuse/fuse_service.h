#ifndef STEGODISK_FUSE_FUSESERVICE_H_
#define STEGODISK_FUSE_FUSESERVICE_H_

#ifndef Q_OS_WIN

#include <QString>
#include <QProcess>

#ifdef Q_OS_MAC
#include <osxfuse/fuse.h>
#else
#include <fuse.h>
#endif


#include "fuse_service_delegate_.h"
#include "logging/log.h"
#include "virtual_storage/virtual_storage.h"

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 26

static struct fuse_operations stegofs_ops;

class FuseService {

public:
  FuseService();

  static FuseServiceDelegate*     delegate_;
  static VirtualStorage*        	virtual_storage_;
  static uint64           		capacity_;
  static bool                     fuse_mounted_;
  static const char*              virtual_file_name_;

  static QString                  mount_point_;

  static QProcess*                child_process_;

  static pid_t                    fuse_proc_pid_;
  //static struct fuse_operations stegofs_ops;

  static int Init(VirtualStorage *virtual_storage);
  //static int mount(const char* mount_point);
  static int MountFuse(QString mount_point);
  static int UnmountFuse(QString mount_poin_);

}

#endif

#endif // STEGODISK_FUSE_FUSESERVICE_H_

