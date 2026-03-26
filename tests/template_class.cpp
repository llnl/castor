// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

template <typename T, typename U>
class Tuple
{
private:
	T first;
	U second;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->first == first
	#pragma castor ensures this->second == second
	#pragma castor writes this->first, this->second
	#pragma castor no_free
	Tuple(T first, U second)
	{
		this->first = first;
		this->second = second;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures result == this->first
	#pragma castor no_write
	#pragma castor no_free
	T get_first()
	{
		return this->first;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures result == this->second
	#pragma castor no_write
	#pragma castor no_free
	U get_second()
	{
		return this->second;
	}

	#pragma castor invariant valid(this)
	#pragma castor requires min_int(T) <= this->first /\ this->first < max_int(T) - 1
	#pragma castor ensures this->first == old(this->first) + 1
	#pragma castor writes this->first
	#pragma castor no_free
	void add_one_first()
	{
		this->first += 1;
	}

	#pragma castor invariant valid(this)
	#pragma castor requires min_int(U) <= this->second /\ this->second < max_int(U) - 1
	#pragma castor ensures this->second == old(this->second) + 1
	#pragma castor writes this->second
	#pragma castor no_free
	void add_one_second()
	{
		this->second += 1;
	}
};

#pragma castor ensures result == 59
long long runner()
{
	Tuple<unsigned int, short> my_tuple(45, 12);

	my_tuple.add_one_first();
	my_tuple.add_one_second();

	return my_tuple.get_first() + my_tuple.get_second();
}
