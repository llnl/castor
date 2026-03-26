// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == x
int foo(const int x)
{
	return x;
}

void modify_through_const_ptr()
{
	int x = 6;
	int * const x_ptr = &x;
	*const_cast<int*>(x_ptr) = 12;
	#pragma castor assert x == 12
	#pragma castor assert x_ptr == &x
}

#pragma castor invariant valid(x)
#pragma castor no_write
#pragma castor no_free
void accepts_const_int_ptr(const int* x) { }

void gives_const_int_ptr()
{
	int x = 5;

	accepts_const_int_ptr(&x);

	x = 6;
	#pragma castor assert x == 6
}

#pragma castor requires x == 5
#pragma castor no_write
#pragma castor no_free
void accepts_const_reference(const int& x) { }

void gives_const_reference()
{
	accepts_const_reference(5);
}

#pragma castor ensures valid(&result) => result == 42
const int& temporary_materialized()
{
	return 42;
}

struct Foo
{
	static const int x = 5;
};

struct Bar
{
	static const int x = 6;
};

void checks_struct_vars()
{
	int x1 = Foo::x;
	int x2 = Bar::x;
	#pragma castor assert x1 == 5 /\ x2 == 6
}

#pragma castor ensures result == checked(x + 1)
int const_vcs(const int& x)
{
	return x + 1;
}
