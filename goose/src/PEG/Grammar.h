// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_Grammar_H
#define PEG_Grammar_H

#include <PEG/BasicTypes.h>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace PEG {

/// A structure that stores the tuple (start, rules) - the constituents of a grammar.
/// TODO: make `start` and `rules.keys` be non-terminals
typedef struct GrammarObject {
    ///< The starting terminal of the grammar.
    std::string start;
    ///< @brief The rules of the grammar.
    ///
    /// These are stored as a hashmap from strings, which correspond to Non-Terminals in the
    /// grammar, to list of Expressions - each member of the list is a Rule in the grammar and
    /// also an Expression.
    std::unordered_map<std::string, std::vector<ExpressionPtr>> rules;

} GrammarObject;

/// Class that defines the Grammar that is used to parse any input.
class Grammar: public std::enable_shared_from_this<Grammar> {
public:
    GrammarObject gram; ///< A struct that contains the constituents of a grammar. @see GrammarObject.

    bool doLeftRecursion = true; ///< Whether this grammar supports left recursion or not.

    /// Prints the grammar as a string to std::cout.
    void print() const;

    /// @brief Given a Non-Terminal, try to parse the given input from a given position using the
    /// corresponding rules.
    ///
    /// The function iterates over all the rules (or alternations) belonging to the non-terminal in the
    /// given order as per the grammar and picks the first result that is a success (i.e. not fail).
    ///
    /// If the grammar is left-recursive, then extra steps are done to ensure a maximum possible parse.
    /// To explicitly enable/disable left-recursion, see the other parse_from_non_terminal.
    ///
    /// @param non_terminal A string that represents the Non-Terminal
    /// @param precedence_level Precedence Level that is used to compare the current parse with a previous
    /// stored level that helps in stopping excessive recursive attempts.
    /// @param input The input that is to be parsed, given as a string
    /// @param position The position in the index from which to start parsing.
    /// @param value_factory The factory class to be used to create an output value in case there's a
    /// successful parse.
    /// @return The result of the parse
    ParseResultPtr parse_from_non_terminal(
        const std::string &non_terminal, const int &precedence_level, const std::string &input, const int &position,
        ValueFactoryInterfacePtr value_factory
    );

    /// Instantiate a shared pointer to an initialized object of class Grammar.
    /// @param gram_start The starting non-terminal represented as a string. This is what any parse
    /// that uses this grammar will start with.
    /// @param gram_rules The rules of the grammar.
    /// @return A shared pointer to an object of class Grammar.
    static GrammarPtr
    instance(const std::string &gram_start, std::unordered_map<std::string, std::vector<ExpressionPtr>> gram_rules);

    ///< A string representing the non-terminal that contains the rules for parsing away whitespace.
    std::string white_space = "_WHITESPACE_";

    /// Parse any whitespaces in the input and skip ahead.
    /// @param input The input currently being parsed, as a string.
    /// @param position_before_space The starting position or the current position in the parse.
    /// @return The next position to parse from after having skipped all whitespaces, if any.
    int parse_whitespace(const std::string &input, const int &position_before_space);

    /// Define a custom hashing function for our unordered_map that uses pair for keys.
    struct hash_pair {
        template <class T1, class T2> std::size_t operator()(const std::pair<T1, T2> &pair) const {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };
    using MemoK =
        std::pair<std::string, int>; ///< Keys of the memoization table: <non-terminal, position index on input>
    using MemoV = std::pair<ParseResultPtr, int>; ///< Values of the memoization table: <parse-result, precedence level>
    using MapKV = std::unordered_map<MemoK, MemoV, hash_pair>; ///< Type alias for custom hashmap for memoization table.

    static MapKV L_table; ///< A memoization table for storing intermediate results while doing left-recursive parses.

    /// @brief Check if there exists dangling non-terminals in our Grammar.
    ///
    /// i.e. making sure that there aren't any non-terminals which do not produce anything i.e. no
    /// there are no production rules.
    /// TODO: refactor into a bfs-style approach.
    ///
    /// @return A boolean value of true if we couldn't find any dangling non-terminals. False, otherwise.
    [[nodiscard]] bool check_grammar() const;

    ///< Set that stores all non-terminals that can be nullable i.e. they produce empty strings, if any.
    std::set<std::string> nullable_non_terminals;
    ///< Set that stores all non-terminals that are left-recursive, if any. (non-terminals are stored as
    /// string values).
    std::set<std::string> left_recursive_non_terminals;

