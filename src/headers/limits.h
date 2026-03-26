// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _LIMITS_H
#define _LIMITS_H 1

#define CHAR_BIT 8

#define SCHAR_MAX 127
#define SCHAR_MIN (-128)

#define UCHAR_MAX 255

#define CHAR_MAX SCHAR_MAX
#define CHAR_MIN SCHAR_MIN

#define MB_LEN_MAX 8

#define SHRT_MAX 32767
#define SHRT_MIN (-32768)

#define USHRT_MAX 65535

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)

#define UINT_MAX 4294967295U

#define LONG_MAX 9223372036854775807L
#define LONG_MIN (-LONG_MAX - 1L)

#define ULONG_MAX 18446744073709551615UL

#define LLONG_MAX 9223372036854775807LL
#define LLONG_MIN (-LLONG_MAX - 1LL)

#define ULLONG_MAX 18446744073709551615ULL

#endif

