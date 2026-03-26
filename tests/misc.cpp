// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == 1
int block()
{
	int x = 0;

	{
		int y = 1;
		x = y;
	}

	return x;
}

#pragma castor ensures result == 1
int bool_test()
{
	int a = 1 == 1;

	return a;
}

#pragma castor ensures result == 5
#pragma castor requires a == 3
int five(int a);

#pragma castor requires a == 3
#pragma castor ensures result == 5
int five(int a)
{
	return a + 2;
}

#pragma castor requires valid(x)
void deref_index(int *x)
{
	auto x1 = *(x + 0);
	auto x2 = x[0];
	#pragma castor assert x1 == x2
}

void pointers_not_equal()
{
	int x;
	int y;
	#pragma castor assert &x > &y \/ &x < &y
}

void unsigned_neg()
{
	unsigned int x = 12345;
	auto neg_x = -x;

	#pragma castor assert neg_x == 4294954951

	auto neg_x_2 = -(int)x;

	#pragma castor assert neg_x_2 == -12345

	auto neg_0 = -0u;
	
	#pragma castor assert neg_0 == 0
}


struct Foo
{
public:
	#pragma castor invariant valid(this)
	#pragma castor no_free
	#pragma castor no_write
	Foo() { }

	#pragma castor invariant valid(this)
	#pragma castor ensures alias_of(result, *this)
	#pragma castor no_free
	#pragma castor no_write
	Foo& do_nothing(int x)
	{
		return *this;
	}
};

void function_order()
{
	int x = 0;
	Foo foo = Foo();
	foo.do_nothing(x = 1).do_nothing(x = 2);

	#pragma castor assert x == 2
}

#pragma castor ensures alias_of(result, x)
#pragma castor no_write
int& create_reference(int& x) { return x; }

void creates_reference()
{
	int x = 5;
	int& y = create_reference(x);
	#pragma castor assert alias_of(x, y)
}

void nth_works()
{
	int x = 5;
	#pragma castor assert nth(x, 0)
	#pragma castor assert !nth(x, 1)
	#pragma castor assert nth(x, 2)
}

void assignment_yields_lvalue()
{
	int x;
	(x = 0) = 1;
	#pragma castor assert x == 1
}

void types()
{
	wchar_t mwchar;
	char16_t mchar16;
	char32_t mchar32;
	char mchar;
	unsigned char muchar;
	signed char mschar;
	short mshort;
	unsigned short mushort;
	signed short msshort;
	int mint;
	unsigned int muint;
	signed int msint;
	long mlong;
	unsigned long mulong;
	signed long mslong;
	long long mllong;
	unsigned long long mullong;
	signed long long msllong;
	bool mbool;
}

void increment_bool()
{
	bool x = false;
	++x;
	#pragma castor assert x
}

unsigned int ctr;

#pragma castor requires ctr < max_uint32
#pragma castor ensures alias_of(result, x)
#pragma castor ensures ctr == old(ctr) + 1
#pragma castor writes ctr
int& wrapper(int& x) { ++ctr; return x; }

void pre_increment()
{
	ctr = 0;
	int foo = 42;
	++wrapper(foo);
	#pragma castor assert ctr == 1
	#pragma castor assert foo == 43
}

void pre_decrement()
{
	ctr = 0;
	int foo = 42;
	--wrapper(foo);
	#pragma castor assert ctr == 1
	#pragma castor assert foo == 41
}

void post_increment()
{
	ctr = 0;
	int foo = 42;
	int old_foo = wrapper(foo)++;
	#pragma castor assert ctr == 1
	#pragma castor assert old_foo == 42
	#pragma castor assert foo == 43
}

void post_decrement()
{
	ctr = 0;
	int foo = 42;
	int old_foo = wrapper(foo)--;
	#pragma castor assert ctr == 1
	#pragma castor assert old_foo == 42
	#pragma castor assert foo == 41
}

