cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CTEST_TIMEOUT)
	set(CTEST_TIMEOUT 5)
endif()

if (NOT CTEST_NEGATIVE_TIMEOUT)
	set(CTEST_NEGATIVE_TIMEOUT 5)
endif()

project(castor LANGUAGES CXX DESCRIPTION "Formally verifying idiomatic C++")

include(CTest)

add_subdirectory(goose/src/PEG)

execute_process(
	COMMAND rose-config prefix
	OUTPUT_VARIABLE ROSE_HOME
	OUTPUT_STRIP_TRAILING_WHITESPACE
	RESULT_VARIABLE ROSE_CONFIG_RESULT
	ERROR_VARIABLE ROSE_CONFIG_ERROR)

if(NOT ROSE_CONFIG_RESULT EQUAL 0)
	message(FATAL_ERROR "Could not read ROSE information from 'rose-config'.\n"
			    "Are you sure 'rose-config' is visible in your PATH?")
endif()

find_library(ROSE_LIB NAMES rose PATHS ${ROSE_HOME}/lib REQUIRED)
find_package(Boost 1.66 COMPONENTS graph filesystem REQUIRED)
find_program(WHY3 why3 REQUIRED)
find_program(WHY3FIND why3find REQUIRED)
include_directories(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/pegs/src)

if(NOT CODETHORN_INCLUDE_DIR)
	set(CODETHORN_INCLUDE_DIR ${ROSE_HOME}/include/rose/codethorn)
endif()

if(NOT CODETHORN_LIB_DIR)
	set(CODETHORN_LIB_DIR ${ROSE_HOME}/lib)
endif()

if(NOT SMOKE_TESTS OR SMOKE_TESTS STREQUAL "false")
	set(SMOKE_TESTS "")
elseif(SMOKE_TESTS STREQUAL "true")
	set(SMOKE_TESTS --smoke-tests)
else()
	message(FATAL_ERROR "Variable SMOKE_TESTS must be either 'true' or 'false'")
endif()

if(NOT EXISTS ${CODETHORN_INCLUDE_DIR}/NormalizationCxx.h)
	message(FATAL_ERROR "Could not find required CodeThorn headers. Did you forget to set CODETHORN_INCLUDE_DIR?")
endif()

find_library(CODETHORN_LIB NAMES codethorn PATHS ${CODETHORN_LIB_DIR} REQUIRED)

message(STATUS "ROSE home found at: ${ROSE_HOME}")
message(STATUS "Why3 executable found at: ${WHY3}")
message(STATUS "Why3find executable found at: ${WHY3FIND}")
message(STATUS "CodeThorn headers found at: ${CODETHORN_INCLUDE_DIR}")

if (NOT SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC OR SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC STREQUAL "false")
	set(DYNAMIC "")
else()
	set(DYNAMIC -DSUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC)
	message(WARNING "Dynamic memory allocation in Castor is unsound! You may prove something false!")
endif()

add_executable(${PROJECT_NAME}
	src/Castor/main.cxx
	src/Castor/irgenerator.cxx
	src/Castor/ir.cxx
	src/Castor/whyml.cxx
	src/Castor/parser.cxx
	src/Castor/types.cxx
	src/Castor/fill_types.cxx
	src/Castor/irgen_aux.cxx
	src/Castor/symbol_table.cxx
	src/Castor/validate_ir.cxx
	src/Castor/whygenerator.cxx
	src/Castor/whygen_aux.cxx
	src/Castor/offset_table.cxx
	src/Castor/vc_generator.cxx
	src/Castor/set_metadata.cxx
	src/Castor/order_functions.cxx
	src/Castor/normalization.cxx
	src/Castor/function_table.cxx
	src/Castor/debug_info.cxx
	src/Castor/class_table.cxx)

set(REPLACEMENT_INCLUDES ${CMAKE_SOURCE_DIR}/src/headers)

target_compile_options(${PROJECT_NAME} PRIVATE
	-I${ROSE_HOME}/include/rose
	-I${ROSE_HOME}/include
	-I${CODETHORN_INCLUDE_DIR}
	-O0 -g -pthread -w -Wall -Wextra -Werror
	-Wno-misleading-indentation
	-DSOURCE_DIR="${CMAKE_SOURCE_DIR}/src"
	${DYNAMIC}
	-DWHY3="${WHY3}"
	-DWHY3FIND="${WHY3FIND}"
	-DREPLACEMENT_INCLUDES="${REPLACEMENT_INCLUDES}")

