/**
* @file test_assert_helper.h
* @author Matus Kysel
* @date 2016
* @brief Helper test assertation
*
*/

#ifndef TEST_ASSERT_HELPER_H
#define TEST_ASSERT_HELPER_H

#include <iostream>

#define STEGO_TEST_CHECK_ABORT(expr) { if (!(expr)) { std::cout << __FILE__ << ':' << __LINE__ << ':' << #expr << std::endl; abort(); } } ((void)0)

#define STEGO_TEST_CHECK(expr, ret) { if (!(expr)) { std::cout << __FILE__ << ':' << __LINE__ << ':' << #expr << std::endl; return ret; } } ((void)0)

#endif // TEST_ASSERT_HELPER_H
