// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == 17
int oob_positive()
{
	int arr[1];
	int i;
	arr[2] = 17;
	return i;
}

