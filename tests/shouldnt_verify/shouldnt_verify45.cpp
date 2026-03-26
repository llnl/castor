// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

int ctr = 0;

#pragma castor requires ctr < max_sint32
#pragma castor ensures alias_of(result, x)
#pragma castor ensures ctr == old(ctr) + 1
#pragma castor writes ctr
int& lhs_evaluated_once_helper(int& x) { ++ctr; return x; }

#pragma castor requires is_sint16(a) /\ is_sint16(b)
#pragma castor ensures a == old(a) + b
void lhs_evaluated_once(int& a, int b)
{
	ctr = 0;
	lhs_evaluated_once_helper(a) += b;
	#pragma castor assert ctr != 1
}
