// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

typedef unsigned int DATA;

void tester()
{
	int x = 6;
	int *y = &x;
	#pragma castor assert y == &x
	#pragma castor assert *y == x
	x = 7;
	int z = *y;
	#pragma castor assert z == 7
	*y = 2;
	#pragma castor assert x == 2
}

#pragma castor requires x < max_uint32 - 1
#pragma castor ensures x == old(x) + 1
void adder_simple(DATA x)
{
	DATA *y = &x;
	*y = *y + 1;
}

#pragma castor writes *x
#pragma castor no_free
#pragma castor invariant valid(x)
#pragma castor requires *x < max_uint32 - 1
#pragma castor requires is_uint32(*x)
#pragma castor ensures *x == old(*x) + 1
void adder(DATA *x)
{
	DATA **y = &x;
	**y = **y + 1;
}

#pragma castor requires x < max_uint32 - 1
#pragma castor ensures result == old(x) + 1
#pragma castor ensures x == old(x) + 1
#pragma castor ensures result == x
DATA get_addr_assign(DATA x)
{
	adder(&x);
	return x;
}

#pragma castor requires x < max_uint32 - 1
#pragma castor ensures x == old(x) + 1
void get_addr_assign_explicit(DATA x)
{
	DATA *y = &x;
	adder(y);
}

#pragma castor requires valid(x, *x)
#pragma castor requires 0 < **x /\ **x < max_uint32 - 1
#pragma castor ensures **x == old(**x) + 1
void deref_assign(DATA **x)
{
	adder(*x);
}

#pragma castor requires x < max_uint32 - 1
#pragma castor ensures x == old(x)
#pragma castor ensures result == x + 1
DATA unchanged(DATA x)
{
	DATA y = x;
	adder(&y);
	#pragma castor assert y == old(x) + 1
	return y;
}

#pragma castor requires valid(x)
#pragma castor requires 0 < *x /\ *x < max_uint32 - 1
#pragma castor ensures *x == old(*x) + 1
void copy(DATA *x)
{
	DATA *y = x;
	adder(y);
}

void valid_axiom_1(int a)
{
	int* b = &a;
	#pragma castor assert valid(b)
}

#pragma castor requires valid(a)
void valid_axiom_2(int *a)
{
	int *b = a;
	#pragma castor assert valid(b)
}

void valid_axiom_3()
{
	int a = 0;
	int *b = &a;
	#pragma castor assert valid(b)
}

/*void valid_axiom_4()
{
	int *a = new int(0);
	#pragma castor assert valid(a)
	delete a;
	#pragma castor assert !valid(a)
}*/
