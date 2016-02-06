#if defined(__unix__) || defined(__APPLE__)
#include "file.h" //TODO(matus) cele zle

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <string>
#include <vector>

namespace stego_disk {

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

static int AddFilesInDir(std::string base_path, std::string current_path, std::string mask, std::vector<File>& file_list) {

  DIR *dir;
  struct dirent *de;
  struct stat sb;
  int ret;

  // base_path_safe contains / at the end of the string
  std::string base_path_safe = base_path;

  if (base_path_safe.empty())
    base_path_safe = ".";

  if (base_path_safe[base_path_safe.length()-1] != '/')
    base_path_safe = base_path_safe.append("/");

  // current_path_safe doesnt contain / at the end of the string nor at the beginning
  std::string current_path_safe = current_path;
  if (current_path_safe[current_path_safe.length() - 1] == '/')
    current_path_safe = current_path_safe.substr(0, current_path_safe.length() - 1);

  if (current_path_safe.length() > 0)
    if (current_path_safe[0] == '/')
      current_path_safe = current_path_safe.substr(1);

  std::string path = base_path_safe + current_path_safe;

  // otvorime adresar
  dir = opendir(path.c_str());
  if ( !dir ) {
    fprintf(stderr,"Error: opendir '%s': chyba pri otvarani adresara\n", path.c_str() );
    return -1;
    //TODO: throw exception
  }

  while ( (de = readdir(dir)) != NULL ) {
    std::string new_current_path = current_path + "/" + std::string(de->d_name);
    std::string new_path = base_path + new_current_path;
    // nechceme sa vnarat do adresarov, ktore su nalinkovane, pretoze by sme museli
    // komplikovane zistovat, ci nenastane zacyklenie...
    // preto pouzijeme zistovanie info o samotnom objekte fs, nie o pripadnom cieli
    if ( lstat(new_path.c_str(), &sb) == -1 ) {
      fprintf( stderr, "Error: printDir: chyba zistovania info o objekte fs\n" );
      closedir(dir);
      //TODO: throw exception
      return -1;
    }

    // ak je polozka adresarom, tak sa rekurzivne vnorime
    if ( S_ISDIR(sb.st_mode) ) {
      // nesmieme sa vnorit do toho isteho adresara ani o uroven vyssie
      if ( strcmp(de->d_name, ".") && strcmp(de->d_name, "..") ) {
        // treba otestovat opravnenie na prava citania a pristupu do adresara
        if ( access(new_path.c_str(), X_OK | R_OK) != -1 ) {
          // otestovat navratovu hodnotu na chybove stavy
          ret = AddFilesInDir(base_path, current_path, mask, file_list);
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
      file_list.push_back(File(base_path, new_current_path));
    }

  } // while readdir

  // zavrieme adresar
  closedir( dir );

  return 0;
}


// TODO: add wildcard parameter to specify supported file extensions (*.bmp, *.jpg ...)
std::vector<File> File::GetFilesInDir(std::string directory, std::string mask)
{
  std::vector<File> file_list;

  AddFilesInDir(directory, "", mask, file_list);

  return file_list;
}

} // stego_disk

#endif // defined(__unix__) || defined(__APPLE__)
