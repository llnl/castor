// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct Foo
{
	int a;
	int b;

	#pragma castor invariant valid(this)
	#pragma castor writes this->a, this->b
	#pragma castor ensures this->a == 0 /\ this->b == 0
	Foo() : a(0), b(0) { }

	#pragma castor invariant valid(this)
	#pragma castor writes this->a, this->b
	#pragma castor ensures this->a == a /\ this->b == b
	Foo(int a, int b);

	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor ensures result <-> (f == *this)
	bool operator==(Foo& f);
};

void foo()
{
	Foo f(1, 2);
	Foo g(3, 4);

	#pragma castor assert f.a == 1 /\ f.b == 2

	g = f;

	#pragma castor assert f.a == 1 /\ f.b == 2
	#pragma castor assert g.a == 1 /\ g.b == 2

	Foo h;
	
	#pragma castor assert h.a == 0 /\ h.b == 0
}

#pragma castor no_write
#pragma castor ensures result.a == 5 /\ result.b == 10
Foo bar();

void baz()
{
	auto f = bar();

	#pragma castor assert f.a == 5 /\ f.b == 10
}

struct Bar
{
	Foo a;

	#pragma castor invariant valid(this)
	#pragma castor writes this->a
	#pragma castor ensures this->a == x
	Bar(Foo x);
};

void bert()
{
	Bar b(Foo(3, 4));

	Foo f;

	#pragma castor assert b.a.a == 3
	#pragma castor assert b.a.b == 4

	#pragma castor assert valid(&b.a)

	f = b.a;

	#pragma castor assert f.a == 3
	#pragma castor assert f.b == 4
}

void brie_ne()
{
	Foo f { 0, 0 };
	Foo g { 0, 1 };

	auto f_g_equal = f == g;
	#pragma castor assert !f_g_equal
	#pragma castor assert f != g
}

void brie_eq()
{
	Foo f { 9, 8 };
	Foo g { 9, 8 };

	auto f_g_equal = f == g;
	#pragma castor assert f_g_equal
	#pragma castor assert f == g
}
