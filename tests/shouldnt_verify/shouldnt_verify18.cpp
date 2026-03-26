// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures *result == 0
#pragma castor ensures valid(result)
int* returns_temporary()
{
	int x = 0;
	return &x;
}

void accepts_temporary()
{
	int x = *returns_temporary();
	#pragma castor assert x == 0
}
