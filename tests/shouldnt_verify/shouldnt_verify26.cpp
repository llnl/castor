// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void tester()
{
	#pragma castor invariant 0 <= i /\ i < 10
	#pragma castor variant 10 - i
	for (int i = 0; i < 10; i += 1) { ; }
}
