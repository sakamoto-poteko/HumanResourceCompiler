cmake_minimum_required(VERSION 3.25)
project(ebnf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 2.3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost 1.45.0 COMPONENTS graph)

set(LEXER_OUT "${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp")
set(PARSER_OUT "${CMAKE_CURRENT_BINARY_DIR}/parser.cpp")

flex_target(LEXER "lexer.l" "${LEXER_OUT}")
bison_target(PARSER "parser.y" "${PARSER_OUT}" DEFINES_FILE "parser.h")

add_executable(${PROJECT_NAME}
main.cpp
ASTNode.cpp
ASTNodeVisitor.cpp
ASTPrintVisitor.cpp
${LEXER_OUT}
${PARSER_OUT}
)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
