cmake_minimum_required(VERSION 2.8)

project(StegoDisk)

option(VERSION_NAMES "Append version into binary/library name" OFF)

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

configure_file(${CMAKE_SOURCE_DIR}/src/version.h.in ${CMAKE_BINARY_DIR}/src/version.h)
include_directories(${CMAKE_BINARY_DIR}/src) #this is for version.h file

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

if(NOT WIN32)
	set(BUILD_SHARED_LIBS ON)
else()
	set(BUILD_SHARED_LIBS OFF)
endif()

if(NOT BUILD_DIR)
  set(BUILD_DIR ${CMAKE_INSTALL_PREFIX})
endif()

if(QT_CREATOR_PROJECT_SHOW)
  file(GLOB_RECURSE SRCS *.cc)
  file(GLOB_RECURSE HDRS *.h)
  add_custom_target(DummyHeaderShow SOURCES ${SRCS} ${HDRS})
endif()

################ CXX FLAGS ##################
#include(CXXCompilerFlags)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")

if(WIN32)
	if (BUILD_SHARED_LIBS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DBUILD_SHARED_LIBS")
	endif()
endif()
################ CXX FLAGS ##################

include_directories(${CMAKE_SOURCE_DIR}/src) #this makes src root dir

##############################################################################################################################
##############################################################################################################################
##############################################################################################################################

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
  src/permutations/permutation.h
  src/permutations/permutation_factory.h
  src/permutations/identity_permutation.h
  src/permutations/affine_permutation.h
  src/permutations/affine64_permutation.h
  src/permutations/feistel_num_permutation.h
  src/permutations/feistel_mix_permutation.h
)

set(PERMUTATIONS_SRCS
  src/permutations/permutation.cc
  src/permutations/permutation_factory.cc
  src/permutations/identity_permutation.cc
  src/permutations/affine_permutation.cc
  src/permutations/affine64_permutation.cc
  src/permutations/feistel_num_permutation.cc
  src/permutations/feistel_mix_permutation.cc
)

# UTILS

set(UTILS_HDRS
  src/utils/cpp11_compat.h
  src/utils/config.h
  src/utils/file.h
  src/utils/memory_buffer.h
  src/utils/stego_errors.h
  src/utils/stego_header.h
  src/utils/stego_math.h
  src/utils/stego_system_params.h
  src/utils/stego_types.h
  src/utils/keccak/keccak.h
)

set(UTILS_SRCS
  src/utils/file.cc
  src/utils/file_unix.cc
  src/utils/file_win.cc
  src/utils/memory_buffer.cc
  src/utils/stego_math.cc
  src/utils/stego_system_params.cc
  src/utils/keccak/keccak.cc
)

## STEGODISK_LIBRARY ##
set(STEGODISK_HDRS
  ${HASH_HDRS}
  ${KEYS_HDRS}
  ${LOGGING_HDRS}
  ${PERMUTATIONS_HDRS}
  ${UTILS_HDRS}
)

set(STEGODISK_SRCS
  ${HASH_SRCS}
  ${KEYS_SRCS}
  ${LOGGING_SRCS}
  ${PERMUTATIONS_SRCS}
  ${UTILS_SRCS} 
)

#set(THIRD_LIBRARIES ${PLATFORM_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LOG4CPLUS_LIBRARIES} ${SQLITE_LIBRARIES} ${ZLIB_LIBRARIES})

if(APPLE)
   find_library(COREFOUNDATION_LIBRARY CoreFoundation)
   list(APPEND THIRD_LIBRARIES ${COREFOUNDATION_LIBRARY})
endif(APPLE)

if(VERSION_NAMES)
  set(STEGODISK_LIBRARY stego-disk.${STEGODISK_VERSION})
else()
  set(STEGODISK_LIBRARY stego-disk)
endif()

add_library(${STEGODISK_LIBRARY} ${STEGODISK_SRCS} ${STEGODISK_HDRS})

if(REQUIRE_LIB_SYMBOL)
  target_link_libraries(${STEGODISK_LIBRARY} ${PLATFORM_LIBS} ${THIRD_LIBRARIES})
endif()


#install(TARGETS ${STEGODISK_LIBRARY}
#  LIBRARY
#  ARCHIVE DESTINATION ${LIBDIR}
#  LIBRARY DESTINATION ${LIBDIR}
#  RUNTIME DESTINATION ${LIBDIR})
