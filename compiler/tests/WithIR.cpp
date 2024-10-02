#include "WithIR.h"
#include "BuildSSAPass.h"
#include "ControlFlowGraphBuilder.h"
#include "EliminateDeadBasicBlockPass.h"
#include "ErrorManager.h"
#include "IROptimizationPassManager.h"
#include "MergeConditionalBranchPass.h"
#include "RenumberVariableIdPass.h"
#include "StripEmptyBasicBlockPass.h"
#include "StripUselessInstructionPass.h"
#include "VerifySSAPass.h"

void WithIR::setup_ir(bool optimize, const TestCaseData &data, bool &result)
{
    setup_semantic_analyze(optimize, data, result);

    hrl::irgen::IROptimizationPassManager irop_passmgr(program);
    irop_passmgr.add_pass<hrl::irgen::StripUselessInstructionPass>(
        "StripNoOpPass",
        "build/strnop.hrasm",
        "build/strnop.dot");
    irop_passmgr.add_pass<hrl::irgen::StripEmptyBasicBlockPass>(
        "StripEmptyBasicBlockPass",
        "build/strebb.hrasm",
        "build/strebb.dot");
    irop_passmgr.add_pass<hrl::irgen::ControlFlowGraphBuilder>(
        "ControlFlowGraphBuilderPass",
        "build/cfgbuilder.hrasm",
        "build/cfgbuilder.dot");
    irop_passmgr.add_pass<hrl::irgen::MergeConditionalBranchPass>(
        "MergeCondBrPass",
        "build/mgcondbr.hrasm",
        "build/mgcondbr.dot");
    irop_passmgr.add_pass<hrl::irgen::EliminateDeadBasicBlockPass>(
        "EliminateDeadBasicBlockPass",
        "build/edbb.hrasm",
        "build/edbb.dot");
    irop_passmgr.add_pass<hrl::irgen::BuildSSAPass>(
        "BuildSSAPass",
        "build/ssa.hrasm",
        "build/ssa.dot");
    irop_passmgr.add_pass<hrl::irgen::RenumberVariableIdPass>(
        "SSARenumberVariableId",
        "build/ssa-renum.hrasm",
        "build/ssa-renum.dot");
    irop_passmgr.add_pass<hrl::irgen::VerifySSAPass>("VerifySSA");

    int irop_result = irop_passmgr.run(true);
    ErrorManager::instance().print_all();

    ASSERT_EQ(irop_result, 0) << "Expected IR optimizer to pass but it failed";
}
