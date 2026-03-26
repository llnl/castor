// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <cstddef>

void nullptr_test()
{
	int* test = nullptr;
	#pragma castor assert test == 0

	bool nullptr_eq_nullptr = nullptr == nullptr;
	#pragma castor assert nullptr_eq_nullptr

	std::nullptr_t null = nullptr;
	#pragma castor assert null == 0
}
