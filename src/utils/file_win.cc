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

  // <mask> must be in a form: "*", "*.jpg", "*.bmp" ...
  static int addFilesInDir(std::string basePath, std::string currentPath, std::string mask, std::vector<File>& fileList)
  {
    // basePathSafe contains / at the end of the string
    string basePathSafe = basePath;

    //if (basePathSafe.empty())
    //	basePathSafe = ".";

    //if (basePathSafe[basePathSafe.length() - 1] != '\\')
    //	basePathSafe = basePathSafe.append("\\");

    // currentPathSafe doesnt contain / at the end of the string nor at the beginning
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
      path = path.append("\\"+currentPathSafe);

    path.append("\\"+mask);

    WIN32_FIND_DATAA ffd;
    LARGE_INTEGER filesize;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    //cout << "path: " << path << endl;

    // Find the first file in the directory.

    hFind = FindFirstFileA(path.c_str(), &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
      //cout << "invalid handle value" << endl;
      return -1;
    }

    // List all the files in the directory with some info about them.

    do
    {
      string newCurrentPath = currentPath + "\\" + string(ffd.cFileName);
      string newPath = basePath + newCurrentPath;

      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        //printf("  %s   <DIR>\n", ffd.cFileName);
        if ((string(ffd.cFileName) != ".") && (string(ffd.cFileName) != ".."))
          addFilesInDir(basePath, newCurrentPath, mask, fileList);
      }
      else
      {
        filesize.LowPart = ffd.nFileSizeLow;
        filesize.HighPart = ffd.nFileSizeHigh;
        //printf("  %s   %ld bytes\n", ffd.cFileName, filesize.QuadPart);
        //_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);

        // TODO: sedi <mask> s koncovkou subora?
        fileList.push_back(File(basePath, newCurrentPath));
      }
    } while (FindNextFileA(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
      //DisplayErrorBox(TEXT("FindFirstFile"));
    }

    FindClose(hFind);





    /*
    // otvorime adresar
    dir = opendir(path.c_str());
    if (!dir) {
      fprintf(stderr, "Error: opendir '%s': chyba pri otvarani adresara\n", path.c_str());
      return -1;
      //TODO: throw exception
    }

    while ((de = readdir(dir)) != NULL) {
      string newCurrentPath = currentPath + "/" + string(de->d_name);
      string newPath = basePath + newCurrentPath;
      // nechceme sa vnarat do adresarov, ktore su nalinkovane, pretoze by sme museli
      // komplikovane zistovat, ci nenastane zacyklenie...
      // preto pouzijeme zistovanie info o samotnom objekte fs, nie o pripadnom cieli
      if (lstat(newPath.c_str(), &sb) == -1) {
        fprintf(stderr, "Error: printDir: chyba zistovania info o objekte fs\n");
        //TODO: throw exception
        return -1;
      }

      // ak je polozka adresarom, tak sa rekurzivne vnorime
      if (S_ISDIR(sb.st_mode)) {
        // nesmieme sa vnorit do toho isteho adresara ani o uroven vyssie
        if (strcmp(de->d_name, ".") && strcmp(de->d_name, "..")) {
          // treba otestovat opravnenie na prava citania a pristupu do adresara
          if (access(newPath.c_str(), X_OK | R_OK) != -1) {
            // otestovat navratovu hodnotu na chybove stavy
            ret = addFilesInDir(basePath, newCurrentPath, fileList);
            if (ret < 0) {
              closedir(dir);
              //TODO: throw exception
              return ret;
            }
          }
        }
      } // if S_ISDIR
      else {
        fileList.push_back(File(basePath, newCurrentPath));
      }

    } // while readdir

    // zavrieme adresar
    closedir(dir);
    */

    return 0;
  }

  // TODO: add wildcard parameter to specify supported file extensions (*.bmp, *.jpg ...)
  std::vector<File> File::getFilesInDir(std::string path, std::string mask)
  {
    std::vector<File> fileList;

    addFilesInDir(path, "", mask, fileList);

    return fileList;
  }

} // stego_disk

#endif // _WIN32
