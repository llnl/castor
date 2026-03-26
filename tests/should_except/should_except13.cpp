// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void foo()
{
	int* x = new int(0);
	char* y = reinterpret_cast<char*>(x);
}
