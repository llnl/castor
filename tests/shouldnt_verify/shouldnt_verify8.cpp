// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct Foo
{
	int a;
	int b;

	#pragma castor invariant valid(this)
	#pragma castor no_free
	#pragma castor writes this->a, this->b
	#pragma castor ensures this->a == 0 /\ this->b == 0
	Foo()
	{
		a = b = 0;
	}

	#pragma castor invariant valid(this)
	#pragma castor no_free
	#pragma castor writes this->a, this->b
	#pragma castor ensures this->a == x /\ this->b == y
	Foo(int x, int y)
	{
		a = x;
		b = y;
	}

	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor no_free
	#pragma castor ensures result <-> (f == *this)
	#pragma castor ensures unchanged(*this) /\ unchanged(f)
	bool operator==(Foo& f)
	{
		return f.a == this->a && f.b == this->b;
	}
};

void foo()
{
	Foo f(1, 2);
	Foo g;

	#pragma castor assert f.a == 1 /\ f.b == 2

	g = f;

	#pragma castor assert f.a == 1 /\ f.b == 2
	#pragma castor assert g.a == 1 /\ g.b == 2
	#pragma castor assert !(g.a == 1 /\ g.b == 2)
}
