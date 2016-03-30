cmake_minimum_required(VERSION 2.8)

project(StegoDisk)

option(VERSION_NAMES "Append version into binary/library name" OFF)
option(WITH_FUSE "Build this library with FUSE" ON)
option(BUILD_SHARED "Build this as shared library" OFF)
option(BUILD_TESTS "Whether to build automatic test suite" ON)


if(BUILD_SHARED)
  message (STATUS "Build this library as shared")
  set(BUILD_SHARED_LIBS ON)
else()
  message (STATUS "Build this library as static")
  set(BUILD_SHARED_LIBS OFF)
endif()

cmake_policy(SET CMP0009 NEW)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

if(NOT WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()

set(STEGODISK_MAJOR_VERSION 0)
set(STEGODISK_MINOR_VERSION 0)
set(STEGODISK_PATCH_VERSION 1)
set(STEGODISK_VERSION ${STEGODISK_MAJOR_VERSION}.${STEGODISK_MINOR_VERSION}.${STEGODISK_PATCH_VERSION})
set(STEGODISK_RELEASE 0)

if(NOT WIN32)
  set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
endif()

math(EXPR STEGODISK_VERSION_NUMBER "${STEGODISK_MAJOR_VERSION} * 1000000 + ${STEGODISK_MINOR_VERSION} * 1000 + ${STEGODISK_PATCH_VERSION}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in ${CMAKE_BINARY_DIR}/src/version.h)
include_directories(${CMAKE_BINARY_DIR}/src) #this is for version.h file



set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

if(ADD_CMAKE_MODULE_PATH)
  set(CMAKE_MODULE_PATH ${ADD_CMAKE_MODULE_PATH})
endif()

if(NOT WIN32)
    if(NOT LIBDIR)
        set(LIBDIR "lib")
    endif()
else()
    if(NOT LIBDIR)
        set(LIBDIR "bin")
    endif()
endif()


if(NOT BUILD_DIR)
  set(BUILD_DIR ${CMAKE_INSTALL_PREFIX})
endif()

if(QT_CREATOR_PROJECT_SHOW)
  file(GLOB_RECURSE SRCS *.cc)
  file(GLOB_RECURSE HDRS *.h)
  add_custom_target(DummyHeaderShow SOURCES ${SRCS} ${HDRS})
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

################ CXX FLAGS ##################
#include(CXXCompilerFlags)

if(NOT WIN32)
  set(warnings "-Wall -Wextra") #-Werror
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -pthread ${warnings}")
endif()

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  if (BUILD_SHARED_LIBS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DBUILD_SHARED_LIBS")
  endif()
endif()
################ CXX FLAGS ##################

################ LIBJPEGTURBO ##################
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/libjpeg-turbo/CMakeLists.txt.orig")
  message(STATUS "Library 'libjpeg-turbo' already patched.")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/libjpeg-turbo/CMakeLists.txt")
  execute_process(COMMAND patch -b ${CMAKE_CURRENT_SOURCE_DIR}/lib/libjpeg-turbo/CMakeLists.txt ${CMAKE_CURRENT_SOURCE_DIR}/patches/libjpeg-turbo/libjpeg-turbo.CMakeLists.txt.diff ERROR_QUIET OUTPUT_QUIET)
else()
  message(FATAL_ERROR "Unable to find 'libjpeg-turbo' library.")
endif()

option(ENABLE_SHARED OFF)
option(WITH_SIMD OFF)
option(WITH_TURBOJPEG OFF)
add_subdirectory(lib/libjpeg-turbo)
set(LIBJPEGTURBO_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/lib/libjpeg-turbo)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/libjpeg-turbo)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/libjpeg-turbo)
################ LIBJPEGTURBO ##################

################ FUSE ##################
if(WITH_FUSE)
  find_package(FUSE)
  if(NOT FUSE_FOUND)
    message (STATUS "Unable to find fuse.h")
  else()
    include_directories(${FUSE_INCLUDE_DIRS})
  endif()
else()
  message (STATUS "Build this library without FUSE")
 set(FUSE_FOUND FALSE)
endif()
################ FUSE ##################

################ LodePNG ##################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/lodepng)
set(LODEPNG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/lib/lodepng/lodepng.cpp ${CMAKE_CURRENT_SOURCE_DIR}/lib/lodepng/lodepng.h)
################ LodePNG ##################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src) #this makes src root dir

##############################################################################################################################
##############################################################################################################################
##############################################################################################################################


# CARRIER_FILES

set(CARRIER_FILES_HDRS
  src/carrier_files/carrier_file_bmp.h
  src/carrier_files/carrier_file.h
  src/carrier_files/carrier_file_factory.h
  src/carrier_files/carrier_file_jpeg.h
  src/carrier_files/carrier_file_png.h
)

set(CARRIER_FILES_SRCS
  src/carrier_files/carrier_file_bmp.cc
  src/carrier_files/carrier_file.cc
  src/carrier_files/carrier_file_factory.cc
  src/carrier_files/carrier_file_jpeg.cc
  src/carrier_files/carrier_file_png.cc
)

# ENCODERS

set(ENCODERS_HDRS
  src/encoders/encoder.h
  src/encoders/encoder_factory.h
  src/encoders/hamming_encoder.h
  src/encoders/lsb_encoder.h
)

set(ENCODERS_SRCS
  src/encoders/encoder.cc
  src/encoders/encoder_factory.cc
  src/encoders/hamming_encoder.cc
  src/encoders/lsb_encoder.cc
)

# FITNESS FUNCTIONS

set(FITNESS_HDRS
  src/fitness/fitness.h
  src/fitness/context_fitness.h
)

set(FITNESS_SRCS
  src/fitness/context_fitness.cc
)

# FILE_MANAGEMENT

set(FILE_MANAGEMENT_HDRS
  src/file_management/carrier_files_manager.h
)

set(FILE_MANAGEMENT_SRCS
  src/file_management/carrier_files_manager.cc
)

# FUSE

if(FUSE_FOUND)
  set(FUSE_HDRS
    src/fuse/fuse_service.h
#    src/fuse/fuse_service_delegate.h
  )

  set(FUSE_SRCS
    src/fuse/fuse_service.cc
#    src/fuse/fuse_service_delegate.cc
  )
endif()

# HASH

set(HASH_HDRS
  src/hash/hash.h
  src/hash/hash_impl.h
  src/hash/keccak_hash_impl.h
)

set(HASH_SRCS
  src/hash/hash.cc
  src/hash/hash_impl.cc
  src/hash/keccak_hash_impl.cc
)

# KEYS

set(KEYS_HDRS
  src/keys/key.h
)

set(KEYS_SRCS
  src/keys/key.cc
)

# LOGGING

set(LOGGING_HDRS
  src/logging/logger.h
)

set(LOGGING_SRCS
  src/logging/logger.cc
)

# PERMUTATIONS

set(PERMUTATIONS_HDRS
  src/permutations/affine_permutation.h
  src/permutations/affine64_permutation.h
  src/permutations/feistel_mix_permutation.h
  src/permutations/feistel_num_permutation.h
  src/permutations/identity_permutation.h
  src/permutations/permutation.h
  src/permutations/permutation_factory.h
)

set(PERMUTATIONS_SRCS
  src/permutations/affine_permutation.cc
  src/permutations/affine64_permutation.cc
  src/permutations/feistel_mix_permutation.cc
  src/permutations/feistel_num_permutation.cc
  src/permutations/identity_permutation.cc
  src/permutations/permutation.cc
  src/permutations/permutation_factory.cc
)

# UTILS

set(UTILS_HDRS
  src/utils/config.h
  src/utils/file.h
  src/utils/json.h
  src/utils/json_object.h
  src/utils/memory_buffer.h
  src/utils/stego_config.h
  src/utils/stego_errors.h
  src/utils/stego_header.h
  src/utils/stego_math.h
  src/utils/stego_types.h
  src/utils/thread_pool.h
  src/utils/keccak/keccak.h
)

set(UTILS_SRCS
  src/utils/file.cc
  src/utils/file_unix.cc
  src/utils/file_win.cc
  src/utils/memory_buffer.cc
  src/utils/stego_math.cc
  src/utils/keccak/keccak.cc
)

# VIRTUAL_STORAGE

set(VIRTUAL_STORAGE_HDRS
  src/virtual_storage/virtual_storage.h
)

set(VIRTUAL_STORAGE_SRCS
  src/virtual_storage/virtual_storage.cc
)

## STEGODISK_LIBRARY ##
set(STEGODISK_HDRS
  src/stego_storage.h
  ${ENCODERS_HDRS}
  ${FITNESS_HDRS}
  ${HASH_HDRS}
  ${KEYS_HDRS}
  ${LOGGING_HDRS}
  ${PERMUTATIONS_HDRS}
  ${UTILS_HDRS}
  ${CARRIER_FILES_HDRS}
  ${VIRTUAL_STORAGE_HDRS}
  ${FILE_MANAGEMENT_HDRS}
)

set(STEGODISK_SRCS
  src/stego_storage.cc
  ${ENCODERS_SRCS}
  ${FITNESS_SRCS}
  ${HASH_SRCS}
  ${KEYS_SRCS}
  ${LOGGING_SRCS}
  ${PERMUTATIONS_SRCS}
  ${UTILS_SRCS}
  ${CARRIER_FILES_SRCS}
  ${VIRTUAL_STORAGE_SRCS}
  ${FILE_MANAGEMENT_SRCS}
)

if(FUSE_FOUND)
  set(STEGODISK_HDRS ${STEGODISK_HDRS} ${FUSE_HDRS})
  set(STEGODISK_SRCS ${STEGODISK_SRCS} ${FUSE_SRCS})
endif()

##############################################################################################################################
##############################################################################################################################
##############################################################################################################################
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(FUSE_FOUND)
  set(THIRD_LIBRARIES ${PLATFORM_LIBS} ${LIBJPEGTURBO_LIBRARIES} ${LODEPNG_LIBRARIES} ${FUSE_LIBRARIES})
else()
  set(THIRD_LIBRARIES ${PLATFORM_LIBS} ${LIBJPEGTURBO_LIBRARIES} ${LODEPNG_LIBRARIES})
endif()


if(APPLE)
   find_library(COREFOUNDATION_LIBRARY CoreFoundation)
   list(APPEND THIRD_LIBRARIES ${COREFOUNDATION_LIBRARY})
endif(APPLE)

if(VERSION_NAMES)
  set(STEGODISK_LIBRARY stego-disk.${STEGODISK_VERSION})
else()
  set(STEGODISK_LIBRARY stego-disk)
endif()

add_library(${STEGODISK_LIBRARY} ${STEGODISK_SRCS} ${STEGODISK_HDRS} ${LODEPNG_SRC})

if(REQUIRE_LIB_SYMBOL)
  target_link_libraries(${STEGODISK_LIBRARY} ${THIRD_LIBRARIES})
endif()


#install(TARGETS ${STEGODISK_LIBRARY}
#  LIBRARY
#  ARCHIVE DESTINATION ${LIBDIR}
#  LIBRARY DESTINATION ${LIBDIR}
#  RUNTIME DESTINATION ${LIBDIR})

##############################################################################################################################
##############################################################################################################################
##############################################################################################################################
# TESTING

if(BUILD_TESTS)
  add_subdirectory(src/tests)
endif()
