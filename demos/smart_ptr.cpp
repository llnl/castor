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
	#pragma castor ensures alias_of(result, *[this]->val)
	#pragma castor no_write
	#pragma castor no_free
	T& get()
	{
		return *val;
	}

	#pragma castor invariant valid(this)
	#pragma castor requires valid([this]->val, [this]->count)
	#pragma castor requires separated(this, [this]->val, [this]->count)
	#pragma castor requires *[this]->count > 0
	#pragma castor ensures old(*[this]->count) == 1 -> freed([this]->val, [this]->count)
	#pragma castor ensures old(*[this]->count)  > 1 -> valid([this]->val, [this]->count) /\ separated(this, [this]->val, [this]->count)
	#pragma castor ensures unchanged(this) /\ unchanged([this]->count) /\ unchanged([this]->val)
	#pragma castor ensures !freed([this]->count) -> *[this]->count == old(*[this]->count) - 1
	#pragma castor ensures !freed([this]->val) -> unchanged(*[this]->val)
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

template shared_ptr<int>::shared_ptr(int* initial_value);
template shared_ptr<int>::shared_ptr(shared_ptr<int>& copy);
template int& shared_ptr<int>::get();
template shared_ptr<int>::~shared_ptr();

