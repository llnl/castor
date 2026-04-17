// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor writes arr[0 .. len - 1]
#pragma castor invariant valid_array(arr, len)
#pragma castor requires len > 0
#pragma castor ensures forall sint32: i. 0 <= i /\ i < len => arr[i] == checked(old(arr[i]) + 1)
void add_one(int *arr, int len)
{
	#pragma castor variant len - i
	#pragma castor invariant valid_array(arr, len)
	#pragma castor invariant forall sint32: a. 0 <= a /\ a < i => arr[a] == checked(old(arr[a]) + 1)
	#pragma castor invariant forall sint32: a. i <= a /\ a < len => arr[a] == old(arr[a])
	#pragma castor invariant 0 <= i /\ i <= len
	#pragma castor invariant arr == old(arr) /\ len == old(len)
	#pragma castor writes arr[0 .. len - 1], i
	for (int i = 0; i < len; i += 1)
	{
		arr[i] += 1;
		#pragma castor assert false
	}
}

