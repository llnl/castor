// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct Foo
{
private:
	int x;
	int y;

public:
	#pragma castor invariant valid(this)
	#pragma castor no_write
	Foo() = default;
	
	#pragma castor invariant valid(this)
	#pragma castor writes this->x, this->y
	#pragma castor ensures this->x == a
	#pragma castor ensures this->y == b
	Foo(int a, int b)
	{
		x = a;
		y = b;
	}
};

struct Bar
{
private:
	Foo x;

public:
	#pragma castor invariant valid(this)
	#pragma castor no_write
	Bar() = default;

	#pragma castor invariant valid(this)
	#pragma castor writes this->x
	#pragma castor ensures this->x.x == a
	#pragma castor ensures this->x.y == b
	Bar(int a, int b)
	{
		Foo baz(a, b);
		x = baz;
	}

	Bar(Bar& b) = delete;
};
