// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "PEG/Bootstrap/BootstrapParser.h"
#include "PEG/ParseResult.h"
#include "PEG/ParseTree/PlainParseTree.h"
#include "PEG/Value/ValueFactoryInterface.h"
#include "PEG/Value/ValueInterface.h"

#include <PEG/Parser.h>
#include <iostream>
#include <string>

class NameFactory final : public PEG::ValueFactoryInterface {
public:
    PEG::ValueInterfacePtr createParseTree(const std::string &label, const int &position, const std::string &value, const std::vector<PEG::ValueInterfacePtr> &children) override {
        std::string output_value;
        const auto &child_0 = children[0];
        auto child_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(child_0);
        output_value += child_tree->value;

        const auto &child_1 = children[1];
        auto child_1_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(child_1);
        if(child_1_tree->label == "repeat") {
            for(auto const &iter : child_1_tree->children) {
                auto iter_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(iter);
                output_value += iter_tree->value;
            }
        }
        return std::make_shared<PEG::ParseTree::PlainParseTree>(label, position, output_value, std::vector<PEG::ValueInterfacePtr>());
    }
};

int
main() {
    const std::string input_grammar = R"N(
        Name   <- [A-Za-z_][A-Za-z0-9_]* @nameFactory
    )N";

    std::unordered_map<std::string, PEG::ValueFactoryInterfacePtr> custom_actions;
    custom_actions.insert({"nameFactory", PEG::ValueFactoryInterface::instance<NameFactory>()});

    const auto parser = PEG::Parser::from_str(input_grammar, custom_actions);

    parser->grammar->print();

    const auto result = parser->parse("minimum_is_990", PEG::ValueFactoryInterface::instance<PEG::Bootstrap::Lift>());

    result->output();
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    if(result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);
    }
    return 0;
}