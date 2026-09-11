enable_testing()

include(TestStegoMacros.cmake)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#add_definitions(-DRANDOM_TESTS=10)

if(NOT TEST_DIR)
	if(NOT WIN32)
		set(DST_DIRECTORY /tmp/)
	else()
		set(DST_DIRECTORY "$ENV{TEMP}/")
	endif()
else()
	set(DST_DIRECTORY ${TEST_DIR})
endif()

set(SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/img)
set(PASSWORD "heslo")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/test_config.h @ONLY)

###################################################################################################################################
###################################################################################################################################

message(STATUS "TEST_PATH_PREFIX: ${DST_DIRECTORY}")

add_stego_test(LsbMixedFeistelWPassword "lsb" "mix_feistel" 1)
add_stego_test(LsbIdentityWPassword "lsb" "identity" 1)
add_stego_test(LsbAffine64WPassword "lsb" "affine64" 1)
add_stego_test(LsbAffineWPassword "lsb" "affine" 1)
add_stego_test(LsbNumericFeistelWPassword "lsb" "num_feistel" 1)
add_stego_test(HammingMixedFeistelWPassword "hamming" "mix_feistel" 1)
add_stego_test(HammingIdentityWPassword "hamming" "identity" 1)
add_stego_test(HammingAffine64WPassword "hamming" "affine64" 1)
add_stego_test(HammingAffineWPassword "hamming" "affine" 1)
add_stego_test(HammingNumericFeistelWPassword "hamming" "num_feistel" 1)

# Same matrix without a password.
add_stego_test(LsbMixedFeistelNoPassword "lsb" "mix_feistel" 0)
add_stego_test(LsbIdentityNoPassword "lsb" "identity" 0)
add_stego_test(LsbAffine64NoPassword "lsb" "affine64" 0)
add_stego_test(LsbAffineNoPassword "lsb" "affine" 0)
add_stego_test(LsbNumericFeistelNoPassword "lsb" "num_feistel" 0)
add_stego_test(HammingMixedFeistelNoPassword "hamming" "mix_feistel" 0)
add_stego_test(HammingIdentityNoPassword "hamming" "identity" 0)
add_stego_test(HammingAffine64NoPassword "hamming" "affine64" 0)
add_stego_test(HammingAffineNoPassword "hamming" "affine" 0)
add_stego_test(HammingNumericFeistelNoPassword "hamming" "num_feistel" 0)

# Different global and local permutations.
add_stego_test_ex(LsbGlobalMixFeistelLocalIdentity "lsb" "mix_feistel" "identity" 1)
add_stego_test_ex(LsbGlobalIdentityLocalMixFeistel "lsb" "identity" "mix_feistel" 1)
add_stego_test_ex(LsbGlobalAffineLocalAffine64 "lsb" "affine" "affine64" 1)
add_stego_test_ex(HammingGlobalAffine64LocalNumFeistel "hamming" "affine64" "num_feistel" 1)
add_stego_test_ex(HammingGlobalNumFeistelLocalAffine64 "hamming" "num_feistel" "affine64" 1)
add_stego_test_ex(HammingGlobalMixFeistelLocalAffine "hamming" "mix_feistel" "affine" 1)

# Partial loads, small payloads and non-zero offsets.
add_stego_test_ex(LsbHalfCapacity "lsb" "mix_feistel" "mix_feistel" 1 --percent 50)
add_stego_test_ex(HammingHalfCapacity "hamming" "mix_feistel" "mix_feistel" 1 --percent 50)
add_stego_test_ex(LsbSmallPayload "lsb" "affine64" "affine64" 1 --gen_file_size 4096)
add_stego_test_ex(HammingSmallPayload "hamming" "affine64" "affine64" 1 --gen_file_size 4096)
add_stego_test_ex(LsbOffsetWrite "lsb" "mix_feistel" "mix_feistel" 1 --offset 4096 --gen_file_size 32768)
add_stego_test_ex(HammingOffsetWrite "hamming" "mix_feistel" "mix_feistel" 1 --offset 4096 --gen_file_size 32768)

# Payload that is the bitwise inverse of the current carrier content.
add_stego_test_ex(LsbInvertedData "lsb" "num_feistel" "num_feistel" 1 --invert)
add_stego_test_ex(HammingInvertedData "hamming" "num_feistel" "num_feistel" 1 --invert)

# Reopening with a different password must not yield the payload.
add_stego_test_ex(LsbWrongPassword "lsb" "mix_feistel" "mix_feistel" 1 --wrong_password)
add_stego_test_ex(HammingWrongPassword "hamming" "mix_feistel" "mix_feistel" 1 --wrong_password)

# Unit tests of the building blocks.
add_stego_unit_test(UnitPermutationIdentity permutations identity)
add_stego_unit_test(UnitPermutationAffine permutations affine)
add_stego_unit_test(UnitPermutationAffine64 permutations affine64)
add_stego_unit_test(UnitPermutationNumFeistel permutations num_feistel)
add_stego_unit_test(UnitPermutationMixFeistel permutations mix_feistel)
add_stego_unit_test(UnitEncoderLsb encoders lsb)
add_stego_unit_test(UnitEncoderHamming encoders hamming)
add_stego_unit_test(UnitFactories factories)

###################################################################################################################################
###################################################################################################################################

add_executable(stego-test stego_test.cc)
add_executable(stego-unit-test stego_unit_test.cc)

if(FUSE_FOUND)
  add_executable(stego-fuse-test stego_fuse_test.cc)
endif()

if(NOT WIN32)
	set(LIBJPEGTURBO_LIBRARIES_STATIC ${CMAKE_BINARY_DIR}/${LIBDIR}/libjpeg.a)
else()
	link_directories(${CMAKE_BINARY_DIR}/${LIBDIR}/*/)
	set(LIBJPEGTURBO_LIBRARIES_STATIC jpeg-static)
endif()

set(STEGO_TEST_LIBRARIES ${STEGODISK_LIBRARY})
if(NOT BUILD_SHARED_LIBS)
  set(STEGO_TEST_LIBRARIES ${STEGO_TEST_LIBRARIES} ${LIBJPEGTURBO_LIBRARIES_STATIC})
endif()
set(STEGO_FUSE_TEST_LIBRARIES ${STEGO_TEST_LIBRARIES} ${FUSE_LIBRARIES})

target_link_libraries(stego-test ${STEGO_TEST_LIBRARIES})
target_link_libraries(stego-unit-test ${STEGO_TEST_LIBRARIES})

if(FUSE_FOUND)
  target_link_libraries(stego-fuse-test ${STEGO_FUSE_TEST_LIBRARIES})
endif()

list(APPEND TESTS stego-test stego-unit-test)

add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} -T test --build-config ${CMAKE_CFG_INTDIR} --test-timeout 600 --output-on-failure --parallel 4 
  DEPENDS ${TESTS})
