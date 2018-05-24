# this one is important
#SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
#SET(CMAKE_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION})

# specify the cross compiler
SET(CMAKE_C_COMPILER   $ENV{GCC_BIN_LOC}/bin/gcc)
SET(CMAKE_CXX_COMPILER $ENV{GCC_BIN_LOC}/bin/g++)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH $ENV{GCC_BIN_LOC}/)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
