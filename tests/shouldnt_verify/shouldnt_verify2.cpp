// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == 42
int oob_negative()
{
	int i;
	int arr[1];
	int *arrptr = arr - 1;
	*arrptr = 42;
	return i;
}
