// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires valid(x)
void modify_const_parameter(int const* x)
{
	*const_cast<int*>(x) = 6;
}
