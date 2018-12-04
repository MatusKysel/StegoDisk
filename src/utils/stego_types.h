/**
* @file stego_types.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Definiton of basic types for this library
*
*/

#pragma once

#include <cstdint>

#include "api_mask.h"

namespace stego_disk {
	using uint8 = std::uint8_t;
	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;
	using uint64 = std::uint64_t;

	using int8 = std::int8_t;
	using int16 = std::int16_t;
	using int32 = std::int32_t;
	using int64 = std::int64_t;
} //stego_disk
