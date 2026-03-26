cmake_minimum_required(VERSION 3.23)

#include_directories(${peg-and-tests_SOURCE_DIR}/src)

add_executable(Test01 test_01.cpp)
add_executable(Test02 test_02.cpp)
add_executable(Test03 test_03.cpp)

target_link_libraries(Test01 PEG)
target_link_libraries(Test02 PEG)
target_link_libraries(Test03 PEG)

add_test(NAME test_repeat COMMAND Test01)
add_test(NAME test_not COMMAND Test02)
add_test(NAME test_sequence COMMAND Test03)

add_executable(Test04 test_04.cpp)
target_link_libraries(Test04 PEG)
add_test(NAME test_multi_nonterm COMMAND Test04)

add_executable(Test05 test_05.cpp)
target_link_libraries(Test05 PEG)
add_test(NAME test_nonterminal COMMAND Test05)

add_executable(Test06 test_06.cpp)
target_link_libraries(Test06 PEG)
add_test(NAME test_terminal COMMAND Test06)

add_executable(Test07 test_07.cpp)
target_link_libraries(Test07 PEG)
add_test(NAME test_bootstrap COMMAND Test07)

add_executable(Test08 test_08.cpp)
target_link_libraries(Test08 PEG)
add_test(NAME pragma COMMAND Test08)

add_executable(Test09 test_09.cpp)
target_link_libraries(Test09 PEG)
add_test(NAME calculator COMMAND Test09)

add_executable(Test10 test_10.cpp)
target_link_libraries(Test10 PEG)
add_test(NAME calc_grammar COMMAND Test10)

add_executable(Test11 libPEG.h test_11.cpp)
target_link_libraries(Test11 PEG)
add_test(NAME pragma.new COMMAND Test11)

add_executable(Test12 test_12.cpp)
target_link_libraries(Test12 PEG)
add_test(NAME static_link_header_not_really COMMAND Test12)

add_executable(Test13 test_13.cpp)
target_link_libraries(Test13 PEG)
add_test(NAME AZ_strings COMMAND Test13)

add_executable(Test14 test_14.cpp)
target_link_libraries(Test14 PEG)
add_test(NAME az_strings COMMAND Test14)

add_executable(Test15 test_15.cpp)
target_link_libraries(Test15 PEG)
add_test(NAME 09_strings COMMAND Test15)

add_executable(Test16 test_16.cpp)
target_link_libraries(Test16 PEG)
add_test(NAME peg_grammar COMMAND Test16)

add_executable(Test18 test_18.cpp)
target_link_libraries(Test18 PEG)
add_test(NAME peg_another COMMAND Test18)

add_executable(Test19 test_19.cpp)
target_link_libraries(Test19 PEG)
add_test(NAME calc.peg COMMAND Test19)

