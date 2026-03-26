// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == 0
int test1()
{
	if (2 + 2 == 4)
		return 0;
	else
		return 1;
}

#pragma castor ensures alias_of(result, x)
int& test1_ref(int& x, int& y)
{
	if (2 + 2 == 4)
		return x;
	else
		return y;
}

#pragma castor ensures result == 0
int test2()
{
	if (2 + 2 != 4)
		return 1;

	return 0;
}

#pragma castor ensures alias_of(result, x)
int& test2_ref(int& x, int& y)
{
	if (2 + 2 != 4)
		return y;

	return x;
}

#pragma castor ensures result == 0
int test3()
{
	int x = 0;
	return x;
	x = 1;
	return x;
}

#pragma castor ensures result == 0
int& test3_ref(int& x)
{
	x = 0;
	return x;
	x = 1;
	return x;
}

#pragma castor ensures result == 11
int test4()
{
	int ctr = 0;

	#pragma castor invariant ctr == 2 * i
	#pragma castor invariant 0 <= i /\ i <= 5
	#pragma castor variant 10 - i
	#pragma castor writes i, ctr
	#pragma castor no_free
	for (int i = 0; i < 10; i += 1)
	{
		ctr += 1;
		if (i == 5)
			return ctr;
		ctr += 1;
	}

	return -1;
}
