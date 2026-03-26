cmake_minimum_required(VERSION 3.23)

# cmake file that defines all our c++ source and header files
# defined inside PEG folder into a single library which
# can be called by top level cmake builds or installs to
# link as a library in their cmake executable or tests
#
# probably cannot be built by itself since it doesn't start
# with a project() at the top, the idea is to link it elsewhere

add_library(PEG
        BasicTypes.h
        Value/ValueInterface.h
        Value/ValueFactoryInterface.h
        Value/NothingFactoryInterface.h
        ParseTree/PlainParseTree.h
        ParseTree/PlainParseTree.cpp
        ParseTree/PlainParseTreeFactory.h
        ParseTree/PlainParseTreeFactory.cpp
        ParseResult.h
        ParseResultType/FailParseResult.h
        ParseResultType/FailParseResult.cpp
        ParseResultType/SuccessParseResult.h
        ParseResultType/SuccessParseResult.cpp
        Expression.h
        Expressions/TerminalExpression.h
        Expressions/TerminalExpression.cpp
        Expressions/NonTerminalExpression.h
        Expressions/NonTerminalExpression.cpp
        Expressions/SequenceExpression.h
        Expressions/SequenceExpression.cpp
        Expressions/SeqNExpression.h
        Expressions/SeqNExpression.cpp
        Expressions/NotExpression.h
        Expressions/NotExpression.cpp
        Expressions/RepeatExpression.h
        Expressions/RepeatExpression.cpp
        Expressions/DotExpression.h
        Expressions/DotExpression.cpp
        Expressions/EmptyExpression.h
        Expressions/EmptyExpression.cpp
        Expressions/OptionExpression.h
        Expressions/OptionExpression.cpp
        Expressions/PlusExpression.h
        Expressions/PlusExpression.cpp
        Expressions/AndExpression.h
        Expressions/AndExpression.cpp
        Grammar.h
        Grammar.cpp
        Bootstrap/BootstrapParser.h
        Bootstrap/BootstrapParser.cpp
        Parser.h
        Parser.cpp
        LoggerFactory.h
)

#target_include_directories(PEG INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

message("creating libpeg.so in ${CMAKE_CURRENT_SOURCE_DIR}")

#set_target_properties(libpeg PROPERTIES VERSION ${PROJECT_VERSION})

#set_target_properties(libpeg PROPERTIES PUBLIC_HEADER BasicTypes.h)

target_include_directories(PEG PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)


install(TARGETS PEG)

install(
        #LIBRARY DESTINATION LIB
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include FILES_MATCHING PATTERN "*.h"
)

#install(FILE BasicTypes.h DESTINATION include #FILES_MATCHING PATTERN "*.h")

# set_target_properties(PEG PROPERTIES ENABLE_EXPORTS 1) https://stackoverflow.com/a/65005385
