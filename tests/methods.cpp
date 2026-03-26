// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Point
{
private:
	int x;
	int y;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->x == x
	#pragma castor ensures this->y == y
	#pragma castor writes this->x, this->y
	#pragma castor no_free
	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	#pragma castor invariant valid(this)
	#pragma castor requires min_sint32 <= this->x + this->y /\ this->x + this->y <= max_sint32
	#pragma castor ensures result == this->x + this->y
	#pragma castor no_write
	#pragma castor no_free
	int manhattan_distance()
	{
		return x + y;
	}
};

template <typename T>
class Adder
{
private:
	T value;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->value == value
	#pragma castor writes this->value
	#pragma castor no_free
	Adder(T value)
	{
		this->value = value;
	}

	#pragma castor invariant valid(this)
	#pragma castor requires min_int(T) <= this->value + value /\ this->value + value <= max_int(T)
	#pragma castor ensures this->value == old(this->value) + value
	#pragma castor writes this->value
	#pragma castor no_free
	void add_to(T value)
	{
		this->value += value;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures result == this->value
	#pragma castor no_write
	#pragma castor no_free
	T get_val()
	{
		return value;
	}
};

#pragma castor ensures result == 9
int point_class_test()
{
	Point p1(4, 5);
	#pragma castor assert p1.x == 4 /\ p1.y == 5
	return p1.manhattan_distance();
}

#pragma castor ensures result == 5
int adder_class_test()
{
	Adder<int> x(2);
	x.add_to(3);

	return x.get_val();
}
