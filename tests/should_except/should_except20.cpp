// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires valid(a) /\ valid(b)
#pragma castor ensures *a == 0 /\ *b == 0
#pragma castor no_free
#pragma castor no_free
void foo(int* a, int* b)
{
	*a = *b = 0;
}
