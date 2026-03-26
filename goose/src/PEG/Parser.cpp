// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Bootstrap/BootstrapParser.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueInterface.h>
#include <cassert>
#include <iostream>
#include <string>

namespace PEG {

// TODO: valueFactory is for START, needs to be within the grammar if possible
ParseResultPtr
Parser::parse(const std::string &input, const ValueFactoryInterfacePtr &valueFactory) {
    // std::cout << "output: starting" << std::endl;

    if (grammar->gram.rules.empty()) {
        ParseResultType::FailParseResult::instance(0);
    }

    // TODO: refactor the loop below to Grammar::parse_from_terminal
    //  start can be put into the grammar as a NonTerminal itself
    //  NonTerm(start) can also then have its own valueFactory attached
    //  and it'll be just start.parse();

    return grammar->parse_from_non_terminal(grammar->gram.start, 1, input, 0, valueFactory);

    // TODO: handle a final whitespace parse
    //  find a way to preserve FailParseResult::error/content so that it can be passed onwards
}

ParseResultPtr
Parser::parse(const std::string &input) {
    return parse(input, ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

ParserPtr
Parser::from_str(const std::string &input) {
    return from_str(input, std::unordered_map<std::string, ValueFactoryInterfacePtr>());
}

ParserPtr
Parser::from_str(const std::string &input, std::unordered_map<std::string, ValueFactoryInterfacePtr> action_map) {
    // load the parser parsing grammar
    const auto bootstrap = Bootstrap::BootstrapParser("", std::move(action_map));
    const auto peg_grammar = bootstrap.peg_grammar;
    // empty the white_space rules since the parser already defines spacing & comments
    peg_grammar->gram.rules[peg_grammar->white_space] = std::vector<ExpressionPtr>();

    // disable left recursion since we know that the bootstrap grammar is not left recursive
    // or rather, it was constructed as such
    peg_grammar->doLeftRecursion = false;

    // TODO: reset generated grammar rules for subsequent input_grammar parses in same instance
    // Bootstrap::GrammarRuleValue::rules;

    // parse the input grammar file
    auto peg_parser = Parser(peg_grammar);
    const auto result = peg_parser.parse(input, ValueFactoryInterface::instance<Bootstrap::Lift>());
    if (!result->isSuccess()) {
        result->output();
        std::cout << "parsing input grammar file failed" << std::endl;
        assert(result->isSuccess());
    }
    const auto output = ValueInterface::downcast<Bootstrap::GrammarValue>(result->valueOutput);
    const auto parsed_grammar = output->grammar;

    const auto check_gram = parsed_grammar->check_grammar();
    if (!check_gram) {
        parsed_grammar->print();
        std::cout << "grammar check failed" << std::endl;
        std::cout << "some non terminals dont have corresponding production rules" << std::endl;
        std::cout << "OR there are more production rules than non terminals used" << std::endl;
        assert(check_gram == true);
    }

    auto new_parser = Parser(parsed_grammar);
    return std::make_shared<Parser>(new_parser);
}

} // namespace PEG