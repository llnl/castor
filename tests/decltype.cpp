// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void id_expr_test()
{
	int x = 0;
	decltype(x) y = x;
	y = 1;

	#pragma castor assert x == 0 /\ y == 1
	#pragma castor assert is_integral(y)
}

void rvalue_expr_test()
{
	int x = 0;
	decltype(0) y = x;
	y = 1;

	#pragma castor assert x == 0 /\ y == 1
	#pragma castor assert is_integral(y)
}

void lvalue_expr_test()
{
	int x = 0;
	decltype((x)) y = x;
	y = 1;

	#pragma castor assert x == 1 /\ y == 1
	#pragma castor assert alias_of(x, y) /\ is_integral(y)
}

void decl_auto_test() /* decltype(auto) is not working right now */
		      /* in the future, replace these decltype with decltype(auto) */
{
	int x = 0;
	decltype(x) y = x;
	y = 1;

	#pragma castor assert x == 0 /\ y == 1
	#pragma castor assert is_integral(y)

	decltype((x)) z = (x);
	z = 2;

	#pragma castor assert x == 2 /\ y == 1 /\ z == 2
	#pragma castor assert !alias_of(x, y) /\ alias_of(x, z)
	#pragma castor assert is_integral(z)
}
