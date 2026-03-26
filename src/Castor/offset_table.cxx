// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "offset_table.hxx"
#include "whyml.hxx"
#include <algorithm>

///
/// @brief Constructor
///
OffsetTable::OffsetTable()
{
	this->size = 0;
}

///
/// @brief Gets the offset and type of a field given its name.
///
/// @param name The name of the field
/// @return A reference to its offset and type
///
std::pair<int, std::shared_ptr<Why3::WhyType>>& OffsetTable::operator[](WhyName name)
{
	return this->table[name];
}

///
/// @brief Sets the size of the data structure
///
/// @param size The new size
///
void OffsetTable::set_size(int size)
{
	this->size = size;
}

///
/// @brief Gets the size of the data structure
///
/// @return The data structure's size
///
int OffsetTable::get_size()
{
	return this->size;
}

///
/// @brief Gets a vector of all this data structure's members
///
/// @return A vector of all this data structure's members
///
std::vector<std::pair<int, std::shared_ptr<Why3::WhyType>>> OffsetTable::get_members()
{
	std::vector<std::pair<WhyName, std::pair<int, std::shared_ptr<Why3::WhyType>>>> raw_vec(this->table.begin(), this->table.end());
	std::vector<std::pair<int, std::shared_ptr<Why3::WhyType>>> ret(raw_vec.size());

	std::transform(raw_vec.begin(), raw_vec.end(), ret.begin(), [](auto arg) { return arg.second; });

	return ret;
}