target_link_libraries(${PROJECT_NAME} PRIVATE
	-pthread -fopenmp -lstdc++fs
	${ROSE_LIB}
	${Boost_LIBRARIES}
	${CODETHORN_LIB}
	PEG)

set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS ${PROJECT_NAME})

set(TEST_DIR            ${CMAKE_SOURCE_DIR}/tests)
set(SHOULD_EXCEPT_DIR   ${TEST_DIR}/should_except)
set(SHOULDNT_VERIFY_DIR ${TEST_DIR}/shouldnt_verify)

add_test(NAME arith_model  COMMAND why3find prove  -t ${CTEST_TIMEOUT} --root ${CMAKE_SOURCE_DIR}/src/theories ArithmeticModel.mlw)
add_test(NAME arith_sound  COMMAND why3find prove  -t ${CTEST_TIMEOUT} --root ${CMAKE_SOURCE_DIR}/src/theories ArithmeticModel-Witness.mlw)
add_test(NAME memory_model COMMAND why3find prove  -t ${CTEST_TIMEOUT} --root ${CMAKE_SOURCE_DIR}/src/theories MemoryModel.mlw)
add_test(NAME memory_lemma COMMAND why3find prove  -t ${CTEST_TIMEOUT} --root ${CMAKE_SOURCE_DIR}/src/theories Lemmas.mlw)
add_test(NAME bit_lemmas   COMMAND why3find prove  -t ${CTEST_TIMEOUT} --root ${CMAKE_SOURCE_DIR}/src/theories BitVector.mlw)

add_test(NAME add          COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/add.cpp)
add_test(NAME and_or       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/and_or.cpp)
add_test(NAME annotate     COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/annotate.cpp)
add_test(NAME array        COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/array.cpp --additional-lemmas)
add_test(NAME array_struct COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/array_struct.cpp)
add_test(NAME assume       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/assume.cpp)
add_test(NAME binops       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/binops.cpp)
add_test(NAME decltype     COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/decltype.cpp)
if(DYNAMIC)
add_test(NAME dynamic      COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/dynamic.cpp --additional-lemmas)
endif()
add_test(NAME func         COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/func.cpp)
add_test(NAME global_var   COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/global_variable.cpp)
add_test(NAME if           COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/if.cpp)
add_test(NAME labels       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/labels.cpp)
add_test(NAME loops        COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/loops.cpp)
add_test(NAME methods      COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/methods.cpp)
add_test(NAME misc         COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/misc.cpp)
add_test(NAME modifiers    COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/modifiers.cpp)
add_test(NAME namespace    COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/namespace.cpp)
add_test(NAME overload     COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/overload.cpp)
add_test(NAME ptr          COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/ptr.cpp)
add_test(NAME references   COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/references.cpp)
add_test(NAME sizeof       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/sizeof.cpp)
add_test(NAME static       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/static.cpp)
add_test(NAME struct       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/struct.cpp --additional-lemmas)
add_test(NAME switch       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/switch.cpp)
add_test(NAME template     COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/template.cpp)
add_test(NAME templ_class  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/template_class.cpp)
add_test(NAME templ_meta   COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/template_meta.cpp)
add_test(NAME cloning      COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/variable_cloning.cpp)
add_test(NAME vcs          COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/vcs.cpp)
add_test(NAME class_member COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/class_members.cpp --additional-lemmas)
add_test(NAME limits       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/limits.cpp)
add_test(NAME recursive    COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/recursive.cpp)
add_test(NAME big_rvalues  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/big_rvalues.cpp)
add_test(NAME friend_class COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/friend.cpp)
add_test(NAME mutual_rec   COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/mutually_recursive.cpp)
add_test(NAME multiple_ret COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/multiple_returns.cpp)
add_test(NAME destructor   COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/destructors.cpp)
add_test(NAME ctor_init    COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/ctor_initializer.cpp)
add_test(NAME enum         COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/enum.cpp)
add_test(NAME cast_ovrload COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/class_cast.cpp)
add_test(NAME matrlization COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/materialization.cpp)
add_test(NAME default_delt COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/default_delete.cpp)
add_test(NAME inheritance  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/inheritance.cpp)
add_test(NAME nullptr      COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/nullptr.cpp)
add_test(NAME static_assrt COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/static_assert.cpp)
add_test(NAME scope_resolv COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/scope_resolution.cpp)
add_test(NAME const        COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/const.cpp)
add_test(NAME lemmas       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/lemmas.cpp)
add_test(NAME axioms       COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${TEST_DIR}/axioms.cpp) # this test passes due to inconsistent axiom
add_test(NAME constexpr    COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/constexpr.cpp --standard=c++14) # this test only passes on C++14
add_test(NAME valid        COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/valid.cpp)
add_test(NAME no_body      COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/no_body.cpp)
add_test(NAME byte         COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/byte.cpp --bitwise-lemmas)
add_test(NAME rangefor     COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/rangefor.cpp)
add_test(NAME no_overflow  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_TIMEOUT} ${SMOKE_TESTS} ${TEST_DIR}/no_overflow.cpp --no-overflow)

