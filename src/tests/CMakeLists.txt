enable_testing()

include(TestStegoMacros.cmake)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#add_definitions(-DRANDOM_TESTS=10)

if(NOT WIN32)
	set(DST_DIRECTORY /tmp/)
else()
	set(DST_DIRECTORY "$ENV{TEMP}/")
endif()

set(SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/img)
set(PASSWORD "heslo")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/test_config.h @ONLY)

###################################################################################################################################
###################################################################################################################################

message(STATUS "TEST_PATH_PREFIX: ${DST_DIRECTORY}")

add_stego_test(LsbMixedFeistelWPassword "LSB" "MixedFeistel" 1)
add_stego_test(LsbIdentityWPassword "LSB" "Identity" 1)
add_stego_test(LsbAffine64WPassword "LSB" "Affine64" 1)
add_stego_test(LsbAffineWPassword "LSB" "Affine" 1)
add_stego_test(LsbNumericFeistelWPassword "LSB" "NumericFeistel" 1)
add_stego_test(HammingMixedFeistelWPassword "Hamming" "MixedFeistel" 1)
add_stego_test(HammingIdentityWPassword "Hamming" "Identity" 1)
add_stego_test(HammingAffine64WPassword "Hamming" "Affine64" 1)
add_stego_test(HammingAffineWPassword "Hamming" "Affine" 1)
add_stego_test(HammingNumericFeistelWPassword "Hamming" "NumericFeistel" 1)

###################################################################################################################################
###################################################################################################################################

add_executable(stego-test stego_test.cc)

if(NOT WIN32)
	set(LIBJPEGTURBO_LIBRARIES_STATIC ${CMAKE_BINARY_DIR}/${LIBDIR}/libjpeg.a)
else()
	link_directories(${CMAKE_BINARY_DIR}/${LIBDIR}/*/)
	set(LIBJPEGTURBO_LIBRARIES_STATIC jpeg-static)
endif()

target_link_libraries(stego-test ${STEGODISK_LIBRARY} ${LIBJPEGTURBO_LIBRARIES_STATIC})
list(APPEND TESTS stego-test)

add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} -T test --timeout 600 --output-on-failure --parallel 4
  DEPENDS ${TESTS})
