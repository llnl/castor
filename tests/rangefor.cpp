// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

void foo()
{
	int arr[10];
	int sum = 0;

	#pragma castor writes __begin, sum
	for (const auto& i : arr)
		#pragma castor writes __begin, sum
		for (const auto& j : arr)
			sum += i + j;
}

class Container
{
private:
	int arr[10];

public:
	#pragma castor invariant valid(this)
	#pragma castor no_write
	Container() { }

	#pragma castor no_write
	#pragma castor invariant valid(this)
	#pragma castor ensures result == this->arr
	int* begin()
	{
		return arr;
	}

	#pragma castor no_write
	#pragma castor invariant valid(this)
	#pragma castor ensures result == this->arr + 10
	int* end()
	{
		return arr + 10;
	}
};

void bar()
{
	Container container;

	for (const auto i : container) ;
}
