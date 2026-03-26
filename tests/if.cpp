// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures a == b => result == a
#pragma castor ensures a != b => result == b
int test_if(int a, int b)
{
	int c = 0;
	if (a == b)
		c = a;
	else
		c = b;
	return c;
}

#pragma castor ensures a == b => result == a
#pragma castor ensures a != b => result == b
int test_if_2(int a, int b)
{
	int c = b;
	if (a == b)
	{
		c = 0;
		c = a;
	}
	return c;
}

#pragma castor ensures result == 0
int test_if_truthy()
{
	int a;
	if (1)
		a = 0;
	return a;
}

void test_constexpr_if()
{
	constexpr int x = 5;
	constexpr int y = 6;
	if constexpr (x > y)
	{
		#pragma castor assert false
	}
	else
	{
		#pragma castor assert true
	}
}

void test_decl_if()
{
	int path;

	if (bool f = (2 + 2 == 4))
	{
		path = 1;
		#pragma castor assert f
	}
	else
	{
		path = 2;
		#pragma castor assert !f
	}

	#pragma castor assert path == 1
}

#pragma castor requires param == 5
void test_decl_if_2(int param)
{
	if (auto newparam = param - 4)
	{
		#pragma castor assert newparam == 1
	}
	else
	{
		#pragma castor assert newparam != 1
	}
}

void test_ternary()
{
	int x = 2 + 2 == 4 ? 6 : 7;
	#pragma castor assert x == 6
	x = 2 + 2 != 4 ? 6 : 7;
	#pragma castor assert x == 7

	x = 1;
	int y = 2;
	int z = true ? x : y;
	#pragma castor assert z == 1
	z = 3;
	#pragma castor assert x == 1 /\ y == 2 /\ z == 3

	(false ? x : y) += 2;
	#pragma castor assert x == 1 /\ y == 4 /\ z == 3

	decltype((true ? x : y)) t = true ? x : y;
	t = 2;
	#pragma castor assert x == 2 /\ y == 4 /\ z == 3
}

