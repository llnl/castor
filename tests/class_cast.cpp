// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Foo
{
	int x;

public:
	#pragma castor invariant valid(this)
	#pragma castor writes this->x
	#pragma castor no_free
	#pragma castor ensures this->x == 0
	Foo() : x(0) { }

	#pragma castor invariant valid(this)
	#pragma castor writes this->x
	#pragma castor no_free
	#pragma castor ensures this->x == a
	Foo(int a) : x(a) { }
};

class Bar
{
	int x;

public:
	#pragma castor invariant valid(this)
	#pragma castor writes this->x
	#pragma castor no_free
	#pragma castor ensures this->x == 0
	Bar() : x(0) { }

	#pragma castor invariant valid(this)
	#pragma castor writes this->x
	#pragma castor no_free
	#pragma castor ensures this->x == a
	Bar(int a) : x(a) { }

	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor no_free
	#pragma castor ensures result.x == checked(this->x + 1)
	explicit operator Foo()
	{
		return Foo(x + 1);
	}
};

void converter()
{
	Bar bar(42);
	Foo foo = static_cast<Foo>(bar);
	#pragma castor assert foo.x == 43
}
