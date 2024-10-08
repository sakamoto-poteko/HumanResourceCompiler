cmake_minimum_required(VERSION 3.25)
project(hrlc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler
# YAML-CPP
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Disable YAML-CPP contrib")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable YAML-CPP tools")

include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)
FetchContent_MakeAvailable(yaml-cpp)

if(WIN32)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    find_package(FlexBison REQUIRED)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    find_package(FLEX 2.6 REQUIRED)
endif()

# SPDLOG
find_package(spdlog CONFIG REQUIRED)

# Boost
find_package(Boost CONFIG REQUIRED COMPONENTS graph)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /Zc:preprocessor)
endif()

include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

enable_testing()

# EBNF Tool
add_subdirectory(${CMAKE_SOURCE_DIR}/../utils/ebnf ebnf-tool)
add_custom_target(EbnfTool ALL DEPENDS ebnf)

# Compiler projects
add_subdirectory(util)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(semanalyzer)
add_subdirectory(interpreter)
add_subdirectory(irgen)
add_subdirectory(hrc)
add_subdirectory(hrint)

add_subdirectory(tests)
