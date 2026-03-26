// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires min_sint64 / 2 < a /\ a < max_sint64 / 2
#pragma castor requires min_sint64 / 2 < b /\ b < max_sint64 / 2
#pragma castor ensures result == a + b
long long add(long long a, long long b)
{
	#pragma castor assert a + b <= max_sint64
	return a + b;
}

#pragma castor requires is_sint16(a)
#pragma castor ensures result == a * b
int mul(int a, short b)
{
	#pragma castor assert a * b <= max_sint32
	return a * b;
}
