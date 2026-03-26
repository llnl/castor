// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

constexpr int global = 5;

void reads_global()
{
	#pragma castor assert global == 5
}

struct Foo
{
	static constexpr int x = 5;
};

struct Bar
{
	static constexpr int x = 6;
};

constexpr void checks_struct_vars()
{
	int x1 = Foo::x;
	int x2 = Bar::x;
	#pragma castor assert x1 == 5 /\ x2 == 6
}
