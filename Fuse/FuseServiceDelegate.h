#ifndef FUSESERVICEDELEGATE_H
#define FUSESERVICEDELEGATE_H

#include <string>

class FuseServiceDelegate
{
public:
    virtual void fuseMounted(bool success) = 0;
    virtual void fuseMountProgress(int stage, int stageCount, std::string stageTitle) = 0;
    virtual void fuseUnmounted() = 0;
};

#endif // FUSESERVICEDELEGATE_H
