cmake_minimum_required(VERSION 3.25)
project(hrlc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

enable_testing()

find_package(spdlog REQUIRED)
find_package(FLEX 2.6 REQUIRED)

include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(util)
add_subdirectory(lexer)
add_subdirectory(hrc)