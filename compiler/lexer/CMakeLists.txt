cmake_minimum_required(VERSION 3.25)
project(hrc_lexer)

set(LEXER_OUT "${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp")
set(LEXER_HDR "${CMAKE_CURRENT_BINARY_DIR}/lexer.h")

flex_target(LEXER "src/hrl.l" "${LEXER_OUT}" DEFINES_FILE ${LEXER_HDR})

add_library(${PROJECT_NAME}
    src/HRLLexer.cpp
    src/HRLToken.cpp
    src/lexer_helper.cpp
    ${LEXER_OUT}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_source_files_properties(${LEXER_OUT} PROPERTIES COMPILE_FLAGS "-w")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_source_files_properties(${LEXER_OUT} PROPERTIES COMPILE_FLAGS "/w")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only hrc_util)
