// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor no_write
void requires_non_const(int& x) { }

void provides_const()
{
	constexpr int x = 0;
	requires_non_const(*const_cast<int*>(&x));
}
