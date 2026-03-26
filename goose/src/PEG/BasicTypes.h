// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_BasicTypes_H
#define PEG_BasicTypes_H

#include <memory>

/// The namespace of PEG parsing.
/// TODO: Maybe rename to GOOSE?
namespace PEG {

/// An enumeration of all types of expressions supported
enum EXPRESSION_TYPE {
    PEG_AND,          ///< And/& Expression
    PEG_DOT,          ///< Dot/Any Expression
    PEG_EMPTY,        ///< Empty/Epsilon Expression
    PEG_NON_TERMINAL, ///< Non-Terminal Expression
    PEG_NOT,          ///< Not/! Expression
    PEG_OPTION,       ///< Option/? Expression
    PEG_PLUS,         ///< Plus/+ Expression
    PEG_REPEAT,       ///< Repeat/* Expression
    PEG_SEQ_N,        ///< MultiSequence/SeqN Expression
    PEG_SEQUENCE,     ///< Sequence/Seq Expression
    PEG_TERMINAL      ///< Terminal Expression
};

/// Abstract class that represents the Expressions that form the Parsing
/// Expression Grammar.
class Expression;

/// An alias for shared pointers to an object of class that publicly inherits Expression.
using ExpressionPtr = std::shared_ptr<Expression>;

/// Class that is used to keep the Grammar rules, start state, and related functions.
class Grammar;

/// An alias for shared pointers to an object of class Grammar.
using GrammarPtr = std::shared_ptr<Grammar>;

/// An Abstract class that represents the output values of successful parses.
class ValueInterface;

/// An alias for shared pointers to an object of class that publicly inherits ValueInterface.
using ValueInterfacePtr = std::shared_ptr<ValueInterface>;

/// An Abstract class that defines an interface to downcast/create objects of any class
/// that publicly inherit ValueInterface; aka a Factory.
class ValueFactoryInterface;

/// An alias for shared pointers to an object of class that publicly inherits ValueFactoryInterface.
using ValueFactoryInterfacePtr = std::shared_ptr<ValueFactoryInterface>;

/// An abstract class that represents the result of parses.
class ParseResult;

/// An alias for shared pointers to an object of class that publicly inherits ParseResult.
using ParseResultPtr = std::shared_ptr<ParseResult>;

/// Class that becomes the entry point to write grammars, and parse input based on the grammar.
class Parser;

/// An alias for shared pointers to an object of class Parser.
using ParserPtr = std::shared_ptr<Parser>;

} // namespace PEG

#endif