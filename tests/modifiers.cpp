// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void modifiers()
{
	volatile unsigned y = 7;

	#pragma castor assert y == 7
}
