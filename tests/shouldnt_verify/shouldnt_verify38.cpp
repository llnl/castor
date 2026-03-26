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
	#pragma castor ensures this->a == a /\ this->b == b
	Foo(int a, int b) : a(a), b(b) { }
};

void foo()
{
	Foo f(1, 2);

	#pragma castor assert f.a == f.b
}

