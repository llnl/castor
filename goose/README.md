# Goose-PEG
*yet another Parsing Expression Grammars library for C++17*

## Building

This project uses `cmake` for building. To build, make use of the [`build.sh`](./build.sh) file:

```language=shell
➜ ~/pegs (main) ✗ ./build.sh
```

`cmake` locates the available compilers, build tools, debugger and linkers to compile the source down to a static linked library for C++11 and also create executable tests that check the different functionality available.

It also produces an executable file at the root directory, named "goose", which you can run: 

```language=shell
./goose --grammar <grammar-file> [--input=<input-file>]
```

Please take a look at the tests to get a picture of the different ways to interact with the library and the features it supports

Mainly, we have
- Support for much of the initial Bryan Ford's PEG Grammar, and a bit more
- Declarative and Parser Combinator style construction of PEG Grammars
- Semantic Actions, that perform further computation on the parse outputs
- Left recursive Grammar support, direct or indirect
- Precedence, Left/Right associativity overrides for left-recursive grammars
- A bootstrap parser to parse the PEG syntax

## Source structure

Source files are provided with a (hopefully) relatively straightforward folder hierarchy in `./src/PEG` with the core classes defined in [`BasicTypes.h`](./src/PEG/BasicTypes.h)

Tests are available in `./test`

Additionally, a formal methods style analysis of PEGs in Coq/Rocq Theorem Prover can be found in `./theories`. This is still WIP.

## Example

```c++

// Goose helps create parsers based on parsing expression grammars
// Where each part is built using expressions.

// A Terminal Expression to match strings
PEG::ExpressionPtr
term(const std::string &t) { 
    return PEG::Expressions::TerminalExpression::instance(t);
}
// A Non-Terminal Expression which will have production rules
PEG::ExpressionPtr
non_term(const std::string &nt) { 
    return PEG::Expressions::NonTerminalExpression::instance(nt);
}
// A Sequence of Expressions to consecutively parse things
PEG::ExpressionPtr
seq(PEG::ExpressionPtr expr1, PEG::ExpressionPtr expr2) { 
    return PEG::Expressions::SequenceExpression::instance(std::move(expr1), std::move(expr2));
}
// Empty Expression
PEG::ExpressionPtr
empty() { 
    return PEG::Expressions::EmptyExpression::instance();
}
// Sequence of multiple expressions
PEG::ExpressionPtr
seq_n(const std::vector<PEG::ExpressionPtr> &list_expr) { 
    return PEG::Expressions::SeqNExpression::instance(list_expr);
}

// You can define a Parser for a Grammar declaratively.
const auto example_grammar = PEG::Grammar::instance("integer", {
    {"integer",{
        seq_n({non_term("one_nine"),non_term("digits"),}),
        non_term("digit")
    }},
    {"digits", {
        PEG::Expressions::PlusExpression::instance(non_term("digit")),
    }},
    {"digit", {
        term("0"),
        non_term("one_nine")}},
    {"one_nine", {
         term("1"),
         term("2"),
         term("3"),
         term("4"),
         term("5"),
         term("6"),
         term("7"),
         term("8"),
         term("9"),
    }}
}); // ref: test_46.cpp

// Or using the inbuilt parser generator.
const std::string input_grammar = R"N(
integer <- [1-9] digits / digit
digit <- '0' / [1-9]
digits <- digit+
)N"; // ref: test_47.cpp

const auto parser = PEG::Parser::from_str(input_grammar);

// You can also provide it with custom actions.
const std::string simple_grammar = PEG::Parser::from_str(R"N(
    Expr <- Num '+' Expr @sum
          / Num '-' Expr @minus
          / Num
    Num <- '0' @num0 / '1' @num1
)N", calc_action_map);

// where the actions are defined beforehand using a mapping from non-terminals
// to factory classes: these factories can transform the parsed structure into an
// output value.
// auto calc_action_map = { 
     {"sum", std::make_shared<OpFactory>(OpFactory::PLUS)} ... };
// ref: test_19.cpp

// And it even supports left-recursive grammars.
const auto lrgram = R"N(
    L <- P'.x' / 'x'
    P <- P'(n)' / L
)N"; // ref: test_24.cpp

// Along with operator precedence and left/right-associativity.
const std::string input_grammar = R"N(
    E <- E '+' E^2 / E^2 '*' E^2 / 'n'
)N";
// where:
//  n + n + n  => (n + n) + n
//  n * n * n  => n * ( n * n )
//  n + n * n  => n + ( n * n )
//  n * n + n  => ( n * n ) + n
// ref: test_26.cpp

// To parse against an input, we would do
const std::string input_0 = "n+n+n";
auto const result_0 = parser->parse(input_0);
if(result_0->is_success()) {
    std::cout << "successful parse ";
    if(result_0->is_full_success()){
        std::cout << "parsed till end of input";
    }
} else {
    std::cout << "parse failed :(";
}

```