add_test(NAME should_throw_exception1  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except1.cpp)
add_test(NAME should_throw_exception2  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except2.cpp)
add_test(NAME should_throw_exception3  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except3.cpp)
add_test(NAME should_throw_exception4  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except4.cpp)
add_test(NAME should_throw_exception5  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except5.cpp)
add_test(NAME should_throw_exception6  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except6.cpp)
add_test(NAME should_throw_exception7  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except7.cpp)
add_test(NAME should_throw_exception8  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except8.cpp)
add_test(NAME should_throw_exception9  COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except9.cpp)
add_test(NAME should_throw_exception10 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except10.cpp)
add_test(NAME should_throw_exception11 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except11.cpp)
add_test(NAME should_throw_exception12 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except12.cpp)
add_test(NAME should_throw_exception13 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except13.cpp)
add_test(NAME should_throw_exception14 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except14.cpp)
add_test(NAME should_throw_exception15 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except15.cpp)
add_test(NAME should_throw_exception16 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except16.cpp)
add_test(NAME should_throw_exception17 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except17.cpp)
add_test(NAME should_throw_exception18 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except18.cpp)
add_test(NAME should_throw_exception19 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except19.cpp)
add_test(NAME should_throw_exception20 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except20.cpp)
add_test(NAME should_throw_exception21 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except21.cpp)
add_test(NAME should_throw_exception22 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except22.cpp)
add_test(NAME should_throw_exception23 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except23.cpp)
add_test(NAME should_throw_exception24 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except24.cpp)
add_test(NAME should_throw_exception25 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except25.cpp)
add_test(NAME should_throw_exception26 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except26.cpp --standard=c++11)
add_test(NAME should_throw_exception27 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except27.cpp --standard=c++11)
add_test(NAME should_throw_exception28 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except28.cpp)
add_test(NAME should_throw_exception29 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except29.cpp)
add_test(NAME should_throw_exception30 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except30.cpp)
add_test(NAME should_throw_exception31 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except31.cpp)
add_test(NAME should_throw_exception32 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except32.cpp)
add_test(NAME should_throw_exception33 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except33.cpp)
add_test(NAME should_throw_exception34 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except34.cpp)
add_test(NAME should_throw_exception35 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except35.cpp --no-overflow)
add_test(NAME should_throw_exception36 COMMAND ${PROJECT_NAME} parse ${SHOULD_EXCEPT_DIR}/should_except36.cpp --no-overflow)
add_test(NAME should_throw_exception_A COMMAND ${PROJECT_NAME} parse ${TEST_DIR}/add.cpp -std=c++14)
add_test(NAME should_throw_exception_B COMMAND ${PROJECT_NAME} parse ${TEST_DIR}/add.cpp --standard=c++03)
set_tests_properties(should_throw_exception1 should_throw_exception2 should_throw_exception3 should_throw_exception4
			should_throw_exception5 should_throw_exception6 should_throw_exception7	should_throw_exception8
			should_throw_exception9 should_throw_exception10 should_throw_exception11 should_throw_exception12
			should_throw_exception13 should_throw_exception14 should_throw_exception15 should_throw_exception16
			should_throw_exception17 should_throw_exception18 should_throw_exception19 should_throw_exception20
			should_throw_exception21 should_throw_exception22 should_throw_exception23 should_throw_exception24
			should_throw_exception25 should_throw_exception26 should_throw_exception27 should_throw_exception28
			should_throw_exception29 should_throw_exception30 should_throw_exception31 should_throw_exception32
			should_throw_exception33 should_throw_exception34 should_throw_exception35 should_throw_exception36
			should_throw_exception_A should_throw_exception_B PROPERTIES WILL_FAIL TRUE)

