// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Foo
{
public:
	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor no_free
	Foo() { }

	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor no_free
	void do_nothing() { }
};

void call_non_const_func_on_const_object()
{
	const Foo foo;

	const_cast<Foo*>(&foo)->do_nothing();
}
