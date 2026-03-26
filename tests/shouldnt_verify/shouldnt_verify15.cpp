// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void mutates_const_param(const int x)
{
	*const_cast<int*>(&x) = 6;
}
