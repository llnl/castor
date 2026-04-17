// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor invariant valid(a, b, c) /\ separated(a, b, c)
#pragma castor ensures *a == old(*b) /\ *b == old(*c) /\ *c == old(*a)
#pragma castor writes *a, *b, *c
void swap_3(int *a, int *b, int *c)
{
	int temp = *a;
	*a = *b;
	*b = *c;
	*c = temp;
}

#pragma castor invariant valid(a, *a, b, *b) /\ separated(a, *a, b, *b)
#pragma castor ensures **a == checked(old(**a) + **b)
#pragma castor writes **a
void add_1(int **a, int **b)
{
	**a += **b;
}

#pragma castor invariant valid(a, *a, b, *b) /\ separated(a, *a, b, *b)
#pragma castor ensures **a == checked(old(**a) * **b)
#pragma castor writes **a
void mul(int **a, int **b)
{
	**a *= **b;
}

void call_swap_3()
{
	int a = 1, b = 2, c = 3;
	swap_3(&a, &b, &c);
	#pragma castor assert a == 2 /\ b == 3 /\ c == 1
}

#pragma castor ensures result == 5
int call_add_1()
{
	int a = 2, b = 3;
	int *a_ptr = &a, *b_ptr = &b;

	add_1(&a_ptr, &b_ptr);
	#pragma castor assert is_integral(a) /\ is_pointer(a_ptr)
	return a;
}

void quantifier_test()
{
	#pragma castor assert !(exists uint8: i. i < 0)
	//#pragma castor assert forall uint8 i. exists sint8 j. j < i
	#pragma castor assert forall sint64: i. to_sint64(i) == i
	#pragma castor assert forall sint32: i. i >= 0 => is_uint32(i)
	#pragma castor assert exists uint8: x, uint8: y. x == y
}

void assume()
{
	int x;
	#pragma castor assume x == 5
	#pragma castor assert checked(x + 1) == 6
}

void array_test()
{
	int arr[10];
	arr[1] = 42;

	#pragma castor assert &arr + 1 == arr + 1
	#pragma castor assert *(arr + 1) == 42
	#pragma castor assert *(&arr + 1) == 42
	#pragma castor assert *arr == 0
}

#pragma castor requires valid_array(ptr, 2)
void pointer_test(int * const ptr)
{
	ptr[0] = 5;
	ptr[1] = 6;

	#pragma castor assert *ptr == 5
	#pragma castor assert *(ptr + 1) == 6
	#pragma castor assert ptr[1] == 6
}

struct sizeof_test_dummy
{
	sizeof_test_dummy() {}
};

void sizeof_test()
{
	int dummy;
	#pragma castor assert sizeof(dummy) == 4

	sizeof_test_dummy dummy2 = sizeof_test_dummy();
	#pragma castor assert sizeof(dummy2) >= 0
}
