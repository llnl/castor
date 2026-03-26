// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor invariant valid(a, b) /\ separated(a, b)
#pragma castor ensures *a == old(*b) /\ *b == old(*a)
#pragma castor writes *a, *b
template <typename T>
void swap(T* a, T* b)
{
	auto temp = *a;
	*a = *b;
	*b = temp;
}

#pragma castor invariant !alias_of(a, b)
#pragma castor ensures a == old(b) /\ b == old(a)
#pragma castor writes a, b
template <typename T>
void swap_ref(T& a, T& b)
{
	auto temp = a;
	a = b;
	b = temp;
}

#pragma castor invariant !alias_of(x, y)
void do_swap(int& x, int& y)
{
	swap(&x, &y);
	swap_ref(x, y);
	#pragma castor assert unchanged(x) /\ unchanged(y)
}

template void swap<int>(int* a, int* b);
template void swap<short>(short* a, short* b);
template void swap_ref<int>(int& a, int& b);
template void swap_ref<short>(short& a, short& b);
