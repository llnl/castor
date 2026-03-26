// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires is_sint16(a) /\ is_sint16(b)
#pragma castor ensures result == a + b
int add(int a, int b)
{
	return a + b;
}

#pragma castor requires 0 <= a
#pragma castor ensures result == a - b
int sub(int a, unsigned short b)
{
	return a - b;
}

#pragma castor requires is_sint16(a) /\ is_sint16(b)
#pragma castor ensures result == a * b
int mul(int a, int b)
{
	return a * b;
}

#pragma castor requires b > 0
#pragma castor ensures result == a / b
int div(int a, int b)
{
	return a / b;
}

#pragma castor requires b > 0
#pragma castor ensures result == a % b
int mod(int a, int b)
{
	return a % b;
}

#pragma castor ensures result == to_sint32(a) << b
int lsl(unsigned char a, int b)
{
	return a << b;
}

#pragma castor ensures result == to_sint32(a >> b)
int lsr(unsigned int a, unsigned int b)
{
	return a >> b;
}

#pragma castor ensures result == a >> b
int asr(int a, int b)
{
	return a >> b;
}

#pragma castor ensures result == to_sint16(a >> b)
short sr_mystery(int a, unsigned char b)
{
	return a >> b;
}

#pragma castor ensures result == to_sint32(a) >> 2
int sr_mystery_2(unsigned short a)
{
	return a >> (short)2;
}

#pragma castor ensures is_sint64(result)
#pragma castor ensures result == a & b
long long land(long long a, long long b)
{
	return a & b;
}

#pragma castor ensures is_sint32(result)
#pragma castor ensures result == a | b
int lor(int a, int b)
{
	return a | b;
}

#pragma castor ensures is_sint32(result)
#pragma castor ensures result == a ^ b
int lxor(int a, int b)
{
	return a ^ b;
}

#pragma castor ensures (a & (1 << b)) != 0 <-> result != 0
int bit_test(int a, int b)
{
	return (a & (1 << b));
}

#pragma castor ensures result == true <-> a == b
bool equals(int a, int b)
{
	return a == b;
}

#pragma castor ensures result == true <-> a != b
bool not_equals(unsigned short a, unsigned short b)
{
	return a != b;
}

#pragma castor ensures result == x
int comma(int x)
{
	int foo = 0;
	auto return_val = ((foo = 5), 42);
	#pragma castor assert foo == 5
	#pragma castor assert return_val == 42
	(foo = 6, return_val) = x;
	#pragma castor assert foo == 6
	return return_val;
}

#pragma castor requires valid(a)
#pragma castor requires -max_sint32 / 2 < *a /\ *a < max_sint32 / 2
#pragma castor requires -max_sint32 / 2 < b /\ b < max_sint32 / 2
#pragma castor ensures *a == old(*a) + b
void add_assign(int *a, int b)
{
	*a += b;
}

#pragma castor requires valid(a)
#pragma castor requires -max_sint32 / 2 < *a /\ *a < max_sint32 / 2
#pragma castor requires -max_sint32 / 2 < b /\ b < max_sint32 / 2
#pragma castor ensures *a == old(*a) - b
void sub_assign(int *a, int b)
{
	*a -= b;
}

#pragma castor requires valid(a)
#pragma castor requires is_sint16(*a) /\ is_sint16(b)
#pragma castor ensures *a == old(*a) * b
void mult_assign(int *a, int b)
{
	*a *= b;
}

#pragma castor requires valid(a)
#pragma castor requires is_sint32(*a)
#pragma castor requires b > 0
#pragma castor ensures *a == old(*a) / b
void div_assign(int *a, int b)
{
	*a /= b;
}

#pragma castor requires valid(a)
#pragma castor requires is_sint32(*a)
#pragma castor requires b > 0
#pragma castor ensures *a == old(*a) % b
void mod_assign(int *a, int b)
{
	*a %= b;
}

#pragma castor requires valid(a)
#pragma castor ensures *a == old(*a) << b
void lsl_assign(short *a, short b)
{
	*a <<= b;
}

#pragma castor requires valid(a)
#pragma castor ensures *a == old(*a) >> b
void lsr_assign(unsigned short *a, short b)
{
	*a >>= b;
}

#pragma castor requires valid(a)
#pragma castor ensures *a == old(*a) & b
void and_assign(long long *a, int b)
{
	*a &= b;
}

#pragma castor requires valid(a)
#pragma castor ensures *a == old(*a) | b
void or_assign(long long *a, int b)
{
	*a |= b;
}

#pragma castor requires valid(a)
#pragma castor ensures *a == old(*a) ^ b
void xor_assign(int *a, int b)
{
	*a ^= b;
}

int ctr = 0;

#pragma castor requires ctr < max_sint32
#pragma castor ensures alias_of(result, x)
#pragma castor ensures ctr == old(ctr) + 1
#pragma castor writes ctr
int& lhs_evaluated_once_helper(int& x) { ++ctr; return x; }

#pragma castor requires is_sint16(a) /\ is_sint16(b)
#pragma castor ensures a == old(a) + b
void lhs_evaluated_once(int& a, int b)
{
	ctr = 0;
	lhs_evaluated_once_helper(a) += b;
	#pragma castor assert ctr == 1
}

void bit_neg()
{
	int x = 5;
	int y = ~x;
	#pragma castor assert y == -6
	unsigned int x_2 = 5;
	unsigned int y_2 = ~x_2;
	#pragma castor assert y_2 == 4294967290
}

void ptrdiff()
{
	int a[10];
	int* x = &a[7];
	int* y = &a[3];
	auto z = x - y;
	#pragma castor assert z == 4
}
