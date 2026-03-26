// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "symbol_table.hxx"
#include "exception.hxx"
#include <iostream>

///
/// @brief Constructor, requires a parent scope to exist (though you can pass in a null pointer)
///
/// @param parent_scope The parent scope
///
R2WML_Scope::R2WML_Scope(std::shared_ptr<R2WML_Scope> parent_scope)
{
	this->parent_scope = parent_scope;
}

std::map<SgScopeStatement*, std::shared_ptr<R2WML_Scope>> R2WML_Scope::scope_map;

///
/// @brief Looks up a variable in the symbol table, beginning from the symbol's declaring scope and working upwards
///
/// @param symbol The SAGE symbol
/// @return The found IRVariable object
///
std::shared_ptr<IRVariable> R2WML_Scope::lookup(SgSymbol* symbol)
{
	if (R2WML_Scope::scope_map.count(symbol->get_scope()))
	{
		auto proper_scope = R2WML_Scope::scope_map[symbol->get_scope()];
		return proper_scope->lookup(symbol->get_name());
	}
	else
	{
		return nullptr;
	}
}

///
/// @brief Registers a scope with the scope map
///
/// We don't put this in the constructor because we want to maintain our usage of shared pointers.
/// Doing it like this lets is map to a shared pointer to R2WML_Scope
///
/// @param scope Shared pointer to R2WML_Scope object
/// @param sage_scope The corresponding SAGE scope
///
void R2WML_Scope::register_sage_scope(std::shared_ptr<R2WML_Scope> scope, SgScopeStatement* sage_scope)
{
	R2WML_Scope::scope_map[sage_scope] = scope;
}

///
/// @brief Looks up a variable in the symbol table, beginning from the current scope and working upwards
///
/// @param name The variable name
/// @return The found IRVariable object
///
std::shared_ptr<IRVariable> R2WML_Scope::lookup(IRName name)
{
	std::shared_ptr<IRVariable> ret = nullptr;

	if (this->symbol_table.count(name)) // if the variable is found in the current scope
					    // we just return that
		ret = this->symbol_table[name];

	// otherwise, we have to traverse up the data structure
	if (ret || !parent_scope)
		return ret;
	else
		return this->parent_scope->lookup(name);
}

///
/// @brief Registers a variable with the symbol table
///
/// @param name The variable name
/// @param variable The IRVariable object
///
void R2WML_Scope::register_variable(IRName name, std::shared_ptr<IRVariable> variable)
{
	// When adding member variables to a derived class's symbol table,
	// the class's own member variables are added first, then the
	// inherited member variables. This guard ensures that any inherited
	// member variables with the same name as a re-defined member variable
	// are shadowed.
	if (!this->symbol_table.count(name))
		this->symbol_table[name] = variable;
}

///
/// @brief Gets a pointer to the parent scope
///
/// @return The parent scope
///
std::shared_ptr<R2WML_Scope> R2WML_Scope::get_parent_scope()
{
	return this->parent_scope;
}

///
/// @brief Sets the parent scope (you can pass in a null pointer)
///
/// @param parent_scope The parent scope
///
void R2WML_Scope::set_parent_scope(std::shared_ptr<R2WML_Scope> parent_scope)
{
	this->parent_scope = parent_scope;
}

///
/// @brief Gets a pointer to the parent scope
/// 
/// Because this function can potentially return the object
/// itself, it returns a naked pointer, not a shared pointer.
///
/// @return The parent scope
///
R2WML_Scope* R2WML_Scope::get_toplevel_scope()
{
	if (this->parent_scope)
		return this->parent_scope->get_toplevel_scope();
	else
		return this;
}

///
/// @brief Gets a unique suffix for a given variable name
///
/// @param name The variable name
/// @return An integer to append to the variable, giving it a unique name
///
int R2WML_Scope::get_unique_suffix(IRName name)
{
	// simply count how many variables with this name are visible
	if (this->parent_scope)
		return this->symbol_table.count(name) + this->parent_scope->get_unique_suffix(name);
	else
		return this->symbol_table.count(name);
}

///
/// @brief Gets a unique name for a variable name
///
/// @param name The variable name
/// @return A unique name for this variable
///
IRName R2WML_Scope::get_unique_name(IRName name)
{
	int suffix = this->get_unique_suffix(name);

	// only append the suffix if it's greater than 0
	if (suffix > 0)
		return name + "_" + std::to_string(suffix);
	else
		return name;
}

///
/// @brief Checks if a variable exists in the symbol table, and if so, what its unique identifier is
///
/// @param name The variable name
/// @param unique_name The variable's unique name, if it exists
/// @return Whether or not the variable was found
///
bool R2WML_Scope::find_unique_name(IRName name, IRName& unique_name)
{
	auto var = this->lookup(name);
	if (var)
	{
		unique_name = var->get_name();
		return true;
	}
	else
	{
		return false;
	}
}

///
/// @brief Gets the symbol table associated with this scope
///
/// @return The symbol table
///
std::map<IRName, std::shared_ptr<IRVariable>> R2WML_Scope::get_symbol_table()
{
	return this->symbol_table;
}

///
/// @brief Gets a list of variables that can be accessed from the current scope
///
/// @return All variables that can be accessed from the current scope
///
std::vector<std::shared_ptr<IRVariable>> R2WML_Scope::get_variable_list()
{
	std::vector<std::shared_ptr<IRVariable>> vars;
	
	for (auto pair : this->symbol_table) // add all the variables in this scope
		vars.push_back(pair.second);

	if (this->parent_scope) // and recursively add the parent scope's
	{
		auto parent_vars = this->parent_scope->get_variable_list();
		vars.insert(vars.end(), parent_vars.begin(), parent_vars.end());
	}

	return vars;
}

///
/// @brief Sets a variable as a class variable
///
/// @param name The variable name
///
void R2WML_Scope::set_class_variable(IRName name)
{
	this->class_variables.insert(lookup(name));
}

///
/// @brief Gets whether or not a variable is a class variable
///
/// @param name The variable name
/// @return Whether or not it's a class variable
///
bool R2WML_Scope::get_class_variable(IRName name)
{
	return get_class_variable(lookup(name));
}

///
/// @brief Gets whether or not a variable is a class variable
///
/// @param variable The variable pointer
/// @return Whether or not it's a class variable
///
bool R2WML_Scope::get_class_variable(std::shared_ptr<IRVariable> variable)
{
	// check if the variable is in our class variables list
	bool contained_here = this->class_variables.find(variable) != this->class_variables.end();

	if (this->parent_scope)
		// if we have a parent scope, check there if it's not contained here
		return contained_here || this->parent_scope->get_class_variable(variable);
	else
		return contained_here;
}
