// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires is_sint16(a) /\ is_sint16(b)
#pragma castor ensures result == a + b
#pragma castor no_write
#pragma castor no_free
int add(int a, int b)
{
	return a + b;
}

#pragma castor requires is_sint16(a) /\ is_sint16(b)
#pragma castor ensures result == a + b
#pragma castor no_write
#pragma castor no_free
int add2(int a, int b)
{
	return add(a, b);
}

#pragma castor requires is_sint8(a) /\ is_sint8(b) /\ is_sint16(c)
#pragma castor ensures result == a + b + c
#pragma castor no_write
#pragma castor no_free
int add3(int a, int b, int c)
{
	return add(add(a, b), c);
}

void voidfunc()
{
	return;
}

void call_voidfunc()
{
	voidfunc();
	return;
}

#pragma castor requires x == 5
#pragma castor ensures result == 5
#pragma castor no_write
#pragma castor no_free
int requires_five(int x)
{
	return x;
}

#pragma castor ensures result == 5
int passes_five()
{
	return requires_five(5);
}
