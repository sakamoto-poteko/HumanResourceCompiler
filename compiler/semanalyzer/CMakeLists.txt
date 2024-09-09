cmake_minimum_required(VERSION 3.25)
project(hrc_semanalyzer)

add_library(${PROJECT_NAME}
    src/ScopeManager.cpp
    src/Symbol.cpp
    src/SymbolTable.cpp
    src/SemanticAnalysisPassManager.cpp
    src/SemanticAnalysisPass.cpp
    src/SymbolAnalysisPass.Visits.cpp
    src/SymbolAnalysisPass.Utils.cpp
    src/ConstantFoldingPass.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only hrc_util hrc_parser)
