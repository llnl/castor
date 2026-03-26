// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct Pair
{
	int p1;
	int p2;

	#pragma castor invariant valid(this)
	#pragma castor ensures this->p1 == p1
	#pragma castor ensures this->p2 == p2
	Pair(int p1, int p2)
	{
		this->p1 = p1;
		this->p2 = p2;
	}
};

void foo()
{
	const Pair pair(4, 5);
	const_cast<Pair*>(&pair)->p2 = 6;
}
