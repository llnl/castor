// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Iterator
{
private:
	int* ctr;

public:
	Iterator(int* ctr) : ctr(ctr) { }

	Iterator& operator++()
	{
		++(this->ctr);
		return *this;
	}

	bool operator!=(const Iterator& it)
	{
		return it.ctr != this->ctr;
	}

	int operator*()
	{
		return *ctr;
	}
};

class Container
{
private:
	int data[10];

public:
	Container() { }

	Iterator begin()
	{
		return Iterator(data);
	}

	Iterator end()
	{
		return Iterator(data + 10);
	}
};

void foo()
{
	Container container;

	for (const auto& it : container) { }
}
