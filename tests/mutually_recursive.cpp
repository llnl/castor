// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

int foo(int x);

#pragma castor no_write
#pragma castor no_free
#pragma castor requires x >= 0
#pragma castor ensures result == x
#pragma castor variant x
int bar(int x)
{
	return (x == 0) ? 0 : 1 + foo(x - 1);
}

#pragma castor no_write
#pragma castor no_free
#pragma castor requires x >= 0
#pragma castor ensures result == x
#pragma castor variant x
int foo(int x)
{
	return (x == 0) ? 0 : 1 + bar(x - 1);
}
