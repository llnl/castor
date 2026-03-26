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
	#pragma castor ensures this->x == x
	#pragma castor writes this->x
	#pragma castor no_free
	Foo(int x) : x(x) { }

	#pragma castor invariant valid(this)
	#pragma castor ensures this->x == x
	#pragma castor writes this->x
	#pragma castor no_free
	void set_x(int x) const
	{
		const_cast<Foo*>(this)->x = x;
	}
};
