cmake_minimum_required(VERSION 3.5)
project(hrc)

find_package(Boost CONFIG REQUIRED COMPONENTS program_options)

add_executable(${PROJECT_NAME}
    main.cpp
    CompilerOptions.cpp
    FileManager.cpp
    Utilities.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE hrc_irgen hrc_semanalyzer hrc_parser hrc_lexer hrc_util)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only Boost::program_options)
