// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <cstddef>

#pragma castor ensures result == b
signed char to_int(std::byte b)
{
	return std::to_integer<signed char>(b);
}
