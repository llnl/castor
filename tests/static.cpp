// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

template <typename T>
struct StaticMembersT
{
	static int glorp;

	#pragma castor ensures is_integral(T)
	#pragma castor ensures x == checked(old(x) - 1)
	#pragma castor writes x
	static void dec(T& x)
	{
		x -= 1;
	}
};

struct StaticMembers
{
	static int glorp;
	const static bool always_true = true;

	#pragma castor ensures x == checked(old(x) + 1)
	#pragma castor writes x
	static void inc(int& x)
	{
		x += 1;
	}
};

void caller()
{
	int x = 0;
	StaticMembers::inc(x);
	#pragma castor assert x == 1
	x = 5;
	StaticMembersT<int>::dec(x);
	#pragma castor assert x == 4

	StaticMembers::glorp = 4;
	StaticMembersT<int>::glorp = 5;
	int first = StaticMembers::glorp, second = StaticMembersT<int>::glorp;
	#pragma castor assert first == 4 /\ second == 5

	bool always_true = StaticMembers::always_true;
	#pragma castor assert always_true
}
