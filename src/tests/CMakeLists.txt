enable_testing()

include(TestStegoMacros.cmake)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#add_definitions(-DRANDOM_TESTS=10)

set(DST_DIRECTORY /tmp/)
set(SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/img/)
set(PASSWORD "heslo")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/test_config.h @ONLY)

###################################################################################################################################
###################################################################################################################################

message(STATUS "TEST_PATH_PREFIX: ${DST_DIRECTORY}")

add_stego_test(LsbMixFeistelWPassword "LSB" "MixFeistel" 1)
add_stego_test(HammingMixFeistelWPassword1 "Hamming" "MixFeistel" 1)

###################################################################################################################################
###################################################################################################################################

add_executable(stego-test stego_test.cc)
set(LIBJPEGTURBO_LIBRARIES_STATIC ${CMAKE_BINARY_DIR}/lib/libjpeg.a)
target_link_libraries(stego-test ${STEGODISK_LIBRARY} ${LIBJPEGTURBO_LIBRARIES_STATIC})
list(APPEND TESTS stego-test)

add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} -T test --timeout 600 --output-on-failure --parallel 4
  DEPENDS ${TESTS})