```md
# And ofc, you can write the syntax of PEGs as a PEG
Grammar <- Spacing Definition+ EndOfFile
Definition <- Identifier LEFTARROW Expression

Expression <- Sequence (SLASH Sequence)*
Sequence <- Prefix+ (ACTION Identifier)?
Prefix <- (Prefix_Extra)? Suffix
Prefix_Extra <- AND / NOT
Suffix <- Primary (Suffix0)?
Suffix0 <- QUESTION / STAR / PLUS

Primary <- Identifier ('^'[0-9]([0-9])?)? !LEFTARROW Spacing
        / OPEN Sequence CLOSE
        / Literal
        / Class
        / DOT

Identifier <- IdentStart IdentCont* Spacing
IdentStart <- [A-Za-z_]
IdentCont <- IdentStart
         / [0-9]

Literal <- ['] (!['] Char)+ ['] Spacing
        /   ["] (!["] Char)+ ["] Spacing
Class <- '[' (!']' Range)+ ']' Spacing
Range <- Char '-' !']' Char / Char
Char <- '\' [nrt\[\]\\]
        / .

LEFTARROW <- '<-' Spacing
SLASH <- '/' Spacing
ACTION <- '@' Spacing
AND <- '&' Spacing
NOT <- '!' Spacing
QUESTION <- '?' Spacing
STAR <- '*' Spacing
PLUS <- "+" Spacing
OPEN <- '(' Spacing
CLOSE <- ')' Spacing
DOT <- '.' Spacing

Spacing <- (Spacing_Extra)*
Spacing_Extra <- Space / Comment
Space <- ' ' / '\t' / EndOfLine
Comment <- '#' (!EndOfLine .)* EndOfLine
EndOfLine <- '\r\n' / '\n' / '\r'
EndOfFile <- !.
```


## Expressions

### Types of Parsing Expressions

| Expression  | Example                         | Info                                                                      |
|-------------|---------------------------------|---------------------------------------------------------------------------|
| Terminal    | 'goose'                         |                                                                           |
| And         | &('goose')                      | doesn't consume any input                                                 |
| Dot         | .                               | consumes any non whitespace character                                     |
| Empty       | ""                              | not exposed unless combinatory, useful as a failure case to repeat/option |
| NonTerminal | GOOSE <- 'goose' <br/>/ 'geese' | must have valid alternations to expressions in the grammar                |
| Not         | '[' !([\]]) . ']'               | doesn't consume any input                                                 |
| Option      | ('goose')?                      | might exist, if not that's ok it still suceeds as empty                   |
| Plus        | (GOOSE)+                        | an expression parsed one or more times                                    |
| Repeat      | (GOOSE)*                        | an expression parsed zero or more times. If none, it suceeds as empty     |
| Sequence    | 'hello' GOOSE                   | internally, a pair of expressions                                         |
| SeqN        | 'hello' GOOSE '!'               | a more general sequence, takes a list of expressions                      |

### Parsing Expressions outputs

When any of the above expression parses successfully, it produces a tuple and calls a semantic action to produce a value:

  `Value: (label: string, position: int, value: string, children: [Value])` (from [ValueFactoryInterface](./src/PEG/Value))

i.e., (String, Position) -> Expression ---(s, p, v, c)---> (Value)

Expressions that are composed of other Expressions, wrap the output values of the internal Expressions into a new value

The default semantic action is [PlainParseTree](./src/PEG/ParseTree), which creates a parse tree structure that mimics the input tuple and can be output as a simple json
Semantics Actions are created through inheriting the ValueFactoryInterface class

| Expression   | Example                                         | Label        | Position                              | Value                    | Children                                               |
|--------------|-------------------------------------------------|--------------|---------------------------------------|--------------------------|--------------------------------------------------------|
| Terminal     | 'goose'                                         | terminal     | -1                                    | 'goose'                  | {}                                                     |
| And          | &('goose')                                      | and          | -1                                    | ''                       | {}                                                     |
| Dot          | .                                               | dot          | -1                                    | 'c' (a single character) | {}                                                     |
| Empty        | ""                                              | empty        | -1                                    | ''                       | {}                                                     |
| NonTerminal  | GOOSE <- 'goose' #rule-0<br/> / 'geese' #rule-1 | non_terminal | {0, 1} (which alternation was chosen) | ''                       | output of choice: {Value1} or {Value2}                 |
| Not          | '[' !([\]]) . ']'                               | not          | -1                                    | ''                       | {}                                                     |
| Option_Empty | ('goose')? # internal expression failed         | empty        | -1                                    | ''                       | {}                                                     |
| Option_Parse | (expr)? # internal expression succeeded         | option       | -1                                    | ''                       | {Value_of_expr}                                        |
| Plus         | (expr)+ # parses one or more expr               | plus         | -1                                    | ''                       | {ValueExpr, [ValueExpr, ValueExpr,...]}                |                        
| Repeat_Empty | (GOOSE)* # internal expression failed           | empty        | -1                                    | ''                       | {}                                                     |
| Repeat_Parse | (GOOSE)* # internal expression succeeded        | repeat       | -1                                    | ''                       | {ValueExpr, [ValueExpr, ...]}                          |
| Sequence     | 'hello' GOOSE # Seq(Expr0, Expr1)               | sequence     | -1                                    | ''                       | {0: Value_Expr0, 1: Value_Expr1}                       | 
| SeqN         | 'hello' GOOSE '!' # SeqN(Expr0,... , Expr_n)    | SeqN         | -1                                    | ''                       | {0: Value_Expr0, 1: Value_Expr1,... , n: Value_Expr_n} |

Inspecting the output json, or having a debugger at breakpoints of entry to ValueFactoryInterface subclasses usually gives a sense to how the output is structured (we suggest the latter using CLion)
