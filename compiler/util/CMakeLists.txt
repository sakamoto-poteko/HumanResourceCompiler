cmake_minimum_required(VERSION 3.25)
project(hrc_util)

set(PRE_CONFIGURE_FILE "src/version.cmake.h.in")
set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.cmake.h")
include(${CMAKE_SOURCE_DIR}/cmake/versioning.cmake)

add_library(${PROJECT_NAME}
    ${POST_CONFIGURE_FILE}
    src/TerminalColor.cpp
    src/ErrorManager.cpp
    src/ErrorFilters.cpp
    src/Versioning.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
