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
	#pragma castor no_free
	Point() { }
};

struct PointPair
{
	struct Point p1;
	struct Point p2;

	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor no_free
	PointPair() { }
};

#pragma castor invariant valid(mid)
#pragma castor requires is_sint32(p1.x + p2.x)
#pragma castor requires is_sint32(p1.y + p2.y)
#pragma castor ensures mid->x == (p1.x + p2.x) / 2
#pragma castor ensures mid->y == (p1.y + p2.y) / 2
#pragma castor ensures false
#pragma castor writes mid->x, mid->y
#pragma castor no_free
void midpoint(struct Point p1, struct Point p2, struct Point *mid)
{
	(*mid).x = (p1.x + p2.x) / 2;
	(*mid).y = (p1.y + p2.y) / 2;
}

