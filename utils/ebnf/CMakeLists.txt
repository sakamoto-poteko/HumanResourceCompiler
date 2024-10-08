cmake_minimum_required(VERSION 3.25)
project(ebnf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if(WIN32)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    find_package(FlexBison REQUIRED)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    find_package(FLEX 2.6 REQUIRED)
    find_package(BISON 2.3 REQUIRED)
endif()

find_package(spdlog REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS graph program_options)

set(LEXER_OUT "${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp")
set(PARSER_OUT "${CMAKE_CURRENT_BINARY_DIR}/parser.cpp")

flex_target(LEXER "lexer.l" "${LEXER_OUT}")
bison_target(PARSER "parser.y" "${PARSER_OUT}" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/parser.h")

add_executable(${PROJECT_NAME}
    main.cpp
    Tasks.cpp
    ASTNode.cpp
    ASTNodeVisitor.cpp
    ASTPrintVisitor.cpp
    DependencyGraphBuilder.cpp
    DependencyGraphAnalyzer.cpp
    ${LEXER_OUT}
    ${PARSER_OUT}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_source_files_properties(${LEXER_OUT} ${PARSER_OUT} PROPERTIES COMPILE_FLAGS "-w")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_source_files_properties(${LEXER_OUT} ${PARSER_OUT} PROPERTIES COMPILE_FLAGS "/w")
endif()

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only Boost::program_options)

if(WIN32)
    add_dependencies(ebnf FlexBison)
endif()

set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Default installation directory")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin) # On Windows use RUNTIME, on Linux/macOS, use bin