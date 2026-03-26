// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
#pragma castor ensures result == 0
int deleted_deref()
{
	int *i = new int(0);
	delete i;
	return *i;
}
#else
static_assert(false);
#endif
