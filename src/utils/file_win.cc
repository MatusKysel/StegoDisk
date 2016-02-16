#ifdef _MEM_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
   #ifndef DBG_NEW
      #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
      #define new DBG_NEW
   #endif
#endif  // _DEBUG
#endif // _MEM_LEAKS

#include "stego_header.h"
#include "file.h" //TODO(matus) toto je komplet zle

#ifdef _WIN32

#include <Windows.h>
#include <iostream>

using namespace std;

namespace stego_disk {

  static int AddFilesInDir(std::string basePath, std::string currentPath, std::string mask, std::vector<File>& fileList)
  {
    string basePathSafe = basePath;

    string currentPathSafe = currentPath;
    if (currentPathSafe.length() > 0) {
      if (currentPathSafe[currentPathSafe.length() - 1] == '\\')
        currentPathSafe = currentPathSafe.substr(0, currentPathSafe.length() - 1);
    }
    if (currentPathSafe.length() > 0)
    if (currentPathSafe[0] == '\\')
      currentPathSafe = currentPathSafe.substr(1);

    string path = basePathSafe;
    if (currentPathSafe.length() > 0)
      path = path.append("\\" + currentPathSafe);

	if (path[path.length() - 1] == '\\') {
		path.append(mask);
	}
	else {
      path.append("\\" + mask);
	}

    WIN32_FIND_DATAA ffd;
    LARGE_INTEGER filesize;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    hFind = FindFirstFileA(path.c_str(), &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
      return -1;
    }
    do
    {
      string newCurrentPath = currentPath + "\\" + string(ffd.cFileName);
      string newPath = basePath + newCurrentPath;

      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if ((string(ffd.cFileName) != ".") && (string(ffd.cFileName) != ".."))
          AddFilesInDir(basePath, newCurrentPath, mask, fileList);
      }
      else
      {
        filesize.LowPart = ffd.nFileSizeLow;
        filesize.HighPart = ffd.nFileSizeHigh;
        fileList.push_back(File(basePath, newCurrentPath));
      }
    } while (FindNextFileA(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
      //DisplayErrorBox(TEXT("FindFirstFile"));
    }

    FindClose(hFind);

    return 0;
  }

  // TODO: add wildcard parameter to specify supported file extensions (*.bmp, *.jpg ...)
  std::vector<File> File::GetFilesInDir(std::string path, std::string mask)
  {
    std::vector<File> fileList;
	if (mask.empty())
		mask = "*";

    AddFilesInDir(path, "", mask, fileList);

    return fileList;
  }

} // stego_disk

#endif // _WIN32
