cmake_minimum_required(VERSION 3.10)
project(hrc_tests_semanalyzer)

find_package(Boost CONFIG REQUIRED COMPONENTS unit_test_framework)

add_executable(${PROJECT_NAME}
    tests.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE hrc_semanalyzer hrc_parser hrc_lexer hrc_util)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::unit_test_framework)

add_test(NAME SemanticAnalyzerTests COMMAND ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/hrml)
