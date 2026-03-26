// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_BootstrapParser_H
#define PEG_BootstrapParser_H

#include <PEG/BasicTypes.h>
#include <PEG/Grammar.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Value/ValueFactoryInterface.h>

namespace PEG {

/// @brief An Output Value class that implements ValueInterface and contains the generated rules required to create the
/// output grammar.
///
/// Publicly inherits from ParseTree and transitively inherits from ValueInterface. This was preferred instead of
/// directly inheriting from ValueInterface because it lets us reuse the print functions defined in ParseTree.
///
/// TODO: currently required to be in the outer namespace. Maybe move to Bootstrap namespace and adjust everything else?
class GrammarRuleValue final: public ParseTree::PlainParseTree {
public:
    ///< A hashmap that associates non-terminals with their production rules.
    static std::unordered_map<std::string, std::vector<ExpressionPtr>> rules;
    ///< The start non-terminal for the grammar.
    std::string start_rule;
    ///< A boolean that says to enable (true) left-recursion in the generated grammar. Is false, otherwise.
    static bool doLeftRecursion;

    GrammarRuleValue(
        std::string lhs, const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) : PlainParseTree(label, position, value, children), start_rule(std::move(lhs)) {}
};

/// Namespace that confines all the Bootstrap Parser implementation.
namespace Bootstrap {

/// Class that defines everything related to bootstrapping a PEG parser generation using a PEG Parser.
class BootstrapParser final {

public:
    ///< The output grammar that is generated.
    GrammarPtr peg_grammar;

    ///< @brief The semantic actions to be associated with non-terminals in the grammar.
    /// Any non-terminal which doesn't have a corresponding entry in the hashmap is given the default PlainParseTree
    /// action.
    static std::unordered_map<std::string, ValueFactoryInterfacePtr> action_map;

    /// Constructor for Bootstrap Parser.
    /// @param input String input from which to generate the output grammar & parser.
    /// @param custom_action_map A hashmap specifying any semantics actions for the non-terminals of the grammar.
    BootstrapParser(
        const std::string &input, std::unordered_map<std::string, ValueFactoryInterfacePtr> custom_action_map
    );

    /// A default destructor.
    ~BootstrapParser() = default;
};

/// A ValueInterface class that contains the output generated grammar.
class GrammarValue final: public ParseTree::PlainParseTree {
public:
    ///< A shared pointer to an object of class Grammar - the generated grammar.
    GrammarPtr grammar;

    explicit GrammarValue(
        GrammarPtr grammar, const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) : PlainParseTree(label, position, value, children), grammar(std::move(grammar)) {}
};

/// @brief A helper subclass of ValueFactoryInterface for transforming outputs of non-terminal parse output.
///
/// A non-terminal parse usually contains the name of the non-terminal, the rule used for producing this output, and
/// the actual output of the parsing as the single element in its \p children. This helper class simply returns the
/// output child for cases when only that matters (i.e. we do not need the label, or rule no, just the value).
///
/// Note that there are no checks in this Factory: just transparently returning the first child in \p children.
///
/// Also referred to as NonTerminalLift
class Lift final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        return children[0];
    }
};
} // namespace Bootstrap
} // namespace PEG

#endif