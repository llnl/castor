// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_Parser_H
#define PEG_Parser_H

#include <PEG/BasicTypes.h>
#include <PEG/Grammar.h>

namespace PEG {

/// Class that becomes the entry point to write grammars, and parse input based on the grammar.
class Parser final {
public:
    /// @brief Try to parse to an input to obtain a value.
    /// @param input The string input to be checked against the grammar.
    /// @param valueFactory pointer to Factory that can take the output parsed structure as it's input and
    /// produce the desired value.
    /// @return A ParseResult object that can either be a failed parse or a successful parse with a value.
    ParseResultPtr parse(const std::string &input, const ValueFactoryInterfacePtr &valueFactory);

    /// @brief Parse an input and produce a default printable structure.
    ///
    /// Can be considered as a default parse that doesn't require a value factory.
    ///
    /// @param input The input to be parsed given as a string.
    /// @return A ParseResult object that can either be a failed parse or a successful parse with a structured output
    /// string.
    ParseResultPtr parse(const std::string &input);

    GrammarPtr grammar; ///< Stores the Grammar object that is used to parse inputs against.

    /// Constructor to create an object of class Parser.
    /// @param g Input grammar object
    explicit Parser(GrammarPtr g) : grammar(g) {}

    /// @brief Create a Parser from a string that contains all the rules and allow custom semantic
    /// actions
    ///
    /// Example usage:
    /// `Parser::from_str("
    ///     E -> M '+' N \@A1
    ///     M -> P '*' N \@A2
    ///     P -> '(' E ')' \@A3
    ///     N -> [0-9]+ \@A3
    /// ", {{"A1", PtrObject1}, {"A2", PtrObject2}, {"A3", PtrObject3}});
    ///
    /// Essentially, the input string is parsed using BootstrapParser which outputs a Parser that
    /// has a (generated) grammar corresponding to the list of rules.
    ///
    /// @param input A string value that contains the list of rules to create a grammar.
    /// @param action_map A hashmap that stores the names of semantic actions to its corresponding
    /// shared pointer object.
    /// @return A shared pointer to a Parser made using the generated grammar and provided actions
    static ParserPtr
    from_str(const std::string &input, std::unordered_map<std::string, ValueFactoryInterfacePtr> action_map);

    /// @brief Create a Parser from a string of rules
    ///
    /// This produces a Parser with a Grammar that has the rules as specified in the string. The
    /// output of parse will just be a parsed string structure.
    ///
    /// This is basically `Parser::from_str(input, default_action_map)` where `default_action_map`
    /// maps all output values to just string that can be printed.
    ///
    /// @param input A string value that contains all list of rules to create a grammar.
    /// @return A shared pointer to a Parser made using the generated grammar.
    static ParserPtr from_str(const std::string &input);
};

} // namespace PEG

#endif