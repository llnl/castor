// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

enum struct Three { one = 1, two, three };

void three_test()
{
	Three x = Three::two;

	#pragma castor assert x == 2
}

struct HasEnumDecl
{
	enum InnerEnum { five = 5, seven = 7 };
};

void innerenum_test(HasEnumDecl* x)
{
	auto five = x->five;
	auto seven = HasEnumDecl::seven;

	#pragma castor assert five == 5 /\ seven == 7
};

enum ShortEnum : short { hello, there, readers };

void short_test()
{
	ShortEnum s = hello;
	auto size_of_there = sizeof(there);
	auto size_of_three = sizeof(Three::three);
	#pragma castor assert size_of_there == 2
	#pragma castor assert size_of_three == 4
}
