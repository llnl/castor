// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <cstddef>

void nullptr_test()
{
	int* test = nullptr;
	*test = 6;
}
