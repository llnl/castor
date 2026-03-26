// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class vector3
{
private:
	int x;
	int y;
	int z;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures [this]->x == x1
	#pragma castor ensures [this]->y == y1
	#pragma castor ensures [this]->z == z1
	#pragma castor writes *this
	vector3(int x1, int y1, int z1)
	{
		this->x = x1;
		this->y = y1;
		this->z = z1;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures [this]->x == checked(old([this]->x) + alpha)
	#pragma castor ensures [this]->y == checked(old([this]->y) + alpha)
	#pragma castor ensures [this]->z == checked(old([this]->z) + alpha)
	#pragma castor writes *this
	void operator +=(int alpha)
	{
		this->x += alpha;
		this->y += alpha;
		this->z += alpha;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures [this]->x == checked(old([this]->x) * alpha)
	#pragma castor ensures [this]->y == checked(old([this]->y) * alpha)
	#pragma castor ensures [this]->z == checked(old([this]->z) * alpha)
	#pragma castor writes *this
	void operator *=(int alpha)
	{
		this->x *= alpha;
		this->y *= alpha;
		this->z *= alpha;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures [this]->x == checked(old([this]->x) + [vec].x)
	#pragma castor ensures [this]->y == checked(old([this]->y) + [vec].y)
	#pragma castor ensures [this]->z == checked(old([this]->z) + [vec].z)
	#pragma castor writes *this
	void operator +=(vector3 vec)
	{
		this->x += vec.x;
		this->y += vec.y;
		this->z += vec.z;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures result == checked([this]->x * [vec].x + [this]->y * [vec].y + [this]->z * [vec].z)
	#pragma castor no_write
	int operator *(vector3 vec)
	{
		return this->x * vec.x + this->y * vec.y + this->z * vec.z;
	}

	#pragma castor invariant valid(this)
	#pragma castor requires 0 <= id /\ id <= 2
	#pragma castor ensures id == 0 => result == [this]->x
	#pragma castor ensures id == 1 => result == [this]->y
	#pragma castor ensures id == 2 => result == [this]->z
	#pragma castor no_write
	int get(int id)
	{
		if (id == 0)
			return this->x;
		else if (id == 1)
			return this->y;
		else
			return this->z;
	}
};

#pragma castor ensures result == 43
int main()
{
	vector3 vec1(1, 2, 3);
	vec1 += 1;
	vec1 *= 2;

	auto s1 = vec1.get(0);
	auto s2 = vec1.get(1);
	auto s3 = vec1.get(2);
	#pragma castor assert s1 == 4
	#pragma castor assert s2 == 6
	#pragma castor assert s3 == 8

	vector3 vec2(4, -3, 2);
	#pragma castor assert [vec1].x == 4 /\ [vec1].y ==  6 /\ [vec1].z == 8
	#pragma castor assert [vec2].x == 4 /\ [vec2].y == -3 /\ [vec2].z == 2

	vec1 += vec2;

	auto prod = vec1 * vec2;

	return prod;
}
