// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

static_assert(false); // disable this test

void consumes(int& x, int& y) { }

void produces()
{
	int x = 0;
	consumes(x, x);
}
