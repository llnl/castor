// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <map>
#include <memory>
#include <vector>
#include "ir.hxx"

using namespace IR;

///
/// @brief This represents the class table, which contains a map
/// from class names to class objects, as well as a map representing
/// the inheritance hierarchy.
///
/// Currently, only single inheritance is supported.
///
class ClassTable
{
private:
	std::map<IRName, std::shared_ptr<IRClass>> table; ///< Map from class names to pointers
	std::map<IRName, IRName> inheritance_hierarchy; ///< Represents the inheritance hierarchy, derived -> parent

public:
	///
	/// @brief Default constructor
	///
	ClassTable();

	///
	/// @brief Gets all the classes registered in the class table
	///
	/// @return A vector of pointers to IRClass objects
	///
	std::vector<std::shared_ptr<IRClass>> get_classes();

	///
	/// @brief Represents an index into table
	///
	/// @param name The class name
	/// @return A reference to the pointer-to-class object
	///
	std::shared_ptr<IRClass>& operator[](IRName name);

	///
	/// @brief Sets a parent class
	///
	/// @param base The base class name
	/// @param parent The parent class name
	///
	void set_parent(IRName base, IRName parent);

	///
	/// @brief Gets all the variables belonging to a class, including those inherited
	///
	/// @param name The class name
	/// @return A vector of pointers to variable declarations
	///
	std::vector<std::shared_ptr<IRVariableDeclarationStmt>> get_variables(IRName name);

	///
	/// @brief Gets only the variables belonging to a class that are inherited
	///
	/// @param name The class name
	/// @return A vector of pointers to variable declarations
	///
	std::vector<std::shared_ptr<IRVariableDeclarationStmt>> get_inherited_variables(IRName name);

	///
	/// @brief Checks if a class derives from another class
	///
	/// @param derived The derived class
	/// @param base The base class
	/// @return True if derived eventually inherits from base, false otherwise
	///
	bool inherits_from(IRName derived, IRName base);
};
