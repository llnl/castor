// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct SomeStruct
{
	int a;
	int b;
};

void sizeof_test()
{
	auto t1 = sizeof(char);
	#pragma castor assert t1 == 1
	auto t2 = sizeof(unsigned long int);
	#pragma castor assert t2 == 8
	auto t3 = sizeof(SomeStruct);
	#pragma castor assert t3 >= 0
	auto t4 = sizeof(2 + 2);
	#pragma castor assert t4 == 4
	auto t5 = sizeof(1L);
	#pragma castor assert t5 == 8
	auto t6 = sizeof(1LL);
	#pragma castor assert t6 == 8
}
