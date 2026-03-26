// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == 622
int ptr_scrub()
{
	int i = 620;
	int j = 622;
	int *k = &i;
	k += 1;
	return *k;
}
