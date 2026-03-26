// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <string>
#include <memory>
#include "ir.hxx"

using namespace IR;

///
/// @brief Contains all the mechanisms for parsing verification conditions
///
namespace VCParser
{

///
/// @brief Determines what kind of function's VCs we're parsing.
///
/// Lvalue and Rvalue functions (functions that return references vs. values, respectively) need their VCs parsed differently.
/// Mostly, the \<Result\> node might be an Lvalue or Rvalue.
///
enum ParseType
{
	LValueFunc, ///< Represents an Lvalue function parse
	RValueFunc  ///< Represents an Rvalue function parse
};

///
/// @brief Loads the grammar based on the parse type.
///
/// @param parse_type Either LValueFunc or RValueFunc
/// @return The string representing the grammar
///
std::string load(ParseType parse_type);

///
/// @brief Configures the parser for parsing.
///
void configure();

///
/// @brief Parses a verification condition.
///
/// @param inp The string representing the VC
/// @param parse_type Either LValueFunc or RValueFunc
/// @return The VC as an IR AST
///
std::shared_ptr<IRNode> parse_ver(std::string inp, ParseType parse_type);

}
