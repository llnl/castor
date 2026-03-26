// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor writes arr[0 .. len - 1]
#pragma castor no_free
#pragma castor invariant valid_array(arr, len)
#pragma castor requires len > 0
#pragma castor requires forall sint32: i. 0 <= i /\ i < len => arr[i] < max_sint32
#pragma castor ensures forall sint32: i. 0 <= i /\ i < len => arr[i] == checked(old(arr[i]) + 1)
#pragma castor ensures false
void add_one_explicit(int *arr, int len)
{
	int i;

	#pragma castor variant len - i
	#pragma castor invariant forall sint32: a. 0 <= a /\ a < i => arr[a] == old(arr[a]) + 1
	#pragma castor invariant forall sint32: a. i <= a /\ a < len => arr[a] == old(arr[a])
	#pragma castor invariant forall sint32: a. i <= a /\ a < len => arr[a] < max_sint32
	#pragma castor invariant valid_array(arr, len)
	#pragma castor invariant 0 <= i /\ i <= len
	#pragma castor invariant arr == old(arr) /\ len == old(len)
	#pragma castor writes arr[0 .. len - 1], i
	#pragma castor no_free
	for (i = 0; i < len; i += 1)
	{
		int* newref = arr + i;
		*newref += 1;
	}

	return;
}

