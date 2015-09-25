#ifndef STEGODISK_FUSE_FUSESERVICEDELEGATE_H_
#define STEGODISK_FUSE_FUSESERVICEDELEGATE_H_

#include <string>

class FuseServiceDelegate {
public:
    virtual void FuseMounted(bool success) = 0;
    virtual void FuseMountProgress(int stage, int stage_count, std::string stage_title) = 0;
    virtual void FuseUnmounted() = 0;
};

#endif // STEGODISK_FUSE_FUSESERVICEDELEGATE_H_
