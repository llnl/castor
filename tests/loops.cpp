// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == 55
int for_loop()
{
	int running_total = 0;

	#pragma castor variant 10 - i
	#pragma castor invariant is_sint32(i * (i - 1))
	#pragma castor invariant running_total == (i * (i - 1)) / 2
	#pragma castor invariant 1 <= i /\ i <= 11
	#pragma castor writes i, running_total
	#pragma castor no_free
	for (int i = 1; i <= 10; i += 1)
	{
		running_total += i;
	}

	return running_total;
}

/*#pragma castor ensures result == 55
int for_loop_continue()
{
	int running_total = 0;

	#pragma castor variant 10 - i
	#pragma castor invariant is_sint32(i * (i - 1))
	#pragma castor invariant running_total == (i * (i - 1)) / 2
	#pragma castor invariant 1 <= i /\ i <= 11
	#pragma castor writes i, running_total
	#pragma castor no_free
	for (int i = 1; i <= 10; i += 1)
	{
		running_total += i;
		continue;
	}

	return running_total;
}*/

#pragma castor ensures result == 55
int for_loop_double_initializer()
{
	int running_total = 0;

	#pragma castor variant 10 - i
	#pragma castor invariant is_sint32(i * (i - 1))
	#pragma castor invariant running_total == (i * (i - 1)) / 2
	#pragma castor invariant 1 <= i /\ i <= 11
	#pragma castor writes i, running_total, condition
	#pragma castor no_free
	for (int i = 1; bool condition = i <= 10; i += 1)
	{
		running_total += i;
		#pragma castor assert condition
	}

	return running_total;
}

#pragma castor ensures result == 55
int while_loop()
{
	int running_total = 0;
	int i = 1;

	#pragma castor variant 10 - i
	#pragma castor invariant is_sint32(i * (i - 1))
	#pragma castor invariant running_total == (i * (i - 1)) / 2
	#pragma castor invariant 1 <= i /\ i <= 11
	#pragma castor writes i, running_total
	#pragma castor no_free
	while (i <= 10)
	{
		running_total += i;
		i += 1;
	}

	return running_total;
}

#pragma castor ensures result == 55
int while_loop_initializer()
{
	int running_total = 0;
	int i = 1;

	#pragma castor variant 10 - i
	#pragma castor invariant is_sint32(i * (i - 1))
	#pragma castor invariant running_total == (i * (i - 1)) / 2
	#pragma castor invariant 1 <= i /\ i <= 11
	#pragma castor writes i, running_total, condition
	#pragma castor no_free
	while (bool condition = i <= 10)
	{
		running_total += i;
		i += 1;
		#pragma castor assert condition
	}

	return running_total;
}

#pragma castor ensures result == 55
int do_while_loop()
{
	int running_total = 0;
	int i = 1;

	#pragma castor variant 10 - i
	#pragma castor invariant is_sint32(i * (i - 1))
	#pragma castor invariant running_total == (i * (i - 1)) / 2
	#pragma castor invariant 1 <= i /\ i <= 11
	#pragma castor writes i, running_total
	#pragma castor no_free
	do
	{
		running_total += i;
		i += 1;
	}
	while (i <= 10);

	return running_total;
}

#pragma castor requires iters <= max_sint32
int do_while_loop_2(unsigned iters)
{
	int x = 0;
	int ctr = 0;
	#pragma castor variant iters - x
	#pragma castor invariant ctr == x
	#pragma castor invariant 1 <= x /\ x <= iters + 1
	#pragma castor writes ctr, x
	do ctr++, x++;
	while (x < iters);
	#pragma castor assert ctr > 0
}

/*void do_while_continue()
{
	#pragma castor variant 0
	do continue; while (0);
}*/

void infinite_loop()
{
	#pragma castor variant 0
	while (true)
	{
		break;
	}

	#pragma castor variant 0
	while (1)
	{
		break;
	}
}

