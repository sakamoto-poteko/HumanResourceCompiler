cmake_minimum_required(VERSION 3.25)
project(hrc_lexer)

set(LEXER_OUT "${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp")

flex_target(LEXER "src/hrl.l" "${LEXER_OUT}")

add_library(${PROJECT_NAME}
src/HRLLexer.cpp
src/lexer_helper.cpp
${LEXER_OUT}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
