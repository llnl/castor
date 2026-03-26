// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

template <typename T>
struct Pair
{
	T first;
	T second;

	#pragma castor invariant valid(this)
	#pragma castor ensures this->first == first
	#pragma castor ensures this->second == second
	#pragma castor writes this->first, this->second
	#pragma castor no_free
	Pair(T first, T second)
	{
		this->first = first;
		this->second = second;
	}
};

#pragma castor requires pair.first == 1 /\ pair.second == 2
void consume(Pair<int> pair) { }

void produce()
{
	consume(Pair<int>(1, 2));
}

#define SIZE 10

void dynamic_alloc()
{
	auto pair = new Pair<int>(3, 4);
	#pragma castor assert valid(pair) /\ valid_array(pair, 1)
	auto myint = new int(3);
	#pragma castor assert *myint == 3 /\ valid(myint) /\ valid_array(myint, 1)
	auto default_int = new int();
	#pragma castor assert *default_int == 0 /\ valid(default_int) /\ valid_array(default_int, 1)
	auto mybool = new bool(true);
	#pragma castor assert *mybool == true /\ valid(mybool) /\ valid_array(mybool, 1)
}

#pragma castor requires valid(pair)
#pragma castor ensures !valid(pair)
void delete_pair(Pair<unsigned char>* pair)
{
	delete pair;
	#pragma castor assert freed(pair)
}

#pragma castor ensures valid(result)
#pragma castor ensures *result == 42
int* return_new_int()
{
	return new int(42);
}
