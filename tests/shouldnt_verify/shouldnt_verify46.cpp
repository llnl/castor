// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void foo()
{
	if (auto one = 1)
	{
		#pragma castor assert one != 1
	}
}
