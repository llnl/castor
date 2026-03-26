// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "irtraversal.hxx"
#include "symbol_table.hxx"
#include "ir.hxx"
#include "whyml.hxx"
#include "offset_table.hxx"
#include <utility>

using namespace IR;
using namespace Why3;

///
/// @brief Inherited values for the WhyGenerator class
///
struct WhyGenIV
{
	bool in_vc;                                                        ///< Whether or not we're in a verification condition
	bool in_func;                                                      ///< Whether or not we're in a function
	std::shared_ptr<std::vector<std::pair<std::shared_ptr<IRVariable>, ///< Pointer to list of global variables, paired with
				 std::shared_ptr<IRExpression>>>> globals; ///< a potential const init value
	bool in_class;                                                     ///< Whether or not we're in a class
};

///
/// @brief This class generates the WhyML from the IR
///
class WhyGenerator : public IRTraversal<WhyGenIV, std::shared_ptr<WhyNode>>
{
private:
	///
	/// @brief Gets the WhyType from an IR type.
	///
	/// This does not necessarily correlate 1:1 with a Why3 type. In fact, it usually doesn't.
	/// The pretty-printers handle that.
	///
	/// @param type The IR type
	/// @return The corresponding WhyType
	///
	std::shared_ptr<WhyType> getWhyTypeFromIRType(std::shared_ptr<IRType> type);

	///
	/// @brief Handler for calculating the member offsets for a class.
	///
	/// See OffsetTable for further information.
	///
	/// @param clas The IR class type
	/// @return The offset table
	///
	OffsetTable calculateOffsets(std::shared_ptr<IRClassType> clas);

	///
	/// @brief Gets a list of WhyVariables from a scope object (which usually returns IRVariables).
	///
	/// @param scope The scope object
	/// @return A list of WhyVariables
	///
	std::vector<std::shared_ptr<WhyVariable>> get_variables(std::shared_ptr<R2WML_Scope> scope);

	///
	/// @brief Gets a Why3 literal representing the size of the argument
	///
	/// @param type The type to get the size of
	/// @return A Why3 literal representing the size of the argument
	///
	std::shared_ptr<WhyRValue> get_sizeof(std::shared_ptr<WhyType> type);

protected:
	using SynthesizedAttributesList = IRTraversal<WhyGenIV, std::shared_ptr<WhyNode>>::SynthesizedAttributesList;

	///
	/// @brief Downwards traversal.
	///
	/// @param astNode The IRNode to look at
	/// @param inheritedValue Inherited values passed down
	/// @return Inherited value to pass down
	///
	WhyGenIV evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, WhyGenIV inheritedValue);

	///
	/// @brief Upwards traversal.
	///
	/// @param astNode The IRNode to look at
	/// @param inheritedValue The inherited values
	/// @param list The synthesized attributes so far
	/// @return The new synthesized attribute
	///
	std::shared_ptr<WhyNode> evaluateSynthesizedAttribute(std::shared_ptr<IRNode> astNode, WhyGenIV inheritedValue, SynthesizedAttributesList list);

	///
	/// @brief This is a handler for binary operations.
	///
	/// @param astNode The IRBinaryOperation to look at
	/// @param inheritedValue The inherited values
	/// @param list The synthesized attributes so far
	/// @return The synthesized binary operation
	///
	std::shared_ptr<WhyNode> handleBinaryOperation(std::shared_ptr<IRBinaryOperation> astNode, WhyGenIV inheritedValue, SynthesizedAttributesList list);

	///
	/// @brief This is a handler for assignment operations
	///
	/// @param astNode The IRAssignmentOperation to look at
	/// @param inheritedValue The inherited values
	/// @param list The synthesized attributes so far
	/// @return The synthesized assigment operation
	///
	std::shared_ptr<WhyNode> handleAssignmentOperation(std::shared_ptr<IRAssignmentOperation> astNode, WhyGenIV inheritedValue, SynthesizedAttributesList list);

	///
	/// @brief This is a handler for unary operations.
	///
	/// @param astNode The IRUnaryOperation to look at
	/// @param list The synthesized attributes so far
	/// @return The synthesized unary operation
	///
	std::shared_ptr<WhyNode> handleUnaryOperation(std::shared_ptr<IRUnaryOperation> astNode, SynthesizedAttributesList list);

	///
	/// @brief This is a handler for literals
	///
	/// @param astNode The IRLiteral to look at
	/// @return The Why3 literal object
	///
	std::shared_ptr<WhyNode> handleLiteral(std::shared_ptr<IRLiteral> astNode);

	///
	/// @brief This is a handler for function calls that return a reference
	///
	/// @param ast The function call expression
	/// @param list The synthesized attributes list
	/// @return The WhyFunctionCall objet
	///
	std::shared_ptr<WhyNode> handleFunctionCall(std::shared_ptr<IRFunctionCallExpr<IRLValue>> ast, SynthesizedAttributesList list);

	///
	/// @brief This is a handler for function calls that return a non-reference
	///
	/// @param ast The function call expression
	/// @param list The synthesized attributes list
	/// @return Result of calling the function (might be a WhyFunctionCall, might be a builtin function)
	///
	std::shared_ptr<WhyNode> handleFunctionCall(std::shared_ptr<IRFunctionCallExpr<IRRValue>> ast, SynthesizedAttributesList list);

public:
	///
	/// @brief Constructor
	///
	WhyGenerator() = default;

	///
	/// @brief Turns an arbitrary expression into an rvalue expression.
	///
	/// This handles lvalue-to-rvalue conversion. If we already have an rvalue, just return that.
	///
	/// @param expr The expression to convert
	/// @return The converted expression
	///
	static std::shared_ptr<WhyRValue> makeRValue(std::shared_ptr<WhyExpression> expr);

	///
	/// @brief Functor, so that instances of this class may be used as a function
	///
	/// @param project The base IRNode
	/// @return The synthesized WhyNode
	///
	std::shared_ptr<WhyNode> operator()(std::shared_ptr<IRNode> project);
};

