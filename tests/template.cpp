// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires is_sint8(a) /\ is_sint8(b)
#pragma castor ensures result == a + b
#pragma castor no_write
template <class T>
T add(T a, T b)
{
	return a + b;
}

#pragma castor requires x < max_int(T) - 1
#pragma castor ensures result == x + 1
#pragma castor no_write
template<class T>
T inc(T x)
{
	return x + 1;
}

#pragma castor invariant valid(a, b) /\ separated(a, b)
#pragma castor writes *a, *b
#pragma castor ensures old(*a) == *b /\ old(*b) == *a
template<class T>
void swap(T *a, T *b)
{
	T temp = *a;
	*a = *b;
	*b = temp;
}

#pragma castor ensures is_integral(T)
#pragma castor no_write
template <typename T>
void inst_check(T x) { }

struct Dummy { };

#pragma castor ensures !is_integral(T) /\ is_class(T)
#pragma castor no_write
template <typename T>
void inst_check_2(T x) { }

#pragma castor ensures is_pointer(T)
#pragma castor no_write
template <typename T>
void inst_check_3(T x) { }

void runner()
{
	add<int>(1, 2);
	add<short>(3, 4);
	inc<long long>(5);
}

template void swap<int>(int *a, int *b);
template void swap<long long>(long long *a, long long *b);
template void swap<char>(char *a, char *b);
template void inst_check<int>(int x);
template void inst_check_2<Dummy>(Dummy x);
template void inst_check_3<int*>(int *x);
