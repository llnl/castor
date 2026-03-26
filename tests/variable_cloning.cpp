// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires a < max_sint32 - 3
#pragma castor ensures result == a + 4
int v(int a)
{
	int b = a;

	{
		int a = b;
		a += 1;

		{
			int a = 0;

			a = 4;
		}

		a += 1;
	}

	return a + 4;
}

#pragma castor ensures result == a
int u(int a)
{
	int b = a;

	{
		int a = b;
		a += 1;
	}

	return a;
}
