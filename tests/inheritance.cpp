// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

int global;

class Animal
{
protected:
	int length;
	int shadows;

public:
	#pragma castor invariant valid(this)
	#pragma castor no_write
	#pragma castor no_free
	Animal() = default;

	#pragma castor invariant valid(this)
	#pragma castor ensures this->length == length
	#pragma castor writes this->length
	#pragma castor no_free
	Animal(int length) : length(length) { }

	#pragma castor invariant valid(this)
	#pragma castor ensures result == this->length
	#pragma castor no_write
	#pragma castor no_free
	int get_length()
	{
		return this->length;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures global == 1
	#pragma castor writes global
	#pragma castor no_free
	void do_something()
	{
		global = 1;
	}
};

class Mammal : public Animal
{
private:
	int legs;
	int* shadows;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->legs == legs
	#pragma castor ensures this->length == length
	#pragma castor ensures this->shadows == &this->legs
	#pragma castor ensures is_pointer(this->shadows)
	#pragma castor writes this->legs, this->length, this->shadows
	#pragma castor no_free
	Mammal(int legs, int length)
	{
		this->length = length;
		this->legs = legs;
		this->shadows = &(this->legs);
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures this->legs == 4
	#pragma castor ensures this->length == length
	#pragma castor ensures this->shadows == 0
	#pragma castor ensures is_pointer(this->shadows)
	#pragma castor writes this->legs, this->length, this->shadows
	#pragma castor no_free
	Mammal(int length) : Animal(length)
	{
		this->shadows = nullptr;
		this->legs = 4;
	}

	#pragma castor invariant valid(this)
	#pragma castor no_free
	#pragma castor no_write
	#pragma castor ensures result == this->legs
	int get_legs()
	{
		return this->legs;
	}

	#pragma castor invariant valid(this)
	#pragma castor ensures global == 2
	#pragma castor writes global
	#pragma castor no_free
	void do_something()
	{
		global = 2;
	}
};

struct Cat : public Mammal
{
	bool cute;

	#pragma castor invariant valid(this)
	#pragma castor ensures this->length == length
	#pragma castor ensures this->legs == legs
	#pragma castor ensures this->cute == cute
	#pragma castor no_free
	#pragma castor writes this->length, this->legs, this->cute, this->shadows
	Cat(int legs, int length, bool cute) : Mammal(legs, length)
	{
		this->cute = cute;
	}
};

int init1()
{
	Mammal dog(4, 10);
	auto length = dog.get_length();
	auto legs = dog.get_legs();
	#pragma castor assert length == 10
	#pragma castor assert legs == 4
}

int init2()
{
#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
	Animal* animal = new Mammal(6, 11);
#else
	Mammal animal_base(6, 11);
	Animal* animal = &animal_base;
#endif
	#pragma castor assert alias_of(animal_base, *animal)

	auto legs = ((Mammal*)animal)->get_legs();
	#pragma castor assert legs == 6

	animal->do_something();
	#pragma castor assert global == 1
	static_cast<Mammal*>(animal)->do_something();
	#pragma castor assert global == 2
}

int init3()
{
	Animal animal2 = Mammal(2, 6);
	#pragma castor assert animal2.length == 6

	Cat cat(4, 5, true);
	Animal* cat_animal = &cat;

	#pragma castor assert alias_of(cat, *cat_animal)

	Cat* cat_animal_downcasted = static_cast<Cat*>(cat_animal);

	#pragma castor assert cat_animal_downcasted->cute
}

class Base
{
public:
	#pragma castor invariant valid(this)
	#pragma castor no_write
	Base() = default;

	#pragma castor invariant valid(this)
	#pragma castor ensures result == 0
	#pragma castor no_write
	int get_id() { return 0; }
};

class Derived : public Base
{
public:
	#pragma castor invariant valid(this)
	#pragma castor no_write
	Derived() { }

	#pragma castor invariant valid(this)
	#pragma castor ensures result == 1
	#pragma castor no_write
	int get_id() { return 1; }
};

void ref_poly()
{
	Derived derived;
	Base& base = (Base&)derived;

	#pragma castor assert alias_of(base, derived)

	auto derived_id = derived.get_id();
	#pragma castor assert derived_id == 1

	auto base_id = base.get_id();
	#pragma castor assert base_id == 0
}
