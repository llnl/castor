// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void loop()
{
	int sum = 0;

	#pragma castor invariant 0 <= i /\ i <= 11
	#pragma castor invariant sum == i * (i - 1) / 2
	#pragma castor writes i, sum
	#pragma castor variant 11 - i
	for (int i = 1; i <= 10; i += 1)
		sum += i;

	#pragma castor assert sum == 55
}

