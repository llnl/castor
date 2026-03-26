// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void foo()
{
	int arr[10];
	int sum = 0;

	for (const auto& i : arr)
	{
		#pragma castor assert false
	}
}
