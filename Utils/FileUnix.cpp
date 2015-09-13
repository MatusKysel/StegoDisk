#ifdef STEGO_OS_UNIX

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace StegoDisk {

/*
 * beautiful C core of this awesome method is written by mY
 * ugly C++ part of this method is writen by drunk martin
*/

/*
 *  struct dirent **namelist;
    int n;

    n = scandir(".", &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        while (n−−) {
            printf("%s\n", namelist[n]−>d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
*/

static int addFilesInDir(std::string basePath, std::string currentPath, std::string mask, std::vector<File>& fileList)
{

    DIR *dir;
    struct dirent *de;
    struct stat sb;
    int    ret;

    // basePathSafe contains / at the end of the string
    string basePathSafe = basePath;

    if (basePathSafe.empty())
        basePathSafe = ".";

    if (basePathSafe[basePathSafe.length()-1] != '/')
        basePathSafe = basePathSafe.append("/");

    // currentPathSafe doesnt contain / at the end of the string nor at the beginning
    string currentPathSafe = currentPath;
    if (currentPathSafe[currentPathSafe.length()-1] == '/')
        currentPathSafe = currentPathSafe.substr(0, currentPathSafe.length()-1);

    if (currentPathSafe.length() > 0)
        if (currentPathSafe[0] == '/')
            currentPathSafe = currentPathSafe.substr(1);

    string path = basePathSafe + currentPathSafe;

    // otvorime adresar
    dir = opendir( path.c_str() );
    if ( !dir ) {
        fprintf(stderr,"Error: opendir '%s': chyba pri otvarani adresara\n", path.c_str() );
        return -1;
        //TODO: throw exception
    }

    while ( (de=readdir(dir)) != NULL ) {
        string newCurrentPath = currentPath + "/" + string(de->d_name);
        string newPath = basePath + newCurrentPath;
        // nechceme sa vnarat do adresarov, ktore su nalinkovane, pretoze by sme museli
        // komplikovane zistovat, ci nenastane zacyklenie...
        // preto pouzijeme zistovanie info o samotnom objekte fs, nie o pripadnom cieli
        if ( lstat(newPath.c_str(),&sb) == -1 ) {
            fprintf( stderr, "Error: printDir: chyba zistovania info o objekte fs\n" );
            //TODO: throw exception
            return -1;
        }

        // ak je polozka adresarom, tak sa rekurzivne vnorime
        if ( S_ISDIR(sb.st_mode) ) {
            // nesmieme sa vnorit do toho isteho adresara ani o uroven vyssie
            if ( strcmp(de->d_name,".") && strcmp(de->d_name,"..") ) {
                // treba otestovat opravnenie na prava citania a pristupu do adresara
                if ( access(newPath.c_str(),X_OK|R_OK) != -1 ) {
                    // otestovat navratovu hodnotu na chybove stavy
                    ret = addFilesInDir(basePath, newCurrentPath, mask, fileList);
                    if ( ret < 0 ) {
                        closedir(dir);
                        //TODO: throw exception
                        return ret;
                    }
                }
            }
        } // if S_ISDIR
        else {
			// TODO: over <mask> ci sedi s priponou
            fileList.push_back(File(basePath, newCurrentPath));
        }

    } // while readdir

    // zavrieme adresar
    closedir( dir );

    return 0;
}


// TODO: add wildcard parameter to specify supported file extensions (*.bmp, *.jpg ...)
std::vector<File> File::getFilesInDir(std::string directory, std::string mask)
{
    std::vector<File> fileList;

    addFilesInDir(directory, "", mask, fileList);

    return fileList;
}

};

#endif
