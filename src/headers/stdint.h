// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _STDINT_H
#define _STDINT_H

typedef char          int8_t;
typedef short int     int16_t;
typedef int           int32_t;
typedef long long int int64_t;

typedef int8_t  int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef int64_t intmax_t;
typedef int64_t intptr_t;

typedef unsigned char          uint8_t;
typedef unsigned short int     uint16_t;
typedef unsigned int           uint32_t;
typedef unsigned long long int uint64_t;

typedef uint8_t  uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

typedef uint8_t  uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

typedef uint64_t uintmax_t;
typedef uint64_t uintptr_t;

#endif
