#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>
#include "StegoTypes.h"

namespace StegoDisk {

class File;

class FilePtr {

    friend class File;

public:
    ~FilePtr();
    FILE* get() { return fileHandle; };

private:
    FilePtr(const File& file);
    FILE* fileHandle;
};

class File
{
public:
    File(std::string basePath, std::string relativePath);
    std::string getAbsolutePath() const;
    std::string getRelativePath() const;
    std::string getBasePath() const;
    std::string getNormalizedPath() const;

    std::string getExtension(bool convertToLowercase = true) const;
    std::string getFileName();

    uint64 getSize();

    static std::string normalizePath(std::string platformSpecificPath);

	static std::vector<File> getFilesInDir(std::string directory, std::string mask);

    FilePtr open();

private:
    std::string basePath;
    std::string relativePath;
};

};

#endif // FILE_H
