// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor writes arr[0 .. len - 1]
#pragma castor no_free
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
	#pragma castor no_free
	for (int i = 0; i < len; i += 1)
		arr[i] += 1;

	return;
}

#pragma castor writes arr[0 .. len - 1]
#pragma castor no_free
#pragma castor invariant valid_array(arr, len)
#pragma castor requires len > 0
#pragma castor requires forall sint32: i. 0 <= i /\ i < len => arr[i] < max_sint32
#pragma castor ensures forall sint32: i. 0 <= i /\ i < len => arr[i] == checked(old(arr[i]) + 1)
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

#define ARRLEN 10

void caller()
{
	int arr[ARRLEN];

	#pragma castor assert forall sint32: a. 0 <= a /\ a < ARRLEN => arr[a] == 0
	add_one(arr, ARRLEN);
	#pragma castor assert forall sint32: a. 0 <= a /\ a < ARRLEN => arr[a] == 1
}

#pragma castor ensures result == 17
int simple_array_test()
{
	char single_elem[1];
	single_elem[0] = 17;
	return single_elem[0];
}

#pragma castor ensures result == 42
int simple_array_test_2()
{
	int two_elems[2];
	two_elems[1] = 43;
	int *first = &two_elems[0];
	int *second = &first[1];
	*second -= 1;
	return two_elems[1];
}
