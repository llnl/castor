// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_Expression_H
#define PEG_Expression_H

#include <PEG/BasicTypes.h>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace PEG {

/// @brief An abstract class to represent Expressions.
///
/// This forms the single core element of parsing using Parsing Expression Grammars.
/// We define our grammar using rules which are composed off of Expressions - which are
/// atomic, recursive or derived using other Expressions.
///
/// In addition to being able to parse, we also have helper functions to print, and
/// check nullability.
class Expression {

public:
    /// @brief Parse the given input starting from the given position within the
    /// context of a given grammar.
    ///
    /// @param grammar Grammar object with the current set of rules for parsing.
    /// @param input Input string whose suffix is to be parsed.
    /// @param position The position in \p input that marks the start of parsing.
    /// @return A shared pointer to an object of class ParseResult.
    virtual ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) = 0;

    /// @brief Return information about the Expression object.
    ///
    /// This is usually a visual description of what the Expression consists of, with
    /// constituent's info used if any.
    ///
    /// @return A string representation of the object.
    virtual std::string info() = 0;

    /// @brief A symbolic representation of the Expression
    ///
    /// As one would write while writing rules on paper, text or using
    /// Parser::from_str.
    ///
    /// @return A string with symbols corresponding to the object.
    virtual std::string print() = 0;

    /// A default destructor.
    virtual ~Expression() = default;

    /// @brief Constructor for Expression.
    ///
    /// Initializes the factory class that transform parsed ouput structures into
    /// output values.
    ///
    /// @param valueFactory A abstract class shared pointer to an object of class that
    /// publicly inherits ValueFactoryInterface.
    explicit Expression(ValueFactoryInterfacePtr valueFactory) : value_factory(std::move(valueFactory)) {}

    ///< @brief Factory that can transform the output of a successful parse into a
    /// value as is appropriate to the use case.
    ///
    /// A default factory stores the parse output as is and lets you print it.
    /// This can be useful for understanding how the parse output looks and then
    /// slowly iterate over the final desired semantic action.
    /// @see PlainParseTree
    ValueFactoryInterfacePtr value_factory;

    /// Helper function to tell us what Expression the current object is.
    /// @return A member of the enumeration EXPRESSION_TYPE that corresponds to the
    /// kind of Expression.
    virtual EXPRESSION_TYPE get_expr_type() = 0;

    /// Find non-terminals embedded in the expression, if any
    /// @return Set of non-terminals present within the expression.
    virtual std::vector<std::string> get_non_terms() = 0;

    /// Whether the current Expression object is nullable in the given context.
    /// @param nullable_non_terminals Set of already known/assumed nullable
    /// non-terminal given as context.
    /// @return A boolean value where true means the object is nullable, false
    /// otherwise.
    virtual bool isNullable(const std::set<std::string> &nullable_non_terminals) = 0;

    /// @brief Find the first of a given expression in the present context.
    ///
    /// @see Grammar::find_all_non_terminal_first
    ///
    /// @param non_terminal_map_first A hashmap of already known/provided first values
    /// of non-terminal.
    /// @param nullable_non_terminals A set of known/provided nullable non-terminals.
    /// @return The set of first of current object.
    virtual std::set<std::string> find_first(
        const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
        const std::set<std::string> &nullable_non_terminals
    ) = 0;
};

} // namespace PEG

#endif