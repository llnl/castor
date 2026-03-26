// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires 0 <= a /\ a < (max_sint32 / 2) /\ 0 <= b /\ b < (max_sint32 / 2)
#pragma castor ensures result == a + b
int mutability_demo(int a, int b)
{
	#pragma castor assert exists sint32: i. a == i
	int c = a;
	c = c + b;
	return c;
}

#pragma castor requires 0 <= a /\ a < (max_sint32 / 3) /\ 0 <= b /\ b < (max_sint32 / 3)
#pragma castor ensures result == a + b + c
int simple_add(int a, int b, short c)
{
	return a + b + c;
}

#pragma castor ensures result == 4
int four()
{
	return 2 + 2;
}

#pragma castor ensures result == 0
#pragma castor ensures result != 1
int blank_init()
{
	int x;
	x = 0;
	return x;
}
