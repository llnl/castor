// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct A
{
	static int x;

	#pragma castor invariant valid(this)
	A()
	{
		x = 0;

		static_assert(false, "Disabling this test.");
	}
};
