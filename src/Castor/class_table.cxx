// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "class_table.hxx"
#include <iostream>

///
/// @brief Default constructor
///
ClassTable::ClassTable() = default;

///
/// @brief Gets all the classes registered in the class table
///
/// @return A vector of pointers to IRClass objects
///
std::vector<std::shared_ptr<IRClass>> ClassTable::get_classes()
{
	std::vector<std::shared_ptr<IRClass>> classes;

	for (auto entry : this->table)
		classes.push_back(entry.second);

	return classes;
}

///
/// @brief Represents an index into table
///
/// @param name The class name
/// @return A reference to the pointer-to-class object
///
std::shared_ptr<IRClass>& ClassTable::operator[](IRName name)
{
	return this->table[name];
}

///
/// @brief Sets a parent class
///
/// @param base The base class name
/// @param parent The parent class name
///
void ClassTable::set_parent(IRName base, IRName parent)
{
	this->inheritance_hierarchy[base] = parent;
}

///
/// @brief Gets all the variables belonging to a class, including those inherited
///
/// @param name The class name
/// @return A vector of pointers to variable declarations
///
std::vector<std::shared_ptr<IRVariableDeclarationStmt>> ClassTable::get_variables(IRName name)
{
	// We don't need to concatenate all of the member variables up the inheritance hierarchy.
	// This is because when a class is registered with the symbol table, it already contains
	// all of its inherited variables as a normalization step. This is done in IRGenerator.
	return table[name]->get_vars();
}

///
/// @brief Gets only the variables belonging to a class that are inherited
///
/// @param name The class name
/// @return A vector of pointers to variable declarations
///
std::vector<std::shared_ptr<IRVariableDeclarationStmt>> ClassTable::get_inherited_variables(IRName name)
{
	std::vector<std::shared_ptr<IRVariableDeclarationStmt>> variables;

	if (this->inheritance_hierarchy.count(name))
		variables = get_variables(inheritance_hierarchy[name]);

	return variables;
}

///
/// @brief Checks if a class derives from another class
///
/// @param derived The derived class
/// @param base The base class
/// @return True if derived eventually inherits from base, false otherwise
///
bool ClassTable::inherits_from(IRName derived, IRName base)
{
	if (this->inheritance_hierarchy.count(derived))
	{
		auto parent = this->inheritance_hierarchy[derived];

		if (parent == base)
			return true;
		else
			return inherits_from(parent, base);
	}
	else
	{
		return false;
	}
}
