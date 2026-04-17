// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Int2
{
private:
	int x;
	int y;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->x == a
	#pragma castor ensures this->y == b
	#pragma castor writes this->x, this->y
	Int2(int a, int b)
	{
		this->x = a;
		this->y = b;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures result == checked(this->x * second.x + this->y * second.y)
	#pragma castor no_write
	int operator*(Int2 second)
	{
		return this->x * second.x + this->y * second.y;
	}
};

void tester()
{
	Int2 a(1, 2);
	#pragma castor assert a.x == 1 /\ a.y == 2
	Int2 b(4, 3);
	#pragma castor assert b.x == 4 /\ b.y == 3
	auto prod = a * b;
	#pragma castor assert prod == 10
}
