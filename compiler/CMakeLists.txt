cmake_minimum_required(VERSION 3.25)
project(hrlc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX)
endif()

# EBNF Tool
add_subdirectory(${CMAKE_SOURCE_DIR}/../utils/ebnf ebnf-tool)

add_custom_target(
    EbnfTool ALL
    DEPENDS ebnf
)

# Compiler
find_package(spdlog REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS graph)
find_package(FLEX 2.6 REQUIRED)

include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(util)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(semanalyzer)
add_subdirectory(hrc)