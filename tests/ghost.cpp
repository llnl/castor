// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void foo(int x)
{
	#pragma castor ghost
	int y = x + 1;
	#pragma castor assert y == x + 1
}
