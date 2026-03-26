// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/DotExpression.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/NotExpression.h>
#include <PEG/Expressions/OptionExpression.h>
#include <PEG/Expressions/PlusExpression.h>
#include <PEG/Expressions/RepeatExpression.h>
#include <PEG/Expressions/SequenceExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Parser.h>
#include <cassert>
#include <iostream>

// g++ -Wall -O3 -I.. -std=c++14 peg.cpp -o peg
// g++ -std=c++14 -Wall -O3 -I.. -fPIC ../PEG/ParseTree.cpp ../PEG/FailParseResult.cpp ../PEG/SuccessParseResult.cpp
// ../PEG/TerminalExpression.cpp ../PEG/NonTerminalExpression.cpp ../PEG/SequenceExpression.cpp ../PEG/NotExpression.cpp
// ../PEG/RepeatExpression.cpp ../PEG/DotExpression.cpp ../PEG/EmptyExpression.cpp ../PEG/OptionExpression.cpp
// ../PEG/PlusExpression.cpp ../PEG/Grammar.cpp ../PEG/Parser.cpp -shared -o libPEG.so g++ -std=c++14 -Wall -O3 -I..
// -fPIC ./libPEG.so ../bootstrap.cpp -o peg.out

// peg_sequence_from_list(std::vector<std::string>({"n", "r", "t", "'", "[", "]", "\\"}))
PEG::ExpressionPtr peg_sequence_from_list(std::vector<std::string> chars);
// peg_sequence_from_digits(rules, 0, 7)
PEG::ExpressionPtr
peg_sequence_from_digits(std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, int start, int end);
// peg_non_terminal_from_alphabets(rules, 'a', 'z')
PEG::ExpressionPtr peg_non_terminal_from_alphabets(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, char start, char end
);

