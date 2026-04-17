// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

int global;

class Foo
{
public:
	#pragma castor invariant valid(this)
	#pragma castor writes global
	#pragma castor ensures global == 5
	Foo()
	{
		global = 5;
	}

	#pragma castor invariant valid(this)
	#pragma castor writes global
	#pragma castor ensures global == 7
	~Foo()
	{
		global = 7;
	}
};

void tester()
{
	{
		Foo f;

		#pragma castor assert global == 5
	}

	#pragma castor assert global == 7
}
