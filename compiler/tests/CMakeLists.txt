cmake_minimum_required(VERSION 3.10)
project(hrc_tests)

find_package(GTest REQUIRED)
find_package(Boost CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
   Tests.cpp
   TestSemAnalyzer.cpp
   TestASTInterpreter.cpp
   TestIRInterpreter.cpp
   WithParsed.cpp
   WithSemanticAnalyzed.cpp
   WithIR.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE FALLBACK_HRML_DIR="${CMAKE_CURRENT_SOURCE_DIR}/hrml")

target_link_libraries(${PROJECT_NAME} PRIVATE hrc_interpreter hrc_irgen hrc_semanalyzer hrc_parser hrc_lexer hrc_util)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::GTest GTest::Main)

add_test(
   NAME
   CompilerTests
   COMMAND
   ${PROJECT_NAME}
   # ${CMAKE_CURRENT_SOURCE_DIR}/hrml
   WORKING_DIRECTORY
   ${CMAKE_CURRENT_SOURCE_DIR}/run
)
