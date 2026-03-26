// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <string>
#include <vector>
#include <memory>

#ifndef VC_GEN
#define VC_GEN

namespace Why3
{
	class WhyFunction;

	class WhyVariable;

	class WhyLValue;

	class WhyRValue;

	class WhyExpression;

	class WhyType;

	class WhyClassType;
}

///
/// @brief Holds whether generate_writes is generating a `writes` clause or a `frees` clause.
///
enum WritesGenerator
{
	GenerateWrites, ///< Generating a `writes` clause
	GenerateFrees   ///< Generating a `frees` clause
};

using namespace Why3;

///
/// @brief Generates bound constraints for class member variables (e.g., is_sint32)
///
/// @param type The class type in question
/// @param addr The address of the object to generate constraints for
/// @param global_offset The offset to begin offsetting from addr
/// @return The Why3 verification condition
///
std::string generate_class_bounds(std::shared_ptr<WhyClassType> type, std::shared_ptr<WhyRValue> addr, int global_offset);

///
/// @brief Generates common verification conditions for functions and loops.
///
/// An example of a verification condition generated here might be that two non-pointer function parameters do not alias the same value.
///
/// @param vars List of variables in play
/// @param in_loop Whether or not we're generating loop VCs (default=false)
/// @return The Why3 verification conditions
///
std::string generate_vcs(std::vector<std::shared_ptr<WhyVariable>> vars, bool in_loop = false);

///
/// @brief Generate the Why3 for a `valid` clause
///
/// @param vars List of variables that currently exist
/// @param lv Variable to assert is valid
/// @return The Why3 clause generated
///
std::string generate_valid(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyExpression> lv);

///
/// @brief Called if lv's size in the Why3 memory model is greater than 1
///
/// This is sometimes called by generate_valid if its lv's size is greater than 1.
/// In this case, we need to write a slightly different verification condition.
/// This can also be called by valid_array, in which case the verification condition is similar.
///
/// @param vars List of variables that currently exist
/// @param lv Variable to assert is valid
/// @param size The size of the chunk of memory to assert is valid
/// @return The Why3 clause generated
///
std::string generate_valid_contiguous(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyExpression> lv, std::shared_ptr<WhyRValue> size);

///
/// @brief Generates a separated clause
///
/// @param vars List of variables that currently exist
/// @param lvs List of variables to assert are separated
/// @return The Why3 clause generated
///
std::string generate_separated(std::vector<std::shared_ptr<WhyVariable>> vars, std::vector<std::shared_ptr<WhyExpression>> lvs);

///
/// @brief Generates either a `writes` or a `frees` clause
///
/// @param lvs List of variables to assert we're writing to or freeing
/// @param in_loop Whether or not this clause is for a loop
/// @param to_write Whether or not we're generating a `writes` or a `frees`
/// @param debug_str The debug identifier to propogate Why3 results back to the user
/// @return The Why3 clause generated
///
std::string generate_writes(std::vector<std::shared_ptr<WhyLValue>> lvs, bool in_loop, WritesGenerator to_write, std::string debug_str);

///
/// @brief Generates a list of contracts based on a function's return type
///
/// For example, if we're returning an object, we need to specify how much data is coming out
///
/// @param return_type The return type of the function
/// @param is_lvalue Whether or not the function is an lvalue function
///
std::string generate_contracts(std::shared_ptr<WhyType> return_type, bool is_lvalue);

#endif
