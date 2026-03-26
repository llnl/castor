// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Foo {
	int field;
public:
	#pragma castor invariant valid(this)
	#pragma castor ensures field == param
	Foo(int param) : field(param) { }
};
