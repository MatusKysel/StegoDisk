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

add_test(NAME StateConfigurationReset COMMAND stego-state-test config)
add_test(NAME StateCapacityEstimate COMMAND stego-state-test capacity)
add_test(NAME StateEncoderOverride COMMAND stego-state-test encoder_override)
add_test(NAME StateReopen COMMAND stego-state-test reopen)
add_test(NAME StateFirstLoad COMMAND stego-state-test first_load)
add_test(NAME StateIoBounds COMMAND stego-state-test io_bounds)
add_test(NAME StateLoadFailure COMMAND stego-state-test load_failure)
add_test(NAME PngMetadataLifetime COMMAND stego-png-lifetime-test)
add_test(NAME UnitMemoryBufferRandomize COMMAND stego-randomize-test)

# Select the project suite separately from the bundled JPEG library's tests.
get_property(STEGO_TEST_NAMES DIRECTORY PROPERTY TESTS)
set_tests_properties(${STEGO_TEST_NAMES} PROPERTIES LABELS stegodisk)

###################################################################################################################################
###################################################################################################################################

add_executable(stego-test stego_test.cc)
add_executable(stego-unit-test stego_unit_test.cc)
add_executable(stego-state-test stego_state_test.cc)
add_executable(stego-png-lifetime-test stego_png_lifetime_test.cc)
add_executable(stego-randomize-test stego_randomize_test.cc)

if(FUSE_FOUND)
  add_executable(stego-fuse-test stego_fuse_test.cc)
endif()

# The library target carries its JPEG dependency, including configuration paths.
target_link_libraries(stego-test PRIVATE ${STEGODISK_LIBRARY})
target_link_libraries(stego-unit-test PRIVATE ${STEGODISK_LIBRARY})
target_link_libraries(stego-state-test PRIVATE ${STEGODISK_LIBRARY})
target_link_libraries(stego-png-lifetime-test PRIVATE ${STEGODISK_LIBRARY})
target_link_libraries(stego-randomize-test PRIVATE ${STEGODISK_LIBRARY})

if(FUSE_FOUND)
  target_link_libraries(stego-fuse-test PRIVATE ${STEGODISK_LIBRARY} ${FUSE_LIBRARIES})
endif()

list(APPEND TESTS stego-test stego-unit-test stego-state-test)
list(APPEND TESTS stego-png-lifetime-test)
list(APPEND TESTS stego-randomize-test)

add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --test-dir "${CMAKE_CURRENT_BINARY_DIR}"
    --build-config "$<CONFIG>" -L stegodisk --timeout 600
    --output-on-failure --parallel 4 --no-tests=error
  DEPENDS ${TESTS}
  USES_TERMINAL
  VERBATIM)
