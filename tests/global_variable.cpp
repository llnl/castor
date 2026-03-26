// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

int x = 6;

void check_x(int y)
{
	x = 5;
	y = 6;
	#pragma castor assert x == 5
	#pragma castor assert y == 6
}

void call_check_x()
{
	check_x(0);
}

#pragma castor ensures x == checked(old(x) + 1)
#pragma castor writes x
#pragma castor no_free
void incr()
{
	x += 1;
}

void incr_test()
{
	x = 0;
	incr();
	#pragma castor assert x == 1
}

#pragma castor requires x < max_sint32
void incr_test_2()
{
	int y = x;
	incr();
	#pragma castor assert x == y + 1
}

const int const_two = 2;
const int const_global = const_two + 3;

#pragma castor ensures const_global == 5
void const_global_is_5() { }
