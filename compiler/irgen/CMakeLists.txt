cmake_minimum_required(VERSION 3.5)
project(hrc_irgen)

add_library(${PROJECT_NAME}
    src/ThreeAddressCode.cpp
    src/TACGen.Visitor.cpp
    src/TACGen.Utils.cpp
    src/TACGen.ProgramBuilder.cpp
    src/Operand.cpp
    src/IROps.cpp
    src/IRProgramStructure.cpp
    src/IROptimizationPass.cpp
    src/IROptimizationPassManager.cpp
    src/MergeConditionalBranchPass.cpp
    src/StripUselessInstructionPass.cpp
    src/StripEmptyBasicBlockPass.cpp
    src/BuildControlFlowGraphPass.cpp
    src/BuildSSAPass.cpp
    src/EliminateDeadBasicBlockPass.cpp
    src/VerifySSAPass.cpp
    src/PropagateCopyPass.cpp
    src/RenumberVariableIdPass.cpp
    src/AnalyzeLivenessPass.cpp
    src/GraphvizGenerator.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE hrc_semanalyzer hrc_parser hrc_lexer hrc_util)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only yaml-cpp::yaml-cpp)
