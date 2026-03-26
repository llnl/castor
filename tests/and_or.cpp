// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires valid(a, b) /\ separated(a, b)
#pragma castor requires *a == 0
#pragma castor ensures unchanged(*a) /\ unchanged(*b)
void and_test(int *a, int *b)
{
	#pragma castor assert *a == 0 /\ true
	if (*a > 0 && (*b += 1)) { }
}

#pragma castor requires valid(a, b) /\ separated(a, b)
#pragma castor requires *a == 0
#pragma castor ensures unchanged(*a) /\ *b == checked(old(*b) + 1)
void and_test_2(int *a, int *b)
{
	#pragma castor assert *a == 0 /\ true
	if (*a == 0 && (*b += 1)) { }
}

#pragma castor requires valid(a, b) /\ separated(a, b)
#pragma castor requires *a == 0
#pragma castor ensures unchanged(*a) /\ unchanged(*b)
void or_test(int *a, int *b)
{
	#pragma castor assert *a == 0 \/ true
	if (*a == 0 || (*b += 1)) { }
}

#pragma castor requires valid(a, b) /\ separated(a, b)
#pragma castor requires *a == 0
#pragma castor ensures unchanged(*a) /\ *b == checked(old(*b) + 1)
void or_test_2(int *a, int *b)
{
	#pragma castor assert *a == 0 \/ true
	if (*a > 0 || (*b += 1)) { }
}
