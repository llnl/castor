// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "ir.hxx"
#include <utility>

#ifndef FUNC_TABLE
#define FUNC_TABLE

using namespace IR;

///
/// @brief Global function table
///
class FunctionTable
{
private:
	///
	/// @brief The table of functions. The table maps a function name to
	/// a pair. The first element represents whether or not the function
	/// returns a reference, and the second element is a vector representing
	/// whether any of its parameters are reference parameters.
	///
	std::map<std::string, std::pair<bool, std::vector<bool>>> table;

	///
	/// @brief This maps function names to function definitions.
	///
	std::map<std::string, std::shared_ptr<IRFunction>> mapper;

	///
	/// @brief Gets a reference to a function's parameter boolean
	///
	/// @param name Function name
	/// @param idx Parameter index
	/// @return Reference to the parameter in the table
	///
	std::vector<bool>::reference index(std::string name, int idx);

public:
	///
	/// @brief Constructor
	///
	FunctionTable();

	///
	/// @brief Register a function in the function table.
	///
	/// @param name Function name
	/// @param refs Whether or not a parameter is a reference
	/// @param is_ref Whether or not the function returns a reference
	/// @param func Pointer to the function definition
	///
	void register_function(std::string name, std::vector<bool> refs, bool is_ref, std::shared_ptr<IRFunction> func);

	///
	/// @brief Gets a function definition based on its name.
	///
	/// @param name Function name
	/// @return A pointer to a function definition
	///
	std::shared_ptr<IRFunction> get_function(std::string name);

	///
	/// @brief Sets a function parameter as a reference.
	///
	/// @param name Function name
	/// @param idx Parameter index
	///
	void set_ref(std::string name, int idx);

	///
	/// @brief Gets whether or not a parameter is a reference
	/// 
	/// @param name Function name
	/// @param idx Parameter index
	/// @return Whether or not the parameter is a reference
	///
	bool get_ref(std::string name, int idx);

	///
	/// @brief Gets whether or not a function returns a reference
	///
	/// @param name Function name
	/// @return Whether or not the function returns a reference
	///
	bool get_ref(std::string name);

	///
	/// @brief Gets whether or not a function has been declared without a definition
	///
	/// @param name Function name
	/// @return True if the function doesn't have a definition, false otherwise
	///
	bool declared_without_definition(std::string name);
};

#endif
