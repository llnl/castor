// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef R2WML_SYMBOL
#define R2WML_SYMBOL

#include <map>
#include <vector>
#include <set>
#include <rose.h>
#include "ir.hxx"

using IR::IRVariable;
using namespace Rose;

///
/// @brief This represents the notion of a scope in the IR, containing all of the associated variables.
///
/// This also lets you generate unique names for variables, among other helper functions.
///
class R2WML_Scope
{
private:
	std::shared_ptr<R2WML_Scope> parent_scope;                  ///< Pointer to the parent (owning) scope
	std::map<IRName, std::shared_ptr<IRVariable>> symbol_table; ///< The symbol table, mapping variable names to variable objects
	std::set<std::shared_ptr<IRVariable>> class_variables;      ///< Keeps track of what variables are class variables
	static std::map<SgScopeStatement*, std::shared_ptr<R2WML_Scope>> scope_map; ///< Maps SAGE scopes to Castor scopes

public:
	///
	/// @brief Constructor, requires a parent scope to exist (though you can pass in a null pointer)
	///
	/// @param parent_scope The parent scope
	///
	R2WML_Scope(std::shared_ptr<R2WML_Scope> parent_scope);

	///
	/// @brief Sets the parent scope (you can pass in a null pointer)
	///
	/// @param parent_scope The parent scope
	///
	void set_parent_scope(std::shared_ptr<R2WML_Scope> parent_scope);

	///
	/// @brief Looks up a variable in the symbol table, beginning from the symbol's declaring scope and working upwards
	///
	/// @param symbol The SAGE symbol
	/// @return The found IRVariable object
	///
	static std::shared_ptr<IRVariable> lookup(SgSymbol* symbol);

	///
	/// @brief Registers a scope with the scope map
	///
	/// We don't put this in the constructor because we want to maintain our usage of shared pointers.
	/// Doing it like this lets is map to a shared pointer to R2WML_Scope
	///
	/// @param scope Shared pointer to R2WML_Scope object
	/// @param sage_scope The corresponding SAGE scope
	///
	static void register_sage_scope(std::shared_ptr<R2WML_Scope> scope, SgScopeStatement* sage_scope);

	///
	/// @brief Looks up a variable in the symbol table, beginning from the current scope and working upwards
	///
	/// @param name The variable name
	/// @return The found IRVariable object
	///
	std::shared_ptr<IRVariable> lookup(IRName name);

	///
	/// @brief Registers a variable with the symbol table
	///
	/// @param name The variable name
	/// @param variable The IRVariable object
	///
	void register_variable(IRName name, std::shared_ptr<IRVariable> variable);

	///
	/// @brief Gets a pointer to the parent scope
	///
	/// @return The parent scope
	///
	std::shared_ptr<R2WML_Scope> get_parent_scope();

	///
	/// @brief Gets a pointer to the scope at the topmost level
	///
	/// Because this function can potentially return the object
	/// itself, it returns a naked pointer, not a shared pointer.
	///
	/// @return The topmost level scope
	///
	R2WML_Scope* get_toplevel_scope();

	///
	/// @brief Checks if a variable exists in the symbol table, and if so, what its unique identifier is
	///
	/// @param name The variable name
	/// @param unique_name The variable's unique name, if it exists
	/// @return Whether or not the variable was found
	///
	bool find_unique_name(IRName name, IRName& unique_name);

	///
	/// @brief Gets a unique name for a variable name
	///
	/// @param name The variable name
	/// @return A unique name for this variable
	///
	IRName get_unique_name(IRName name);

	///
	/// @brief Gets a unique suffix for a given variable name
	///
	/// @param name The variable name
	/// @return An integer to append to the variable, giving it a unique name
	///
	int get_unique_suffix(IRName name);

	///
	/// @brief Gets the symbol table associated with this scope
	///
	/// @return The symbol table
	///
	std::map<IRName, std::shared_ptr<IRVariable>> get_symbol_table();

	///
	/// @brief Gets a list of variables that can be accessed from the current scope
	///
	/// @return All variables that can be accessed from the current scope
	///
	std::vector<std::shared_ptr<IRVariable>> get_variable_list();

	///
	/// @brief Sets a variable as a class variable
	///
	/// @param name The variable name
	///
	void set_class_variable(IRName name);

	///
	/// @brief Gets whether or not a variable is a class variable
	///
	/// @param name The variable name
	/// @return Whether or not it's a class variable
	///
	bool get_class_variable(IRName name);

	///
	/// @brief Gets whether or not a variable is a class variable
	///
	/// @param variable The variable pointer
	/// @return Whether or not it's a class variable
	///
	bool get_class_variable(std::shared_ptr<IRVariable> variable);
};

#endif
