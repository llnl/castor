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

	#pragma castor assert pair.p2.y != 4
	return pair.p1.x + pair.p1.y + pair.p2.x + pair.p2.y;
}
