// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

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

#pragma castor requires is_sint32(p1.x + p2.x)
#pragma castor ensures result == (p1.x + p2.x) / 2
#pragma castor no_write
int midpoint_x(struct Point p1, struct Point p2)
{
	return (p1.x + p2.x) / 2;
}

#pragma castor requires is_sint32(p1.y + p2.y)
#pragma castor ensures result == (p1.y + p2.y) / 2
#pragma castor no_write
int midpoint_y(struct Point p1, struct Point p2)
{
	return (p1.y + p2.y) / 2;
}

#pragma castor ensures p.y == 1
#pragma castor writes p.y
void ptr_set(struct Point p)
{
	int *y_ptr = &(p.y);
	*y_ptr = 1;
}

#pragma castor invariant valid(mid)
#pragma castor requires is_sint32(p1.x + p2.x)
#pragma castor requires is_sint32(p1.y + p2.y)
#pragma castor ensures mid->x == (p1.x + p2.x) / 2
#pragma castor ensures mid->y == (p1.y + p2.y) / 2
#pragma castor writes mid->x, mid->y
void midpoint(struct Point p1, struct Point p2, struct Point *mid)
{
	(*mid).x = (p1.x + p2.x) / 2;
	(*mid).y = (p1.y + p2.y) / 2;
}

void return_struct()
{
	struct Point p1 = Point();
	p1.x = 4;
	p1.y = 5;
	#pragma castor assert p1.x == 4 /\ p1.y == 5

	struct Point p2 = Point();
	p2.x = 8;
	p2.y = 7;
	#pragma castor assert p1.x == 4 /\ p1.y == 5
	#pragma castor assert p2.x == 8 /\ p2.y == 7
label:

	struct Point mid = Point();
	#pragma castor assert p1.x == 4 /\ p1.y == 5
	#pragma castor assert p2.x == 8 /\ p2.y == 7
	midpoint(p1, p2, &mid);

	#pragma castor assert p1.x == at(p1.x, @label) /\ p1.y == at(p1.y, @label)
	#pragma castor assert p2.x == at(p2.x, @label) /\ p2.y == at(p2.y, @label)
	#pragma castor assert mid.x == (p1.x + p2.x) / 2
	#pragma castor assert mid.y == (p1.y + p2.y) / 2
	#pragma castor assert mid.x == 6 /\ mid.y == 6
}

#pragma castor ensures result == 10
int construct_pointpair()
{
	struct PointPair pair = PointPair();
	pair.p1.x = 1;
	pair.p1.y = 2;
	pair.p2.x = 3;
	pair.p2.y = 4;
	#pragma castor assert pair.p1.x == 1
	#pragma castor assert pair.p1.y == 2
	#pragma castor assert pair.p2.x == 3
	#pragma castor assert pair.p2.y == 4

	return pair.p1.x + pair.p1.y + pair.p2.x + pair.p2.y;
}
