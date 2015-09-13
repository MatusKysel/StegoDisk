#include "StegoHeader.h"
#include "File.h"


#if defined(STEGO_OS_WIN)
	#define PATH_SEPARATOR	'\\'
//    #include "FileWin.cpp"
#elif defined(STEGO_OS_UNIX)
	#define PATH_SEPARATOR	'/'
    #include "FileUnix.cpp"
#else
    #error Unsupported OS
#endif

//TODO: move to FileUnix
#include <sys/stat.h>
#include <stdio.h>
#include <algorithm>

using namespace std;

namespace StegoDisk {

std::string File::getAbsolutePath() const
{
    return basePath + relativePath;
}

std::string File::getRelativePath() const
{
    return relativePath;
}

std::string File::getBasePath() const
{
    return basePath;
}

std::string File::getNormalizedPath() const
{
//        string normalizedPath = path;
//        std::transform(path.begin(), path.end(), path.begin(), ::tolower);
//         "/" and "\" are normalized by Qt

    return normalizePath(relativePath);
}

std::string File::normalizePath(std::string platformSpecificPath)
{
	std::transform(platformSpecificPath.begin(), platformSpecificPath.end(), platformSpecificPath.begin(), ::tolower);
	std::replace(platformSpecificPath.begin(), platformSpecificPath.end(), PATH_SEPARATOR, '/');
    return platformSpecificPath;
}

uint64 File::getSize()
{
    struct stat fileStat;
    if (stat(getAbsolutePath().c_str(), &fileStat) < 0) {
        //TODO: throw exception
        return 0;
    }

    return fileStat.st_size;
}

File::File(std::string basePath, std::string relativePath)
{
    string basePathSafe = basePath;

    if (basePathSafe.empty())
        basePathSafe = ".";

    if (basePathSafe[basePath.length()-1] != PATH_SEPARATOR)
        basePathSafe.push_back(PATH_SEPARATOR);

    this->basePath = basePathSafe;

    string relativePathSafe = relativePath;
    if (relativePathSafe.length() > 0)
        if (relativePathSafe[0] == PATH_SEPARATOR)
            relativePathSafe = relativePathSafe.substr(1);

    this->relativePath = relativePathSafe;
}

FilePtr File::open()
{
    return FilePtr(*this);
}


std::string File::getExtension(bool convertToLowercase) const
{
    unsigned long extPos = (unsigned long) relativePath.find_last_of(".");
    if (extPos == string::npos) return "";
    if ((relativePath.length() - extPos) > 5) return "";

    string ext = relativePath.substr(extPos);
    if (convertToLowercase)
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return ext;
}

std::string File::getFileName()
{
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


FilePtr::FilePtr(const File& file)
{
	int ret = 0;
#ifdef STEGO_OS_WIN
	fileHandle = nullptr;
    ret = fopen_s(&fileHandle, file.getAbsolutePath().c_str(), "r+b");		
#else
	if ( (fileHandle = fopen(file.getAbsolutePath().c_str(), "r+b")) == nullptr )
		ret = errno;
#endif
	if ( ret != 0 ) {
		LOG_ERROR("FilePtr::FilePtr: cannot open file '" << file.getAbsolutePath() << "': " << strerror(errno));
		throw std::runtime_error("FilePtr::FilePtr: cannot open file '" + file.getAbsolutePath() + "': " + strerror(errno));
	}
}


FilePtr::~FilePtr()
{
    if (fileHandle != nullptr)
    {
        fclose(fileHandle);
        fileHandle = nullptr;
    }
}



};
