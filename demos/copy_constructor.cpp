// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class DummyClass
{
private:
	int field1;
	long field2;

	char* ptr;

public:
	#pragma castor invariant valid(this)
	DummyClass() : field1(0), field2(0), ptr(nullptr) { }

	#pragma castor invariant valid(this)
	#pragma castor invariant separated(this, &copy)
	#pragma castor ensures *this == copy
	DummyClass(const DummyClass& copy) : field1(copy.field1), field2(copy.field2), ptr(copy.ptr) { }
};
