// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires is_sint32(a + b)
#pragma castor requires !alias_of(a, b)
#pragma castor ensures a == old(a) + b
#pragma castor writes a
void add(int& a, const int& b)
{
	a += b;
}

#pragma castor ensures result == x / 2
unsigned int shift_right(unsigned int x)
{
	return x >> 1;
}