PEG::GrammarPtr
make_grammar() {
    /*
    # Hierarchical syntax
    Grammar <- Spacing Definition+ EndOfFile
    Definition <- Identifier LEFTARROW Expression
    Expression <- Sequence (SLASH Sequence)*
    Sequence <- Prefix*
    Prefix <- (AND / NOT)? Suffix
    Suffix <- Primary (QUESTION / STAR / PLUS)?
    Primary <- Identifier !LEFTARROW
    / OPEN Expression CLOSE
    / Literal / Class / DOT

    # Lexical syntax
    Identifier <- IdentStart IdentCont* Spacing
    IdentStart <- [a-zA-Z_]
    IdentCont <- IdentStart / [0-9]
    Literal <- [’] (![’] Char)* [’] Spacing
    / ["] (!["] Char)* ["] Spacing
    Class <- ’[’ (!’]’ Range)* ’]’ Spacing
    Range <- Char ’-’ Char / Char
    Char <- ’\\’ [nrt’"\[\]\\]
    / ’\\’ [0-2][0-7][0-7]
    / ’\\’ [0-7][0-7]?
    / !’\\’ .
    LEFTARROW <- ’<-’ Spacing
    SLASH <- ’/’ Spacing
    AND <- ’&’ Spacing
    NOT <- ’!’ Spacing
    QUESTION <- ’?’ Spacing
    STAR <- ’*’ Spacing
    PLUS <- ’+’ Spacing
    OPEN <- ’(’ Spacing
    CLOSE <- ’)’ Spacing
    DOT <- ’.’ Spacing
    Spacing <- (Space / Comment)*
    Comment <- ’#’ (!EndOfLine .)* EndOfLine
    Space <- ’ ’ / ’\t’ / EndOfLine
    EndOfLine <- ’\r\n’ / ’\n’ / ’\r’
    EndOfFile <- !.
    */

    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> rules;

    std::string start = "Grammar";

    // EndOfFile <- !.
    const std::string end_of_file = "EndOfFile";
    rules.insert(std::make_pair(
        end_of_file, std::vector<PEG::ExpressionPtr>({PEG::Expressions::NotExpression::instance(PEG::Expressions::DotExpression::instance())})
    ));

    // EndOfLine <- ’\r\n’ / ’\n’ / ’\r’
    std::string end_of_line = "EndOfLine";
    rules.insert(std::make_pair(
        end_of_line, std::vector<PEG::ExpressionPtr>(
                         {PEG::Expressions::TerminalExpression::instance("\r\n"), PEG::Expressions::TerminalExpression::instance("\n"),
                          PEG::Expressions::TerminalExpression::instance("\r")}
                     )
    ));

    // Space <- ’ ’ / ’\t’ / EndOfLine
    const std::string space = "Space";
    rules.insert(std::make_pair(
        space, std::vector<PEG::ExpressionPtr>(
                   {PEG::Expressions::TerminalExpression::instance(" "), PEG::Expressions::TerminalExpression::instance("\t"),
                    PEG::Expressions::NonTerminalExpression::instance(end_of_line)}
               )
    ));

    // Comment <- '#' (!EndOfLine .)* EndOfLine
    const std::string comment = "Comment";
    rules.insert(std::make_pair(
        comment, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                     PEG::Expressions::SequenceExpression::instance(
                         PEG::Expressions::TerminalExpression::instance("#"),
                         PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                             PEG::Expressions::NotExpression::instance(PEG::Expressions::NonTerminalExpression::instance(end_of_line)),
                             PEG::Expressions::DotExpression::instance()
                         ))
                     ),
                     PEG::Expressions::NonTerminalExpression::instance(end_of_line)
                 )})
    ));

    // Spacing <- (Space / Comment)*
    // rewritten as
    // Spacing <- (SpacingExtra)*
    // SpacingExtra <- Space / Comment
    std::string spacing_extra = "SpacingExtra";
    rules.insert(std::make_pair(
        spacing_extra, std::vector<PEG::ExpressionPtr>(
                           {PEG::Expressions::NonTerminalExpression::instance(space), PEG::Expressions::NonTerminalExpression::instance(comment)}
                       )
    ));

    std::string spacing = "Spacing";
    rules.insert(std::make_pair(
        spacing, std::vector<PEG::ExpressionPtr>(
                     {PEG::Expressions::RepeatExpression::instance(PEG::Expressions::NonTerminalExpression::instance(spacing_extra))}
                 )
    ));

    // DOT <- ’ .’ Spacing
    std::string dot = "DOT";
    rules.insert(std::make_pair(
        dot, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                 PEG::Expressions::TerminalExpression::instance("."), PEG::Expressions::NonTerminalExpression::instance(spacing)
             )})
    ));

    // CLOSE <- ’ )’ Spacing
    std::string close = "CLOSE";
    rules.insert(std::make_pair(
        close, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                   PEG::Expressions::TerminalExpression::instance(")"), PEG::Expressions::NonTerminalExpression::instance(spacing)
               )})
    ));

    // OPEN <- ’ (’ Spacing
    std::string open = "OPEN";
    rules.insert(std::make_pair(
        open, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                  PEG::Expressions::TerminalExpression::instance("("), PEG::Expressions::NonTerminalExpression::instance(spacing)
              )})
    ));

    // PLUS < - ’ +’ Spacing
    std::string plus = "PLUS";
    rules.insert(std::make_pair(
        plus, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                  PEG::Expressions::TerminalExpression::instance("+"), PEG::Expressions::NonTerminalExpression::instance(spacing)
              )})
    ));

    // STAR < - ’ *’ Spacing
    std::string star = "STAR";
    rules.insert(std::make_pair(
        star, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                  PEG::Expressions::TerminalExpression::instance("*"), PEG::Expressions::NonTerminalExpression::instance(spacing)
              )})
    ));

    // QUESTION < - ’?’ Spacing
    std::string question = "QUESTION";
    rules.insert(std::make_pair(
        question, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                      PEG::Expressions::TerminalExpression::instance("?"), PEG::Expressions::NonTerminalExpression::instance(spacing)
                  )})
    ));

    // NOT < - ’ !’ Spacing
    std::string not_ = "NOT";
    rules.insert(std::make_pair(
        not_, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                  PEG::Expressions::TerminalExpression::instance("!"), PEG::Expressions::NonTerminalExpression::instance(spacing)
              )})
    ));

    // AND < - ’ &’ Spacing
    std::string and_ = "AND";
    rules.insert(std::make_pair(
        and_, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                  PEG::Expressions::TerminalExpression::instance("&"), PEG::Expressions::NonTerminalExpression::instance(spacing)
              )})
    ));

    // SLASH <- ’/’ Spacing
    std::string slash = "SLASH";
    rules.insert(std::make_pair(
        slash, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                   PEG::Expressions::TerminalExpression::instance("/"), PEG::Expressions::NonTerminalExpression::instance(spacing)
               )})
    ));

    // LEFTARROW <- '<-' Spacing
    std::string left_arrow = "LEFTARROW";
    rules.insert(std::make_pair(
        left_arrow, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                        PEG::Expressions::TerminalExpression::instance("<-"), PEG::Expressions::NonTerminalExpression::instance(spacing)
                    )})
    ));

    /* CHAR <- '\\' [nrt'"\[\]\\]
     *      /  '\\' [0-2][0-7][0-7]
     *      /  '\\' [0-7][0-7]?
     *      / !'\\' .
     */

    std::string charas = "CharEx";
    rules.insert(std::make_pair(
        charas, std::vector<PEG::ExpressionPtr>(
                    {PEG::Expressions::TerminalExpression::instance("n"), PEG::Expressions::TerminalExpression::instance("r"),
                     PEG::Expressions::TerminalExpression::instance("t"), PEG::Expressions::TerminalExpression::instance("'"),
                     PEG::Expressions::TerminalExpression::instance("["), PEG::Expressions::TerminalExpression::instance("]"),
                     PEG::Expressions::TerminalExpression::instance("\\")}
                )
    ));

    auto digits0to7 = peg_sequence_from_digits(rules, 0, 7);

    std::string char_ = "Char";
    rules.insert(std::make_pair(
        char_,
        std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::SequenceExpression::instance(
                 PEG::Expressions::TerminalExpression::instance("\\"), PEG::Expressions::NonTerminalExpression::instance(charas)
             ),
             PEG::Expressions::SequenceExpression::instance(
                 PEG::Expressions::SequenceExpression::instance(
                     PEG::Expressions::SequenceExpression::instance(
                         PEG::Expressions::TerminalExpression::instance("\\"), peg_sequence_from_digits(rules, 0, 2)
                     ),
                     digits0to7
                 ),
                 digits0to7
             ),
             PEG::Expressions::SequenceExpression::instance(
                 PEG::Expressions::SequenceExpression::instance(PEG::Expressions::TerminalExpression::instance("\\"), digits0to7),
                 PEG::Expressions::OptionExpression::instance(digits0to7)
             ),
             PEG::Expressions::SequenceExpression::instance(
                 PEG::Expressions::NotExpression::instance(PEG::Expressions::TerminalExpression::instance("\\")), PEG::Expressions::DotExpression::instance()
             )}
        )
    ));

    /*
    // CHAR <- !'\\' .
    std::string char_ = "Char";
    rules.insert(std::make_pair(
        char_, std::vector<PEG::ExpressionPtr>({
            PEG::SequenceExpression::instance(
                PEG::NotExpression::instance(PEG::TerminalExpression::instance("\\")),
                PEG::DotExpression::instance())
        })));
    */

    // Range <- Char '-' Char / Char
    std::string range = "Range";
    rules.insert(std::make_pair(
        range, std::vector<PEG::ExpressionPtr>(
                   {PEG::Expressions::SequenceExpression::instance(
                        PEG::Expressions::SequenceExpression::instance(
                            PEG::Expressions::NonTerminalExpression::instance(char_), PEG::Expressions::TerminalExpression::instance("-")
                        ),
                        PEG::Expressions::NonTerminalExpression::instance(char_)
                    ),
                    PEG::Expressions::NonTerminalExpression::instance(char_)}
               )
    ));

    // Class <- '[' (!']' Range)* ']' Spacing
    std::string class_ = "Class";
    rules.insert(std::make_pair(
        class_, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::SequenceExpression::instance(
                        PEG::Expressions::SequenceExpression::instance(
                            PEG::Expressions::TerminalExpression::instance("["),
                            PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                                PEG::Expressions::NotExpression::instance(PEG::Expressions::TerminalExpression::instance("]")),
                                PEG::Expressions::NonTerminalExpression::instance(range)
                            ))
                        ),
                        PEG::Expressions::TerminalExpression::instance("]")
                    ),
                    PEG::Expressions::NonTerminalExpression::instance(spacing)
                )})
    ));

    // Literal <- ['] (!['] Char)* ['] Spacing
    //         /  ["] (!["] Char)* ["] Spacing
    std::string literal = "Literal";
    rules.insert(std::make_pair(
        literal, std::vector<PEG::ExpressionPtr>(
                     {PEG::Expressions::SequenceExpression::instance(
                          PEG::Expressions::SequenceExpression::instance(
                              PEG::Expressions::SequenceExpression::instance(
                                  PEG::Expressions::TerminalExpression::instance("'"),
                                  PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                                      PEG::Expressions::NotExpression::instance(PEG::Expressions::TerminalExpression::instance("'")),
                                      PEG::Expressions::NonTerminalExpression::instance(char_)
                                  ))
                              ),
                              PEG::Expressions::TerminalExpression::instance("'")
                          ),
                          PEG::Expressions::NonTerminalExpression::instance(spacing)
                      ),
                      PEG::Expressions::SequenceExpression::instance(
                          PEG::Expressions::SequenceExpression::instance(
                              PEG::Expressions::SequenceExpression::instance(
                                  PEG::Expressions::TerminalExpression::instance("\""),
                                  PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                                      PEG::Expressions::NotExpression::instance(PEG::Expressions::TerminalExpression::instance("\"")),
                                      PEG::Expressions::NonTerminalExpression::instance(char_)
                                  ))
                              ),
                              PEG::Expressions::TerminalExpression::instance("\"")
                          ),
                          PEG::Expressions::NonTerminalExpression::instance(spacing)
                      )}
                 )
    ));

    // Identifier <- IdentStart IdentCont* Spacing
    // IdentCont  <- IdentStart / [0-9]
    // IdentStart <- [a-zA-Z]
    std::string ident_start = "IdentStart";
    rules.insert(std::make_pair(
        ident_start,
        std::vector<PEG::ExpressionPtr>({
            peg_non_terminal_from_alphabets(rules, 'a', 'z'), peg_non_terminal_from_alphabets(rules, 'A', 'Z')
            // peg_sequence_from_alphabets('a', 'z'), peg_sequence_from_alphabets('A', 'Z')
        })
    ));

    std::string ident_cont = "IdentCont";
    rules.insert(std::make_pair(
        ident_cont, std::vector<PEG::ExpressionPtr>(
                        {PEG::Expressions::NonTerminalExpression::instance(ident_start), peg_sequence_from_digits(rules, 0, 9)}
                    )
    ));

    std::string identifier = "Identifier";
    rules.insert(std::make_pair(
        identifier, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                        PEG::Expressions::SequenceExpression::instance(
                            PEG::Expressions::NonTerminalExpression::instance(ident_start),
                            PEG::Expressions::RepeatExpression::instance(PEG::Expressions::NonTerminalExpression::instance(ident_cont))
                        ),
                        PEG::Expressions::NonTerminalExpression::instance(spacing)
                    )})
    ));

    // Primary <- Identifier !LEFTARROW
    //         /  OPEN Expression CLOSE
    //         /  Literal / Class / DOT
    std::string primary = "Primary";
    std::string expression = "Expression";
    rules.insert(std::make_pair(
        primary,
        std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::SequenceExpression::instance(
                 PEG::Expressions::NonTerminalExpression::instance(identifier),
                 PEG::Expressions::NotExpression::instance(PEG::Expressions::NonTerminalExpression::instance(left_arrow))
             ),
             PEG::Expressions::SequenceExpression::instance(
                 PEG::Expressions::SequenceExpression::instance(
                     PEG::Expressions::NonTerminalExpression::instance(open), PEG::Expressions::NonTerminalExpression::instance(expression)
                 ),
                 PEG::Expressions::NonTerminalExpression::instance(close)
             ),
             PEG::Expressions::NonTerminalExpression::instance(literal), PEG::Expressions::NonTerminalExpression::instance(class_),
             PEG::Expressions::NonTerminalExpression::instance(dot)}
        )
    ));

    // Suffix <- Primary (QUESTION / STAR / PLUS)?
    // rewritten as
    // Suffix <- Primary (SuffixExtra)?
    // SuffixExtra <- QUESTION / STAR / PLUS
    std::string suffix = "Suffix";
    std::string suffix_extra = "SuffixExtra";
    rules.insert(std::make_pair(
        suffix, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::NonTerminalExpression::instance(primary),
                    PEG::Expressions::OptionExpression::instance(PEG::Expressions::NonTerminalExpression::instance(suffix_extra))
                )})
    ));
    rules.insert(std::make_pair(
        suffix_extra, std::vector<PEG::ExpressionPtr>(
                          {PEG::Expressions::NonTerminalExpression::instance(question), PEG::Expressions::NonTerminalExpression::instance(star),
                           PEG::Expressions::NonTerminalExpression::instance(plus)}
                      )
    ));

    // Prefix <- (AND / NOT)? Suffix
    // rewritten as
    // Prefix <- (PrefixExtra)? Suffix
    // PrefixExtra <- AND / NOT
    std::string prefix = "Prefix";
    std::string prefix_extra = "PrefixExtra";
    rules.insert(std::make_pair(
        prefix, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::OptionExpression::instance(PEG::Expressions::NonTerminalExpression::instance(prefix_extra)),
                    PEG::Expressions::NonTerminalExpression::instance(suffix)
                )})
    ));
    rules.insert(std::make_pair(
        prefix_extra, std::vector<PEG::ExpressionPtr>(
                          {PEG::Expressions::NonTerminalExpression::instance(and_), PEG::Expressions::NonTerminalExpression::instance(not_)}
                      )
    ));

    // Sequence <- Prefix*
    std::string sequence = "Sequence";
    rules.insert(std::make_pair(
        sequence,
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::RepeatExpression::instance(PEG::Expressions::NonTerminalExpression::instance(prefix))})
    ));

    // Expression <- Sequence (SLASH Sequence)*
    // std::string expression = "Expression";
    rules.insert(std::make_pair(
        expression, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                        PEG::Expressions::NonTerminalExpression::instance(sequence),
                        PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                            PEG::Expressions::NonTerminalExpression::instance(slash), PEG::Expressions::NonTerminalExpression::instance(sequence)
                        ))
                    )})
    ));

    // Definition <- Identifier LEFTARROW Expression
    std::string definition = "Definition";
    rules.insert(std::make_pair(
        definition,
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::SequenceExpression::instance(
                PEG::Expressions::NonTerminalExpression::instance(identifier), PEG::Expressions::NonTerminalExpression::instance(left_arrow)
            ),
            PEG::Expressions::NonTerminalExpression::instance(expression)
        )})
    ));

    // Grammar <- Spacing Definition+ EndOfFile
    std::string grammar = "Grammar";
    rules.insert(std::make_pair(
        grammar, std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                     PEG::Expressions::SequenceExpression::instance(
                         PEG::Expressions::NonTerminalExpression::instance(spacing),
                         PEG::Expressions::PlusExpression::instance(PEG::Expressions::NonTerminalExpression::instance(definition))
            ),
            PEG::Expressions::NonTerminalExpression::instance(end_of_file)
                 )})
    ));

    return PEG::Grammar::instance(grammar, rules); // std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>>
}

