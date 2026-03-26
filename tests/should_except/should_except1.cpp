// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

static_assert(false); // disable this test

#pragma castor ensures result == 0
template <typename T>
T return_zero()
{
	return 0;
}