add_test(NAME shouldnt_verify1  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify1.cpp)
add_test(NAME shouldnt_verify2  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify2.cpp)
add_test(NAME shouldnt_verify3  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify3.cpp)
add_test(NAME shouldnt_verify4  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify4.cpp)
add_test(NAME shouldnt_verify5  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify5.cpp)
add_test(NAME shouldnt_verify6  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify6.cpp)
add_test(NAME shouldnt_verify7  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify7.cpp)
add_test(NAME shouldnt_verify8  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify8.cpp)
add_test(NAME shouldnt_verify9  COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify9.cpp)
add_test(NAME shouldnt_verify10 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify10.cpp)
add_test(NAME shouldnt_verify11 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify11.cpp)
add_test(NAME shouldnt_verify12 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify12.cpp)
add_test(NAME shouldnt_verify13 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify13.cpp)
add_test(NAME shouldnt_verify14 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify14.cpp)
add_test(NAME shouldnt_verify15 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify15.cpp)
add_test(NAME shouldnt_verify16 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify16.cpp)
add_test(NAME shouldnt_verify17 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify17.cpp)
add_test(NAME shouldnt_verify18 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify18.cpp)
add_test(NAME shouldnt_verify19 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify19.cpp)
add_test(NAME shouldnt_verify20 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify20.cpp)
add_test(NAME shouldnt_verify21 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify21.cpp)
add_test(NAME shouldnt_verify22 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify22.cpp)
add_test(NAME shouldnt_verify23 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify23.cpp)
add_test(NAME shouldnt_verify24 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify24.cpp)
add_test(NAME shouldnt_verify25 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify25.cpp)
add_test(NAME shouldnt_verify26 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify26.cpp)
add_test(NAME shouldnt_verify27 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify27.cpp)
add_test(NAME shouldnt_verify28 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify28.cpp)
add_test(NAME shouldnt_verify29 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify29.cpp)
add_test(NAME shouldnt_verify30 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify30.cpp)
add_test(NAME shouldnt_verify31 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify31.cpp)
add_test(NAME shouldnt_verify32 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify32.cpp)
add_test(NAME shouldnt_verify33 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify33.cpp --smoke-tests) # this test passes on failed smoke test
add_test(NAME shouldnt_verify34 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify34.cpp)
add_test(NAME shouldnt_verify35 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify35.cpp)
add_test(NAME shouldnt_verify36 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify36.cpp)
add_test(NAME shouldnt_verify37 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify37.cpp)
add_test(NAME shouldnt_verify38 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify38.cpp)
add_test(NAME shouldnt_verify39 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify39.cpp)
add_test(NAME shouldnt_verify40 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify40.cpp)
add_test(NAME shouldnt_verify41 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify41.cpp)
add_test(NAME shouldnt_verify42 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify42.cpp)
add_test(NAME shouldnt_verify43 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify43.cpp)
add_test(NAME shouldnt_verify44 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify44.cpp)
add_test(NAME shouldnt_verify45 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify45.cpp)
add_test(NAME shouldnt_verify46 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify46.cpp)
add_test(NAME shouldnt_verify47 COMMAND ${PROJECT_NAME} prove ${DYNAMIC} -t ${CTEST_NEGATIVE_TIMEOUT} ${SHOULDNT_VERIFY_DIR}/shouldnt_verify47.cpp)
set_tests_properties(shouldnt_verify1 shouldnt_verify2 shouldnt_verify3 shouldnt_verify4 shouldnt_verify5 shouldnt_verify6
			shouldnt_verify7 shouldnt_verify8 shouldnt_verify9 shouldnt_verify10 shouldnt_verify11 shouldnt_verify12
			shouldnt_verify13 shouldnt_verify14 shouldnt_verify15 shouldnt_verify16 shouldnt_verify17
			shouldnt_verify18 shouldnt_verify19 shouldnt_verify20 shouldnt_verify21 shouldnt_verify22
			shouldnt_verify23 shouldnt_verify24 shouldnt_verify25 shouldnt_verify26 shouldnt_verify27
			shouldnt_verify28 shouldnt_verify29 shouldnt_verify30 shouldnt_verify31 shouldnt_verify32
			shouldnt_verify33 shouldnt_verify34 shouldnt_verify35 shouldnt_verify36 shouldnt_verify37
			shouldnt_verify38 shouldnt_verify39 shouldnt_verify40 shouldnt_verify41 shouldnt_verify42
			shouldnt_verify43 shouldnt_verify44 shouldnt_verify45 shouldnt_verify46 shouldnt_verify47
			PROPERTIES WILL_FAIL TRUE)
