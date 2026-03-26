// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct HasPair
{
	int a;
	int b;

	#pragma castor no_write
	#pragma castor no_free
	HasPair() { }
};

class HasMembers
{
private:
	HasPair pair;
	int a[2];
	int b;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->pair.a == 0
	#pragma castor ensures this->pair.b == 1
	#pragma castor ensures this->b      == 2
	#pragma castor ensures this->a[0]   == 3
	#pragma castor ensures this->a[1]   == 4
	#pragma castor no_free
	#pragma castor writes *this
	HasMembers()
	{
		pair.a = 0;
		pair.b = 1;
		b      = 2;
		a[0]   = 3;
		a[1]   = 4;
	}
};

class HasMembers2
{
private:
	HasPair a;
	HasPair b;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->a.a == 0
	#pragma castor ensures this->a.b == 1
	#pragma castor ensures this->b.a == 2
	#pragma castor ensures this->b.b == 3
	#pragma castor no_free
	#pragma castor writes *this
	HasMembers2()
	{
		a.a = 0;
		a.b = 1;
		b.a = 2;
		b.b = 3;
	}
};
