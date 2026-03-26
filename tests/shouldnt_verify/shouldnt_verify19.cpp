// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct Pair
{
	int x;
	int y;

	#pragma castor invariant valid(this)
	#pragma castor no_write
	Pair() { }
};

#pragma castor ensures valid(result)
#pragma castor ensures result->x == 4
#pragma castor ensures result->y == 6
#pragma castor no_write
Pair* returns_struct_ptr()
{
	Pair my_pair;
	my_pair.x = 4;
	my_pair.y = 6;
	return &my_pair;
}

void obtains_struct_ptr()
{
	Pair new_pair = *returns_struct_ptr();
	#pragma castor assert new_pair.x == 4
	#pragma castor assert new_pair.y == 6
}