PEG::ExpressionPtr
peg_sequence_from_list(std::vector<std::string> chars) {
    if (chars.size() < 2) {
        // makes sense I'd say........ (not really)
        return PEG::Expressions::EmptyExpression::instance();
    }
    const auto back_expr_ptr = PEG::Expressions::TerminalExpression::instance(chars.back());
    chars.pop_back();
    PEG::ExpressionPtr recursive_seq =
        PEG::Expressions::SequenceExpression::instance(PEG::Expressions::TerminalExpression::instance(chars.back()), back_expr_ptr);
    chars.pop_back();
    while (!chars.empty()) {
        recursive_seq =
            PEG::Expressions::SequenceExpression::instance(PEG::Expressions::TerminalExpression::instance(chars.back()), recursive_seq);
        chars.pop_back();
    }

    return recursive_seq;
}

PEG::ExpressionPtr
peg_sequence_from_digits(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, const int start, const int end
) {
    // peg_sequence_from_digits(0, 2) {
    if (end < start) {
        return PEG::Expressions::EmptyExpression::instance();
    }
    std::string digit = std::string("Digit_") + std::to_string(start) + "_" + std::to_string(end);
    auto w = std::vector<PEG::ExpressionPtr>({PEG::Expressions::TerminalExpression::instance(std::to_string(start))});
    for (int i = start + 1; i <= end; ++i) {
        w.push_back(PEG::Expressions::TerminalExpression::instance(std::to_string(i)));
    }

    rules.insert(std::make_pair(digit, w));

    return PEG::Expressions::NonTerminalExpression::instance(digit);
}

