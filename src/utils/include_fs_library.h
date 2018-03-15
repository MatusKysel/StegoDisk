#ifndef STEGODISK_UTILS_INCLUDE_FS_LIBRARY_H_
#define STEGODISK_UTILS_INCLUDE_FS_LIBRARY_H_

//PSTODO toto by sa malo dat transformovat na cmake-time, ale chceme to? aj tak by to malo co najskor to bude mozne vypadnut uplne...
#ifdef __has_include

#  if __has_include(<filesystem>)
#    include <filesystem>
#    ifdef __cpp_lib_filesystem
#      define HAS_FILESYSTEM_LIBRARY
       namespace fs = std::filesystem;
#    endif //__cpp_lib_filesystem
#  endif //__has_include(<filesystem>)

#  ifndef HAS_FILESYSTEM_LIBRARY
#    if __has_include(<experimental/filesystem>)
#      include <experimental/filesystem>
#      ifdef __cpp_lib_experimental_filesystem
#        define HAS_FILESYSTEM_LIBRARY
         namespace fs = std::experimental::filesystem;
#      endif //__cpp_lib_filesystem
#    endif //__has_include(<experimental/filesystem>)
#  endif //HAS_FILESYSTEM_LIBRARY

#endif //__has_include

#endif //STEGODISK_UTILS_INCLUDE_FS_LIBRARY_H_
