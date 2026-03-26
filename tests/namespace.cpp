// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

namespace Foo
{
	int global;

	#pragma castor ensures x == checked(old(x) + 1)
	#pragma castor writes x
	#pragma castor no_free
	void add(int& x)
	{
		x += 1;
	}

	namespace Bar
	{
		int global;

		#pragma castor ensures x == checked(old(x) - 1)
		#pragma castor writes x
		#pragma castor no_free
		void sub(int& x)
		{
			x -= 1;
		}
	}
}

void caller()
{
	int x = 1;
	Foo::add(x);
	#pragma castor assert x == 2

	Foo::global = 1;
	Foo::Bar::global = 2;
	int first = Foo::global, second = Foo::Bar::global;
	#pragma castor assert first == 1 /\ second == 2
}

using namespace Foo;

namespace Baz
{
	#pragma castor ensures unchanged(x)
	#pragma castor no_write
	void unchanged(int& x) { }
}

void caller2()
{
	int x = 1;
	Bar::sub(x);
	#pragma castor assert x == 0
old_x:
	Baz::unchanged(x);
	#pragma castor assert x == at(x, @old_x)
}

namespace
{
	void is_externally_hidden() { }
}
