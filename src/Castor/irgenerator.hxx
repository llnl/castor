// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <rose.h>
#include <string>
#include <map>
#include <memory>
#include "ir.hxx"
#include "symbol_table.hxx"

using namespace IR;

///
/// @brief Enum to help identify the context of SgInitializedName
///
enum InValues
{
	FunctionParameterList, ///< SgInitializedName is a function parameter
	VariableDeclaration,   ///< SgInitializedName is a variable declaration
	Nothing                ///< No SgInitializedName encountered
};

///
/// @brief Inherited values for the SAGE AST traversal
///
struct IRGeneratorIV
{
	InValues status;                            ///< Status of SgInitializedName
	std::shared_ptr<R2WML_Scope> current_scope; ///< Current scope
	std::shared_ptr<IRType> new_type;           ///< Type being used in a `new` expression
	bool is_ref;                                ///< Whether or not the current function is returning a reference type
	bool is_constexpr;                          ///< Whether or not we're creating a constexpr variable
	bool class_var;                             ///< Whether or not we're declaring a class variable
};

using namespace Rose;
using namespace IR;

typedef typename SgTreeTraversal<IRGeneratorIV, std::shared_ptr<IRNode>>
	::SynthesizedAttributesList SynthesizedAttributesList;

///
/// @brief This is the main class that generates the IR from the SAGE AST
///
class IRGenerator : public AstTopDownBottomUpProcessing<IRGeneratorIV, std::shared_ptr<IRNode>>
{
public:
	///
	/// @brief Constructor
	///
	IRGenerator() = default;

	///
	/// @brief Functor, so that instances of this class can be used as a function
	///
	/// @param project SAGE AST
	/// @return IR AST
	///
	std::shared_ptr<IRNode> operator()(SgNode* project);

protected:
	///
	/// @brief This is a helper function to parse binary operators
	///
	/// @param astNode Binary operator to parse
	/// @param list Synthesized attributes so far
	/// @param scope The current scope
	/// @return The parsed binary operation
	///
	std::shared_ptr<IRNode> parseBinaryOp(SgNode *astNode, SynthesizedAttributesList list, std::shared_ptr<R2WML_Scope> scope);

	///
	/// @brief This is a helper function to parse literals
	///
	/// @param astNode Literal to parse
	/// @param list Synthesized attributes so far
	/// @return The parsed literal
	///
	std::shared_ptr<IRNode> parseLiteral(SgNode* astNode, SynthesizedAttributesList list);

	///
	/// @brief This function parses template arguments from instantiated template functions.
	///
	/// This function is most useful for cases where the template parameter is a non-typename object (e.g. `int`).
	/// In these cases, a variable is created with its constexpr set to the instantiated value.
	/// This lets us use this template variable as a normal variable in verification conditions.
	///
	/// @param ast_node The template function instantiation SAGE object
	/// @param scope The current scope
	/// @tparam T Must be either SgTemplateInstantiationFunctionDecl* or SgTemplateInstantiationMemberFunctionDecl*
	/// @return Vector of variables, initialized accordingly
	///
	template <typename T>
	std::vector<std::shared_ptr<IRVariable>> getTemplateArgs(T ast_node, std::shared_ptr<R2WML_Scope> scope);

	///
	/// @brief This function parses template arguments from uninstantiated template functions.
	///
	/// @param ast_node The template function SAGE object
	/// @tparam T Must be either SgTemplateFunctionDeclaration* or SgTemplateMemberFunctionDeclaration*
	/// @return Vector of variables, initialized accordingly
	///
	template <typename T>
	std::vector<std::shared_ptr<IRVariable>> getTemplateParams(T ast_node);

	///
	/// @brief Gets a good function name for the IR.
	///
	/// @param qualified_name The qualified name of the function from the SAGE tree
	/// @param params Function parameters
	/// @param mangled_name ROSE's mangled name
	/// @tparam T A shared pointer type to either IRVariable (for the function declaration) or IRExpression (callsite)
	/// @return Name to refer to this function in the IR
	///
	template <typename T>
	IRName get_func_name(IRName qualified_name, std::vector<T> params, IRName mangled_name);