add_executable(Test20 test_20.cpp)
target_link_libraries(Test20 PEG)
add_test(
        NAME input_file
        COMMAND Test20 --grammar ./test/calc.peg --input ./test/input.calc
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(Test22 test_22.cpp)
target_link_libraries(Test22 PEG)
add_test(NAME direct_left_recursion COMMAND Test22)

add_executable(Test23 test_23.cpp)
target_link_libraries(Test23 PEG)
add_test(NAME another_left COMMAND Test23)

add_executable(Test24 test_24.cpp)
target_link_libraries(Test24 PEG)
add_test(NAME mutual_left_recursion COMMAND Test24)

add_executable(Test25 test_25.cpp)
target_link_libraries(Test25 PEG)
add_test(NAME precedence COMMAND Test25)

add_executable(Test26 test_26.cpp)
target_link_libraries(Test26 PEG)
add_test(NAME precedence.bootstrap COMMAND Test26)

add_executable(Test27 test_20.cpp)
target_link_libraries(Test27 PEG)
add_test(NAME castor COMMAND Test27 --grammar=${CMAKE_CURRENT_SOURCE_DIR}/pragma/pragma.grammar
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(Test28 test_28.cpp)
target_link_libraries(Test28 PEG)
add_test(NAME open-square-bracket_exclamation_minus_close-square-bracket COMMAND Test28)

add_executable(Test29 test_29.cpp)
target_link_libraries(Test29 PEG)
add_test(NAME comment-should-end-with-new-line COMMAND Test29)

add_executable(Test30 test_30.cpp)
target_link_libraries(Test30 PEG)
add_test(NAME range-charA_minus_charB-charC_minus_charD-charY-star COMMAND Test30)

add_executable(Test31 test_31.cpp)
target_link_libraries(Test31 PEG)
add_test(NAME range-char-minus-char-star-plus COMMAND Test31)

add_executable(Test32 test_32.cpp)
target_link_libraries(Test32 PEG)
add_test(NAME escape-backslash COMMAND Test32)

add_executable(Test33 test_33.cpp)
target_link_libraries(Test33 PEG)
add_test(NAME castor-input01 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input01.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input02 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input02.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input03 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input03.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input04 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input04.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input05 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input05.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input06 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input06.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input07 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input07.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input08 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input08.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input09 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input09.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input10 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input10.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input11 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input11.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input12 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input12.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input13 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input13.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input14 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input14.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input15 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input15.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input16 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input16.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input17 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input17.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME castor-input18 COMMAND Test33 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input18.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(Test34 test_34.cpp)
target_link_libraries(Test34 PEG)
add_test(NAME parse-seqn-whitespace COMMAND Test34)

add_executable(Test35 test_35.cpp)
target_link_libraries(Test35 PEG)
add_test(NAME not-seqn_keyword_not-not-name COMMAND Test35)

add_executable(Test36 test_36.cpp)
target_link_libraries(Test36 PEG)
add_test(NAME square-bracket-quote-square-bracket COMMAND Test36)

add_executable(Test37 test_37.cpp)
target_link_libraries(Test37 PEG)
add_test(NAME test-is-nt-lr COMMAND Test37)

add_executable(Test38 test_38.cpp)
target_link_libraries(Test38 PEG)
add_test(NAME castor-detect-left-recursive COMMAND Test38)

add_executable(Test39 test_39.cpp)
target_link_libraries(Test39 PEG)
add_test(NAME left-recursive-nullable-non-terminal COMMAND Test39)

add_executable(Test40 test_40.cpp)
target_link_libraries(Test40 PEG)
add_test(NAME castor-operator-precedence-levels COMMAND Test40)

add_executable(Test41 test_41.cpp)
target_link_libraries(Test41 PEG)
add_test(NAME preceded-castor-input01 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input01.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input02 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input02.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input03 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input03.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input04 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input04.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input05 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input05.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input06 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input06.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input07 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input07.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input08 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input08.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input09 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input09.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input10 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input10.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input11 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input11.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input12 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input12.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input13 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input13.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input14 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input14.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input15 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input15.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input16 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input16.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input17 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input17.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME preceded-castor-input18 COMMAND Test41 --input=${CMAKE_CURRENT_SOURCE_DIR}/pragma/input18.txt WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(Test42 test_42.cpp)
target_link_libraries(Test42 PEG)
add_test(NAME castor-actions COMMAND Test42 --output=${CMAKE_CURRENT_SOURCE_DIR}/pragma/test42.goose)

add_executable(Test43 test_43.cpp)
target_link_libraries(Test43 PEG)
add_test(NAME json-meta-bench-test COMMAND Test43 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/test.json)
add_test(NAME json-meta-bench-data COMMAND Test43 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/data.json)
add_test(NAME json-meta-bench-apache COMMAND Test43 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/apache_builds.json)

add_executable(Test44 test_44.cpp)
target_link_libraries(Test44 PEG)
add_test(NAME json-parsec-bench-test COMMAND Test44 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/test.json)
add_test(NAME json-parsec-bench-data COMMAND Test44 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/data.json)
add_test(NAME json-parsec-bench-apache COMMAND Test44 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/apache_builds.json)

add_executable(Test45 test_45.cpp)
target_link_libraries(Test45 PEG)
add_test(NAME json-meta-ws-test COMMAND Test45 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/test.json)
add_test(NAME json-meta-ws-data COMMAND Test45 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/data.json)
add_test(NAME json-meta-ws-apache COMMAND Test45 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/apache_builds.json)

add_executable(Test46 test_46.cpp)
target_link_libraries(Test46 PEG)
add_test(NAME json-parsec-ws-bench-test COMMAND Test46 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/test.json)
add_test(NAME json-parsec-ws-bench-data COMMAND Test46 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/data.json)
add_test(NAME json-parsec-ws-bench-apache COMMAND Test46 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/apache_builds.json)

add_executable(Test47 test_47.cpp)
target_link_libraries(Test47 PEG)

add_executable(Test48 test_48.cpp)
target_link_libraries(Test48 PEG)

add_executable(Test49 test_49.cpp)
target_link_libraries(Test49 PEG)

add_executable(Test50 test_50.cpp)
target_link_libraries(Test50 PEG)

# members <- member (',' member)*
add_test(NAME json-meta-bench-canada COMMAND Test43 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/canada.json)
add_test(NAME json-parsec-bench-canada COMMAND Test44 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/canada.json)
add_test(NAME json-meta-ws-canada COMMAND Test45 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/canada.json)
add_test(NAME json-parsec-ws-bench-canada COMMAND Test46 --input=${CMAKE_CURRENT_SOURCE_DIR}/json/canada.json)
