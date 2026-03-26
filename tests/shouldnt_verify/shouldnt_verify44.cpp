// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor no_write
#pragma castor ensures result == 5
#pragma castor ensures valid(&result)
int&& foo()
{
	return 5;
}

void bar()
{
	int x = foo();
}
