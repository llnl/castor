// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <cstddef>

#pragma castor ensures result == b << 4
std::byte test_lsl(std::byte b)
{
	return b << 4;
}

#pragma castor ensures result == b >> 4
std::byte test_lsr(std::byte b)
{
	return b >> 4;
}

#pragma castor ensures result == l & r
std::byte test_and(std::byte l, std::byte r)
{
	return l & r;
}

#pragma castor ensures result == l | r
std::byte test_or(std::byte l, std::byte r)
{
	return l | r;
}

#pragma castor ensures result == l ^ r
std::byte test_xor(std::byte l, std::byte r)
{
	return l ^ r;
}

#pragma castor ensures result == b
int to_int(std::byte b)
{
	return std::to_integer<int>(b);
}
