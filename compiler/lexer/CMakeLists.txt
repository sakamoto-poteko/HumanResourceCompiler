set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LEXER_OUT "${PARSER_DIR}/lexer.cpp")

flex_target(LEXER "hrl.l" "${LEXER_OUT}")

add_library(${PROJECT_NAME}
HRLLexer.cpp
lexer_helper.cpp
${LEXER_OUT}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
