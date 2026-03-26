// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <map>
#include <string>
#include <utility>
#include <memory>
#include <vector>
#include "whyname.cxx"

#ifndef OFFSET_TABLE
#define OFFSET_TABLE

namespace Why3
{
	class WhyType;
}

///
/// @brief This contains the offsets for a class's fields in the backend.
///
/// To summarize, the WhyML needs to know the offset from the base address in order to fetch the memory for each field.
///
class OffsetTable
{
private:
	std::map<WhyName, std::pair<int, std::shared_ptr<Why3::WhyType>>> table; ///< Table which maps field names to offsets+type
	int size; ///< The size of this data structure

public:
	///
	/// @brief Constructor
	///
	OffsetTable();

	///
	/// @brief Gets the offset and type of a field given its name.
	///
	/// @param name The name of the field
	/// @return A reference to its offset and type
	///
	std::pair<int, std::shared_ptr<Why3::WhyType>>& operator[](WhyName name);

	///
	/// @brief Sets the size of the data structure
	///
	/// @param size The new size
	///
	void set_size(int size);

	///
	/// @brief Gets the size of the data structure
	///
	/// @return The data structure's size
	///
	int get_size();

	///
	/// @brief Gets a vector of all this data structure's members
	///
	/// @return A vector of all this data structure's members
	///
	std::vector<std::pair<int, std::shared_ptr<Why3::WhyType>>> get_members();
};

#endif
