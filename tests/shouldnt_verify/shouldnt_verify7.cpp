// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
void modify_const_new()
{
	const int* x = new const int(5);
	*const_cast<int*>(x) = 6;
}
#else
static_assert(false); // disable this test
#endif
