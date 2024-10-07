cmake_minimum_required(VERSION 3.5)
project(hrc_interpreter)

add_library(${PROJECT_NAME}
    src/InterpreterAccumulator.cpp
    src/ASTInterpreter.cpp
    src/AbstractInterpreter.cpp
    src/IRInterpreter.cpp
    src/InterpreterIOManager.cpp
    src/InterpreterMemoryManager.cpp
    src/InterpreterExceptions.cpp
    src/HRMByte.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE hrc_irgen hrc_semanalyzer hrc_parser hrc_lexer hrc_util)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