	///
	/// @brief Attaches implementations to function prototypes.
	///
	/// The IR does not support "prototypes" (i.e. declarations without definitions).
	/// An IRFunction must contain its definition, so after traversing the SAGE AST, we call this function
	/// in order to consolidate this issue.
	///
	/// @param globals A reference to a vector of statements in the global namespace
	///
	void attach_impl(std::vector<std::shared_ptr<IRStatement>>& globals);

	///
	/// @brief Attaches verification conditions to functions.
	///
	/// This function attaches a verification condition to the next legal construct which can accept a verification condition.
	///
	/// @param item The verification condition to attach
	/// @param start_idx The place in list to start searching from
	/// @param list Vector of IRNode pointers that we might want to attach a VC to
	///
	void attach_ver(std::shared_ptr<IRVerificationCondition> item, int start_idx, std::vector<std::shared_ptr<IRNode>> list);

	///
	/// @brief Propagates verification conditions to template instantiations.
	///
	/// This function ensures that verification conditions are properly attached to functions when dealing with templates.
	/// In the source, we annotate template functions with verification conditions.
	/// ROSE will instantiate these functions and create new definitions for each unique instantiation.
	/// We want to make sure to attach the original verification conditions to each instantiation.
	///
	/// @param item The verification condition to attach
	/// @param name The name of the non-instantiated function
	/// @param list Vector of IRNode pointers that we might want to attach a VC to
	///
	void propagate_ver(std::shared_ptr<IRVerificationCondition> item, IRName name, std::vector<std::shared_ptr<IRStatement>> list);

	///
	/// @brief Cleans up the AST
	///
	/// @param globals Reference to vector of IRStatement pointers to tidy up.
	///
	void tidy_ast(std::vector<std::shared_ptr<IRStatement>>& globals);

	///
	/// @brief Attaches loop annotations to loops.
	///
	/// This function takes a vector of statements, and makes sure that invariants which need to be attached to loops
	/// (like invariants, variants, and so on) are properly attached to their loops and deleted from the vector.
	///
	/// @param statements Reference to vector of IRStatement pointers
	///
	void attach_invariants(std::vector<std::shared_ptr<IRStatement>>& statements);

	///
	/// @brief Finds the SgFunctionDeclaration that an SgPragma is attached to.
	///
	/// @param pragma Pragma to use as basis for search
	/// @param found_function Set to the associated function if the function was found, otherwise do nothing
	/// @return Whether or not we found the function
	///
	bool find_function(SgPragma* pragma, SgFunctionDeclaration*& found_function);

	///
	/// @brief Merges the verification conditions of two functions.
	///
	/// This usually takes a function prototype and a function definition.
	/// In the case that one function pointer has VCs and the other one doesn't, the VCs are copied
	/// from the function with VCs to the one without. In the case that both functions have VCs,
	/// they are checked to make sure they're identical (but potentially in a different order).
	///
	/// @param func1 The first function
	/// @param func2 The second function
	///
	void merge_vcs(std::shared_ptr<IRFunction> func1, std::shared_ptr<IRFunction> func2);

	///
	/// @brief Downwards traversal.
	///
	/// @param astNode SAGE node to look at
	/// @param inheritedValue The inherited values
	/// @return The new inherited values to pass on down
	///
	IRGeneratorIV evaluateInheritedAttribute(SgNode *astNode, IRGeneratorIV inheritedValue);

	///
	/// @brief Upwards traversal.
	///
	/// @param astNode SAGE node to look at
	/// @param inheritedValue The inherited values
	/// @param list Synthesized attributes so far
	/// @return The new synthesized value
	///
	std::shared_ptr<IRNode> evaluateSynthesizedAttribute(SgNode *astNode, IRGeneratorIV inheritedValue, SynthesizedAttributesList list);

public:
	///
	/// @brief Tests if two IR types are the same type, sans "const" qualifiers
	///
	/// @param a The first type parameter
	/// @param b The second type parameter
	/// @return true if equal, false otherwise
	///
	static bool are_same_type_without_const(std::shared_ptr<IRType> a, std::shared_ptr<IRType> b);

private:
};
