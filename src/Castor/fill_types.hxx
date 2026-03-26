// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <iostream>
#include "irtraversal.hxx"
#include "symbol_table.hxx"

using namespace IR;

///
/// @brief The inherited values for the type filling traversal.
///
struct FillTypesIV
{
	bool in_vc;                                                    ///< Whether or not we're in a verification condition
	std::shared_ptr<R2WML_Scope> current_scope;                    ///< Pointer to the current enclosing scope
	std::vector<std::shared_ptr<IRVariable>> non_scoped_variables; ///< Variables that exist but aren't in a scope, such as quantifier variables
	std::vector<std::shared_ptr<IRClass>> classes;                 ///< A list of all classes visible from the current point in the traversal
	std::shared_ptr<IRType> function_return_type;                  ///< Return type of the current function
	bool in_checked;                                               ///< Whether or not we're in a checked() function
	bool ref_func;                                                 ///< Whether or not we're in a function that returns a reference
};

///
/// @brief This class defines the traversal over the IR and fills in types, metadata, and other miscellanea
///
class FillTypes : public IRTraversal<FillTypesIV, std::shared_ptr<IRNode>>
{
private:
	///
	/// @brief Takes a pointer to an IRType node, and if the type is an IRClass, finds the class definition and assigns it to the IRType node.
	/// 
	/// If IRType is a pointer or array type, we recurse into the type to see if an IRClass type is hiding somewhere.
	///
	/// @param type Pointer to the type object we want to augment
	/// @param classes Classes we can possibly assign
	///
	void assign_class_definition(std::shared_ptr<IRType> type, std::vector<std::shared_ptr<IRClass>> classes);

	///
	/// @brief Get the max int based on the type of the first parameter.
	///
	/// @param params List of parameters
	/// @return Literal representing the max integral value
	///
	std::shared_ptr<IRLiteral> parse_max_int(std::vector<std::shared_ptr<IRExpression>> params);

	///
	/// @brief Get the min int based on the type of the first parameter.
	///
	/// @param params List of parameters
	/// @return Literal representing the min integral value
	///
	std::shared_ptr<IRLiteral> parse_min_int(std::vector<std::shared_ptr<IRExpression>> params);

	/// 
	/// @brief Gets the resulting type of a bitwise operation, excluding shifts.
	///
	/// C++ type rules are NOT respected here. This is used ONLY in the verification langauge.
	///
	/// @param t1 First parameter in the bitwise operation
	/// @param t2 Second parameter in the bitwise operation
	/// @return Resulting type after the operation
	///
	std::shared_ptr<IRIntegralType> get_bitwise_type(std::shared_ptr<IRIntegralType> t1, std::shared_ptr<IRIntegralType> t2);

	///
	/// @brief Gets the resulting type of a bitwise shift operation
	///
	/// C++ type rules are NOT respected here. This is used ONLY in the verification langauge.
	///
	/// @param t1 First parameter in the bitshift
	/// @param t2 Second parameter in the bitshift
	/// @return Resulting type after the operation
	///
	std::shared_ptr<IRIntegralType> get_bitshift_type(std::shared_ptr<IRIntegralType> t1, std::shared_ptr<IRIntegralType> t2);

	///
	/// @brief Gets the resulting type of an integer operation
	///
	/// C++ type rules are NOT respected here. This is used ONLY in the verification langauge.
	///
	/// @param t1 First parameter in the operation
	/// @param t2 Second parameter in the operation
	/// @return Resulting type after the operation
	///
	std::shared_ptr<IRIntegralType> get_integral_type(std::shared_ptr<IRIntegralType> t1, std::shared_ptr<IRIntegralType> t2);

	///
	/// @brief Downcasts a type to an integral type, accounting for const
	///
	/// @param type The base type
	/// @return The type pointer, downcasted to an integral type
	///
	std::shared_ptr<IRIntegralType> make_integral_type(std::shared_ptr<IRType> type);

protected:
	using SynthesizedAttributesList = IRTraversal<FillTypesIV, std::shared_ptr<IRNode>>::SynthesizedAttributesList;

	///
	/// @brief Downwards traversal
	///
	/// @param astNode Input IRNode
	/// @param inheritedValue The inherited value
	/// @return The inheritedValue we should pass down
	///
	FillTypesIV evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, FillTypesIV inheritedValue);

	///
	/// @brief Upwards traversal
	///
	/// @param astNode Input IRNode
	/// @param inheritedValue The inherited value
	/// @param list List of synthesized attributes so far
	/// @return The new synthesized attribute
	///
	std::shared_ptr<IRNode> evaluateSynthesizedAttribute(std::shared_ptr<IRNode> astNode, FillTypesIV inheritedValue, SynthesizedAttributesList list);

public:
	///
	/// @brief Constructor
	///
	FillTypes() = default;

	///
	/// @brief Functor so that instances of this class may be used as a function
	///
	/// @param project The IR AST to traverse
	/// @result The resulting IR AST
	///
	std::shared_ptr<IRNode> operator()(std::shared_ptr<IRNode> project);


};
