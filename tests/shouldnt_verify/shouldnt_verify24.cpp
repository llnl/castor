// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

const unsigned char x = 0xFF;

void foo()
{
	#pragma castor assert x == -1
}
