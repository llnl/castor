// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void out_of_scope()
{
	int *x_ptr;

	{
		int x = 0;
		x_ptr = &x;
	}

	#pragma castor assert valid(x_ptr)
}
