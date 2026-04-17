// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Foo
{
public:
	#pragma castor invariant valid(this)
	#pragma castor no_write
	Foo() = default;

private:
	int a;

	#pragma castor invariant valid(this)
	#pragma castor writes this->a
	#pragma castor ensures this->a == x
	Foo(int x)
	{
		a = x;
	}

	friend class Bar;
};

class Baz
{
private:
	int unused;

public:
	Baz();
};

class Bar
{
private:
	Foo a;

	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor ensures result == this->a.a
	int get_val()
	{
		return this->a.a;
	}

public:
	#pragma castor invariant valid(this)
	#pragma castor writes this->a
	#pragma castor ensures this->a.a == x
	Bar(int x)
	{
		a = Foo(x);
	}

	friend Baz::Baz();
};

#pragma castor invariant valid(this)
Baz::Baz()
{
	Bar bar(5);
	auto v = bar.get_val();

	#pragma castor assert v == 5
}
