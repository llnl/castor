// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Foo
{
private:
	int x;

public:
	#pragma castor invariant valid(this)
	#pragma castor writes this->x
	#pragma castor ensures this->x == a
	Foo(int a)
	{
		x = a;
	}

	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor ensures result == this->x
	int get_x()
	{
		return x;
	}
};

void tester()
{
	auto x = Foo(5).get_x();
	#pragma castor assert x == 5
}
