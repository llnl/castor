// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void copy()
{
	int x = 5;
	int& y = x;
	y += 1;

	#pragma castor assert x == 6

	int z = x;
	z += 1;

	#pragma castor assert x == 6
}

#pragma castor ensures x == checked(old(x) + 1)
#pragma castor writes x
#pragma castor no_free
void called(int& x)
{
	x += 1;
}

#pragma castor ensures x == checked(old(x) + 1)
#pragma castor writes x
#pragma castor no_free
void not_ref_called(int x)
{
	x += 1;
}

void caller()
{
	int x = 5;

	called(x);
	#pragma castor assert x == 6

	not_ref_called(x);
	#pragma castor assert x == 6
}

void same_pointers()
{
	int  x = 5;
	int& y = x;

	#pragma castor assert &x == &y
}

class RefAccess
{
private:
	int x;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->x == y
	#pragma castor writes this->x
	#pragma castor no_free
	RefAccess(int y)
	{
		this->x = y;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures alias_of(result, this->x)
	#pragma castor no_free
	#pragma castor no_write
	int& get()
	{
		return this->x;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures result == this->x
	#pragma castor no_free
	#pragma castor no_write
	int get_noref()
	{
		return this->x;
	}
};

void refaccess_test()
{
	RefAccess ra(5);
	int six;

	ra.get() += 1;

	six = ra.get_noref();

	#pragma castor assert six == 6
}

using IntRef = int&;

int global;

#pragma castor ensures alias_of(result, global)
#pragma castor no_write
#pragma castor no_free
IntRef global_getter() { return global; }

#pragma castor ensures a == old(b) /\ b == old(a)
#pragma castor writes a, b
void refswap(int& a, int& b)
{
	int temp = a;
	a = b;
	b = temp;
}

#pragma castor requires valid(b)
#pragma castor ensures a == old(*b) /\ *b == old(a)
#pragma castor writes a, *b
void refptrswap(int& a, int* b)
{
	int temp = a;
	a = *b;
	*b = temp;
}

void calls_refswap()
{
	int x = 42;
	refswap(x, x);
	#pragma castor assert x == 42
}

void calls_refptrswap()
{
	int x = 42;
	refptrswap(x, &x);
	#pragma castor assert x == 42
}

void creates_rref()
{
	int&& x = 5;
	#pragma castor assert x == 5
}

void creates_rref_2()
{
	int x = 5;
	int&& x_ref = static_cast<int&&>(x);

	#pragma castor assert alias_of(x, x_ref)
}

#pragma castor ensures result == 0
int overloaded_ref(int& x) { return 0; }

#pragma castor ensures result == 1
int overloaded_ref(int&& x) { return 1; }

void calls_overloaded_ref()
{
	int x = 0;
	int res1 = overloaded_ref(x);
	int res2 = overloaded_ref(5);

	#pragma castor assert res1 == 0 /\ res2 == 1
}

#pragma castor no_write
#pragma castor ensures alias_of(result, x)
int&& move(int& x)
{
	return static_cast<int&&>(x);
}

#pragma castor no_write
#pragma castor ensures alias_of(result, x)
int&& forward(int&& x) { return static_cast<int&&>(x); }

void call_forward()
{
	int x = 42;
	int&& x_ref = forward(static_cast<int&&>(x));

	#pragma castor assert alias_of(x, x_ref)
}
