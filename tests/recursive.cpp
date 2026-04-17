// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires n >= 0
#pragma castor requires is_uint8(n)
#pragma castor variant n
#pragma castor ensures result == n * (n + 1) / 2
#pragma castor ensures is_uint16(result)
#pragma castor no_write
int sum(int n)
{
	if (n == 0)
		return 0;
	else
		return n + sum(n - 1);
}
