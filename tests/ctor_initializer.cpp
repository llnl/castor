// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct Foo
{
	int a;

	#pragma castor invariant valid(this)
	#pragma castor writes this->a
	#pragma castor ensures this->a == x
	Foo(int x) : a(x) { }

	#pragma castor invariant valid(this)
	#pragma castor no_write
	~Foo() { }
};

struct Bar
{
	Foo foo;

	#pragma castor invariant valid(this)
	#pragma castor writes this->foo
	#pragma castor ensures this->foo.a == x
	Bar(int x) : foo(x) { }

	#pragma castor invariant valid(this)
	#pragma castor no_write
	~Bar() = default;
};

void bar()
{
	Bar b(42);
	#pragma castor assert b.foo.a == 42
}

enum class Enum : unsigned char { A = 1, B, C };

void nonclass()
{
	int i(6);
	#pragma castor assert i == 6

	bool b(false);
	#pragma castor assert !b

	int* ptr(nullptr);
	#pragma castor assert ptr == 0

	Enum enm(Enum::B);
	#pragma castor assert enm == 2
}
