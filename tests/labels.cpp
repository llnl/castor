// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void value_at()
{
	int x = 5, y = 0;
should_be_5:
	x = 6;
should_be_6:
	x = y = 7;

	#pragma castor assert at(x, @should_be_5) == 5 /\ at(y, @should_be_5) == 0
	#pragma castor assert at(x, @should_be_6) == 6
	#pragma castor assert x == 7 /\ y == 7
}
