cmake_minimum_required(VERSION 3.10)
project(hrc_tests)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
   Tests.cpp
   SemAnalyzer.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE hrc_semanalyzer hrc_parser hrc_lexer hrc_util)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::GTest GTest::Main)

add_test(NAME CompilerTests COMMAND ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/hrml)