// peg_sequence_from_alphabets('A', 'Z')
PEG::ExpressionPtr
peg_non_terminal_from_alphabets(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, const char start, const char end
) {
    if (end < start) {
        return PEG::Expressions::EmptyExpression::instance();
    }

    // std::to_string(start) not the same as std::string(1, start)
    std::string non_terminal_char = "Alphabet_" + std::string(1, start) + "_" + end;
    auto x = std::vector<PEG::ExpressionPtr>({PEG::Expressions::TerminalExpression::instance(std::string(1, start))});
    for (char i = start + 1; i <= end; ++i) {
        x.push_back(PEG::Expressions::TerminalExpression::instance(std::string(1, i)));
    }
    rules.insert(std::make_pair(non_terminal_char, x));

    return PEG::Expressions::NonTerminalExpression::instance(non_terminal_char);
}

int
main() {
    std::cout << "hello, world!" << std::endl;

    auto grammar = make_grammar();
    auto parser = PEG::Parser(grammar);
    const std::string input;
    auto plain_parse_result_factory = PEG::ParseTree::PlainParseTreeFactory();
    const auto parseResult = parser.parse(input);
    parseResult->output();
    std::cout << std::endl << "input: " << input << std::endl;

    assert(parseResult->isFullSuccess() == false);
    return 0;
}