    /// Function to find all left-recursive non-terminals using a fix-point, if any.
    /// @return A set of all left-recursive non-terminals in the grammar, represented as strings.
    [[nodiscard]] std::set<std::string> find_all_left_recursion() const;

private:
    /// @brief Growing the result of a left-recursive non-terminal parse by repeatedly attempting it
    /// until a fail occurs.
    ///
    /// This function performs the crucial portion of handling left-recursion by memoizing the current
    /// result of a parse and attempting to parse the same input, position, non-terminal with the new
    /// context (the memoized value). This action is done repeatedly until there's a fail or there's no
    /// more forward progress.
    ///
    /// If the precedence level of the memoized value is larger than the current precedence level, the
    /// parse is considered to be failed - this helps provide left-associative and right-associative
    /// operators in our grammar without much overhead.
    ///
    /// @param current_output The current result of a non-terminal parse.
    /// @param non_terminal The left recursive non-terminal
    /// @param precedence_level The precedence level of the non-terminal.
    /// @param input The input who suffix is to be parsed.
    /// @param position The position of the input from which to start parsing.
    /// @param value_factory The factory class to transform the parsed structure into an output value.
    /// @param next_position The resulting new position of the input after the previous successful parse.
    /// @return The final output of (multiple) left-recursive parses over the non-terminal.
    ParseResultPtr
    inc(ParseResultPtr current_output, const std::string &non_terminal, const int &precedence_level,
        const std::string &input, const int &position, ValueFactoryInterfacePtr value_factory,
        const int &next_position);

    /// @brief Find the "firsts" of all non-terminals using a fix-point.
    ///
    /// A First of a non-terminal refers to all terminal characters that replace the non-terminal after
    /// successful application of its rules.
    ///
    /// @param current_map Current hashmap of all known firsts of non-terminals.
    /// @param nullable_non_terminals A set of all nullable non-terminals - i.e. which produce an empty string.
    /// @return A hashmap that maps non-terminals to its firsts.
    [[nodiscard]] std::unordered_map<std::string, std::set<std::string>> find_all_non_terminal_first(
        const std::unordered_map<std::string, std::set<std::string>> &current_map,
        const std::set<std::string> &nullable_non_terminals
    ) const;

    /// @brief Attempt to parse from the current input position using the rules of the given non-terminal,
    /// with an explicit control over enabling/disabling left-recursion, and precedence based checking.
    ///
    /// This function forms the internal function used by the other parse_from_non_terminal function, where
    /// we enable/disable left-recursive depending on if we were able to find the non-terminal as being
    /// left-recursive. While the precedence based checking usually is done alongside left-recursion, it is
    /// kind of independent.
    ///
    /// Left-recursive parsing is achieved by a greedy approach where we continuously try parsing the
    /// current input suffix, storing the result, and then trying to parse the input again with the new
    /// context with the new stored result in our memoization table. We continue to do this, until we reach
    /// a Fail state - in which case, we use the most recently saved result. This approach can be looked at
    /// as 'growing the seed' where we try to find a maximal output by adding layers over layers.
    /// @see inc
    ///
    /// The precedence check helps introduce left-associative & right-associative operators/non-terminals
    /// in a grammar irrespective of its left-recursive nature. When the precedence level of current parse
    /// is lower than the precedence level of a memoized result of the same parse, we issue a failed parse.
    /// Otherwise, we let it proceed and save its result.
    ///
    /// @param isLeftRecursive Whether to enable (true) or disable (false) left-recursion.
    /// @param non_terminal The non-terminal that we'll start parsing from.
    /// @param precedence_level The precedence level of \p non_terminal. Helps reduce excessive
    /// @param input The input of whose suffix we're interested in parsing.
    /// @param position The current position from which to start parsing. This determinds the remaining
    /// suffix left to parse.
    /// @param value_factory The factory function to pass for transforming the parsed structure into
    /// the final output.
    /// @return A shared pointer to an object that represents the result of the parse.
    ParseResultPtr parse_from_non_terminal(
        const bool &isLeftRecursive, const std::string &non_terminal, const int &precedence_level,
        const std::string &input, const int &position, ValueFactoryInterfacePtr value_factory
    );

    /// @brief Fixpoint to compute all non-terminals that are nullable.
    ///
    /// A nullable non-terminal is one which can produce an empty string.
    /// @return A set of all nullable non-terminals
    std::set<std::string> find_all_nullable();

    /// @brief Given all nullable non-terminals given a set of already known nullable non-terminals.
    ///
    /// This function gives us all nullable non-terminals when we start with an empty set and keep
    /// running this function until we reach a fix point. @see find_all_nullable.
    ///
    /// This decomposes into checking if each Expression can be deduced as nullable given the current
    /// known context of set of nullable non-terminals. The current context prevents recursively finding
    /// whether a non-terminal is nullable or not.
    ///
    /// @param current_result
    /// @return
    std::set<std::string> find_nullable_non_terminals(const std::set<std::string> &current_result);

    /// Constructor for Grammar.
    /// @param gram_start The starting non-terminal of our grammar.
    /// @param gram_rules The rules of our grammar.
    Grammar(
        const std::string &gram_start, const std::unordered_map<std::string, std::vector<ExpressionPtr>> &gram_rules
    );
};

} // namespace PEG

#endif