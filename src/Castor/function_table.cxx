// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "function_table.hxx"
#include "exception.hxx"
#include "messaging.hxx"

///
/// @brief Constructor
///
FunctionTable::FunctionTable() = default;

///
/// @brief Register a function in the function table.
///
/// @param name Function name
/// @param refs Whether or not a parameter is a reference
/// @param is_ref Whether or not the function returns a reference
/// @param func Pointer to the function definition
///
void FunctionTable::register_function(std::string name, std::vector<bool> refs, bool is_ref, std::shared_ptr<IRFunction> func)
{
	table[name] = std::make_pair(is_ref, refs);

	if (!mapper.count(name) ||
			(mapper.count(name) && !mapper[name]->has_body() && func->has_body()))
		mapper[name] = func;
}

///
/// @brief Gets a function definition based on its name.
///
/// @param name Function name
/// @return A pointer to a function definition
///
std::shared_ptr<IRFunction> FunctionTable::get_function(std::string name)
{
	if (!mapper.count(name))
	{
		throw CastorException("Encountered unregistered function call \"" + name + "\"");
	}
	else
	{
		return mapper[name];
	}
}

///
/// @brief Gets whether or not a function has been declared without a definition
///
/// @param name Function name
/// @return True if the function doesn't have a definition, false otherwise
///
bool FunctionTable::declared_without_definition(std::string name)
{
	if (!mapper.count(name))
	{
		throw CastorException("Encountered unregistered function call \"" + name + "\"");
	}
	else
	{
		return !mapper[name]->has_body();
	}
}

///
/// @brief Sets a function parameter as a reference.
///
/// @param name Function name
/// @param idx Parameter index
///
void FunctionTable::set_ref(std::string name, int idx)
{
	index(name, idx) = true;
}

///
/// @brief Gets whether or not a parameter is a reference
/// 
/// @param name Function name
/// @param idx Parameter index
/// @return Whether or not the parameter is a reference
///
bool FunctionTable::get_ref(std::string name, int idx)
{
	return index(name, idx);
}

///
/// @brief Gets whether or not a function returns a reference
///
/// @param name Function name
/// @return Whether or not the function returns a reference
///
bool FunctionTable::get_ref(std::string name)
{
	return std::get<0>(table[name]);
}

///
/// @brief Gets a reference to a function's parameter boolean
///
/// @param name Function name
/// @param idx Parameter index
/// @return Reference to the parameter in the table
///
std::vector<bool>::reference FunctionTable::index(std::string name, int idx)
{
	if (!table.count(name)) // this rarely happens, but if it does, let's make sure we handle it
	{
		// this is the explanation of the only time i've seen this condition trigger
		log("Function \"" + name + "\" is not registered in the function table.", LogType::FATAL, 0);
		log("This might be a static, uninstantiated, template function.", LogType::FATAL, 0);
		throw CastorException("Encountered unregistered function call \"" + name + "\"");
	}

	auto& vec = std::get<1>(table[name]);

	if (vec.size() <= idx) // never seen this trigger, but we should throw something if it does
	{
		throw CastorException("Function " + name + " only has " + std::to_string(vec.size()) +
				" arguments. Tried to fetch argument " + std::to_string(idx));
	}

	return vec[idx];
}
