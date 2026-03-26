// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <climits>
#include <cstdint>

void test()
{
	#pragma castor assert CHAR_MIN == min_sint8
	#pragma castor assert CHAR_MAX == max_sint8
	#pragma castor assert SHRT_MIN == min_sint16
	#pragma castor assert SHRT_MAX == max_sint16
	#pragma castor assert INT_MIN  == min_sint32
	#pragma castor assert INT_MAX  == max_sint32
	auto long_min = LONG_MIN;
	#pragma castor assert long_min == min_sint64
	auto long_max = LONG_MAX;
	#pragma castor assert long_max == max_sint64
	auto llong_min = LLONG_MIN;
	#pragma castor assert llong_min == min_sint64
	auto llong_max = LLONG_MAX;
	#pragma castor assert llong_max == max_sint64
	
	#pragma castor assert SCHAR_MIN == min_sint8
	#pragma castor assert SCHAR_MAX == max_sint8
	#pragma castor assert UCHAR_MAX == max_uint8
	#pragma castor assert USHRT_MAX == max_uint16
	auto uint_max = UINT_MAX;
	#pragma castor assert uint_max  == max_uint32
	auto ulong_max = ULONG_MAX;
	#pragma castor assert ulong_max == max_uint64
	auto ullong_max = ULLONG_MAX;
	#pragma castor assert ullong_max == max_uint64

	{
		int8_t int8;
		#pragma castor assert min_int(int8) == min_sint8 /\ max_int(int8) == max_sint8
		uint8_t uint8;
		#pragma castor assert min_int(uint8) == min_uint8 /\ max_int(uint8) == max_uint8
		int16_t int16;
		#pragma castor assert min_int(int16) == min_sint16 /\ max_int(int16) == max_sint16
		uint16_t uint16;
		#pragma castor assert min_int(uint16) == min_uint16 /\ max_int(uint16) == max_uint16
		int32_t int32;
		#pragma castor assert min_int(int32) == min_sint32 /\ max_int(int32) == max_sint32
		uint32_t uint32;
		#pragma castor assert min_int(uint32) == min_uint32 /\ max_int(uint32) == max_uint32
		int64_t int64;
		#pragma castor assert min_int(int64) == min_sint64 /\ max_int(int64) == max_sint64
		uint64_t uint64;
		#pragma castor assert min_int(uint64) == min_uint64 /\ max_int(uint64) == max_uint64
	}

	{
		std::int8_t int8;
		#pragma castor assert min_int(int8) == min_sint8 /\ max_int(int8) == max_sint8
		std::uint8_t uint8;
		#pragma castor assert min_int(uint8) == min_uint8 /\ max_int(uint8) == max_uint8
		std::int16_t int16;
		#pragma castor assert min_int(int16) == min_sint16 /\ max_int(int16) == max_sint16
		std::uint16_t uint16;
		#pragma castor assert min_int(uint16) == min_uint16 /\ max_int(uint16) == max_uint16
		std::int32_t int32;
		#pragma castor assert min_int(int32) == min_sint32 /\ max_int(int32) == max_sint32
		std::uint32_t uint32;
		#pragma castor assert min_int(uint32) == min_uint32 /\ max_int(uint32) == max_uint32
		std::int64_t int64;
		#pragma castor assert min_int(int64) == min_sint64 /\ max_int(int64) == max_sint64
		std::uint64_t uint64;
		#pragma castor assert min_int(uint64) == min_uint64 /\ max_int(uint64) == max_uint64
	}
}
