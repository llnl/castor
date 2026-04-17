// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == a + b
int add(int a, int b)
{
	return a + b;
}

class Rectangle {
	unsigned int side1_length;
	unsigned int side2_length;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->side1_length == side1
	#pragma castor ensures this->side2_length == side2
	#pragma castor writes this->side1_length, this->side2_length
	Rectangle(unsigned int side1, unsigned int side2)
		: side1_length(side1), side2_length(side2) { }

	#pragma castor invariant valid(this)
	#pragma castor ensures result == 2 * this->side1_length + 2 * this->side2_length
	#pragma castor no_write
	unsigned long get_perimeter() {
		return 2 * side1_length + 2 * side2_length;
	}
};

class Square : public Rectangle {
public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->side1_length == side
	#pragma castor ensures this->side2_length == side
	#pragma castor writes this->side1_length, this->side2_length
	Square(unsigned int side) : Rectangle(side, side) { }

	#pragma castor invariant valid(this)
	#pragma castor invariant this->side1_length == this->side2_length
	#pragma castor ensures exists int: x. x * 4 == result
	#pragma castor no_write
	unsigned long get_perimeter() {
		return this->Rectangle::get_perimeter();
	}
};

struct Point
{
	int x;
	int y;

	#pragma castor invariant valid(this)
	#pragma castor no_write
	Point() { }
};

struct PointPair
{
	struct Point p1;
	struct Point p2;

	#pragma castor invariant valid(this)
	#pragma castor no_write
	PointPair() { }
};

#pragma castor invariant valid(mid)
#pragma castor ensures mid->x == (p1.x + p2.x) / 2
#pragma castor ensures mid->y == (p1.y + p2.y) / 2
#pragma castor writes mid->x, mid->y
void midpoint(struct Point p1, struct Point p2, struct Point *mid)
{
	(*mid).x = (p1.x + p2.x) / 2;
	(*mid).y = (p1.y + p2.y) / 2;
}

#pragma castor ensures result == 55
int for_loop()
{
	int running_total = 0;

	#pragma castor variant 10 - i
	#pragma castor invariant running_total == (i * (i - 1)) / 2
	#pragma castor invariant 1 <= i /\ i <= 11
	#pragma castor writes i, running_total
	for (int i = 1; i <= 10; i += 1)
	{
		running_total += i;
	}

	return running_total;
}

