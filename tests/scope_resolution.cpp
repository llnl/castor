// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

int x;

int different_x(int x)
{
	auto& x_alias = ::x;
	#pragma castor assert !alias_of(x_alias, x)
}
