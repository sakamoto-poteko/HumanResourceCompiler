cmake_minimum_required(VERSION 3.25)
project(hrc_parser)

set(OUTPUT_BASE "${CMAKE_CURRENT_BINARY_DIR}/First")

add_library(${PROJECT_NAME}
    src/ParseTreeNode.cpp
    src/ParseTreeNodeVisitor.cpp
    src/RecursiveDescentParser.Parsing.cpp
    src/RecursiveDescentParser.Expr.cpp
    src/RecursiveDescentParser.Utils.cpp
    src/ParseTreeNodeGraphvizBuilder.cpp
    src/Formatter.cpp
    "${OUTPUT_BASE}.h"
    "${OUTPUT_BASE}.cpp"
)

add_custom_command(
    OUTPUT "${OUTPUT_BASE}.h" "${OUTPUT_BASE}.cpp"
    COMMAND ebnf -i ${CMAKE_SOURCE_DIR}/../design/hrl.ebnf -s compilation_unit -f ${CMAKE_SOURCE_DIR}/../design/hrl.tokens -F${OUTPUT_BASE}
    DEPENDS ${CMAKE_SOURCE_DIR}/../design/hrl.ebnf ${CMAKE_SOURCE_DIR}/../design/hrl.tokens
    COMMENT "Generating ${OUTPUT_BASE}.h and ${OUTPUT_BASE}.cpp using ebnf"
)

# # Add a custom target that depends on the generated files
# add_custom_target(generate_hrl_files ALL
#     DEPENDS "${OUTPUT_BASE}.h" "${OUTPUT_BASE}.cpp"
# )

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only hrc_util hrc_lexer)
