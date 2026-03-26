// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor invariant valid(x, y) /\ separated(x, y)
#pragma castor ensures *x == old(*y) /\ *y == old(*x)
#pragma castor writes *x
void swap(int* x, int* y)
{
	int temp = *x;
	*x = *y;
	*y = temp;
}
