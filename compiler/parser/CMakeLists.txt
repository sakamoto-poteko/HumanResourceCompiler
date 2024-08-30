cmake_minimum_required(VERSION 3.25)
project(hrc_parser)

add_library(${PROJECT_NAME}
src/ASTNode.cpp
src/ASTNodeVisitor.cpp
src/RecursiveDescentParser.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only hrc_util hrc_lexer)
