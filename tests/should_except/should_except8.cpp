// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void foo()
{
	#pragma castor writes i
	#pragma castor no_write
	for (int i = 0; i < 10; ++i) ;
}
