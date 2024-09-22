cmake_minimum_required(VERSION 3.25)
project(hrlc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    # add_compile_options(-fsanitize=address)
    # add_link_options(-fsanitize=address)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /Zc:preprocessor)
endif()

enable_testing()

# EBNF Tool
add_subdirectory(${CMAKE_SOURCE_DIR}/../utils/ebnf ebnf-tool)

add_custom_target(
    EbnfTool ALL
    DEPENDS ebnf
)

# Compiler
find_package(spdlog CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS graph)

if(WIN32)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    find_package(FlexBison REQUIRED)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    find_package(FLEX 2.6 REQUIRED)
endif()

include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(util)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(semanalyzer)
add_subdirectory(interpreter)
add_subdirectory(irgen)
add_subdirectory(hrc)
add_subdirectory(hrint)

add_subdirectory(tests)
