// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == -1
int test4()
{
	int ctr = 0;

	#pragma castor invariant ctr == 2 * i
	#pragma castor invariant 0 <= i /\ i <= 5
	#pragma castor variant 10 - i
	for (int i = 0; i < 10; i += 1)
	{
		ctr += 1;
		if (i == 5)
			return ctr;
		ctr += 1;
	}

	return -1;
}
