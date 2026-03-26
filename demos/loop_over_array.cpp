// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor invariant valid_array(array, size)
#pragma castor requires size > 0
#pragma castor ensures forall uint32: i. i < size => array[i] == 0
#pragma castor writes array[0 .. size - 1]
void init_to_0(int *array, int size)
{
	#pragma castor invariant valid_array(array, size)
	#pragma castor invariant 0 <= i /\ i <= size
	#pragma castor invariant unchanged(size)
	#pragma castor invariant forall uint32: idx. idx < i => array[idx] == 0
	#pragma castor writes array[0 .. size - 1], i
	#pragma castor variant size - i
	for (int i = 0; i < size; i += 1)
		array[i] = 0;
}
