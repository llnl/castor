// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

template <typename T>
class shared_ptr
{
public:
	T* val;
	unsigned int* count;

public:
	#pragma castor invariant valid(this, initial_value)
	#pragma castor invariant separated(this, initial_value)
	#pragma castor ensures valid([this]->val, [this]->count)
	#pragma castor ensures separated(this, [this]->val, [this]->count)
	#pragma castor ensures *[this]->count == 1
	#pragma castor ensures [this]->val == initial_value
	#pragma castor ensures unchanged(*initial_value)
	#pragma castor writes [this]->val, [this]->count
	#pragma castor no_free
	shared_ptr(T* initial_value)
	{
		val = initial_value;
		count = new unsigned int(1);
	}

	#pragma castor requires valid(this, [copy].val, [copy].count) /\ separated(this, [copy].val, [copy].count)
	#pragma castor requires *[copy].count > 0 /\ is_uint32(*[copy].count + 1)
	#pragma castor ensures valid(this, [this]->count, [this]->val, [copy].val, [copy].count)
	#pragma castor ensures separated(this, [this]->val, [this]->count) /\ separated([copy].val, [copy].count)
	#pragma castor ensures *[copy].count == old(*[copy].count) + 1
	#pragma castor ensures [this]->count == old([copy].count) /\ [this]->val == old([copy].val)
	#pragma castor writes [this]->val, [this]->count, *[copy].count
	#pragma castor no_free
	shared_ptr(shared_ptr<T>& copy)
	{
		val = copy.val;
		count = copy.count;

		*count += 1;
	}

	#pragma castor invariant valid(this, [this]->val, [this]->count) /\ separated(this, [this]->val, [this]->count)
	#pragma castor requires *[this]->count > 0
	#pragma castor ensures result == *[this]->val
	#pragma castor no_write
	#pragma castor no_free
	T get()
	{
		return *val;
	}

	#pragma castor invariant valid(this, [this]->val, [this]->count) /\ separated(this, [this]->val, [this]->count)
	#pragma castor requires *[this]->count > 0
	#pragma castor ensures unchanged([this]->val) /\ unchanged(*[this]->count) /\ unchanged([this]->count)
	#pragma castor ensures *[this]->val == new_value
	#pragma castor writes *[this]->val
	#pragma castor no_free
	void set(T new_value)
	{
		*val = new_value;
	}

	#pragma castor invariant valid(this)
	#pragma castor requires valid([this]->val, [this]->count)
	#pragma castor requires separated(this, [this]->val, [this]->count)
	#pragma castor requires *[this]->count > 0
	#pragma castor ensures old(*[this]->count) == 1 => freed([this]->val, [this]->count)
	#pragma castor ensures old(*[this]->count)  > 1 => valid([this]->val, [this]->count) /\ separated(this, [this]->val, [this]->count)
	#pragma castor ensures unchanged(this) /\ unchanged([this]->count) /\ unchanged([this]->val)
	#pragma castor ensures !freed([this]->count) => *[this]->count == old(*[this]->count) - 1
	#pragma castor ensures !freed([this]->val) => unchanged(*[this]->val)
	#pragma castor writes *[this]->count
	#pragma castor frees *[this]->count, *[this]->val
	~shared_ptr()
	{
		*count -= 1;

		if (*count == 0)
		{
			delete val;
			delete count;
		}
	}
};

template class shared_ptr<int>;

#pragma castor invariant valid([input].val, [input].count) /\ separated([input].val, [input].count)
#pragma castor invariant *[input].count > 0 /\ is_uint32(*[input].count + 1)
#pragma castor ensures *[input].val == 6
#pragma castor ensures unchanged(*[input].count)
#pragma castor ensures unchanged([input].val) /\ unchanged([input].count)
#pragma castor writes *[input].val
#pragma castor no_free
void tester2(shared_ptr<int>& input)
{
	shared_ptr<int> int_ptr_2 = shared_ptr<int>(input);

	int_ptr_2.set(6);
	#pragma castor assert [input].val == [int_ptr_2].val
	#pragma castor assert *[input].val == 6
	#pragma castor assert *[int_ptr_2].count >= 2
}

#pragma castor requires valid(input)
#pragma castor requires *input == 5
#pragma castor ensures freed(input)
#pragma castor ensures !valid(input)
void tester(int* input)
{
	shared_ptr<int> int_ptr = shared_ptr<int>(input);

	tester2(int_ptr);

	auto ret = int_ptr.get();
	#pragma castor assert ret == 6
	#pragma castor assert input == [int_ptr].val
	#pragma castor assert *[int_ptr].count == 1
}

