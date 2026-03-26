// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == I * (I + 1) / 2
#pragma castor no_write
template <int I>
int summation()
{
	return I + summation<I - 1>();
}

template <>
int summation<0>()
{
	return 0;
}

#pragma castor ensures result == 15
int sum5()
{
	return summation<5>();
}
