#include "WithIR.h"
#include "AnalyzeLivenessPass.h"
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
        data.filename + "-strnop.hrasm",
        data.filename + "-strnop.dot");
    irop_passmgr.add_pass<hrl::irgen::StripEmptyBasicBlockPass>(
        "StripEmptyBasicBlockPass",
        data.filename + "-strebb.hrasm",
        data.filename + "-strebb.dot");
    irop_passmgr.add_pass<hrl::irgen::ControlFlowGraphBuilder>(
        "ControlFlowGraphBuilderPass",
        data.filename + "-cfgbuilder.hrasm",
        data.filename + "-cfgbuilder.dot");
    irop_passmgr.add_pass<hrl::irgen::MergeConditionalBranchPass>(
        "MergeCondBrPass",
        data.filename + "-mgcondbr.hrasm",
        data.filename + "-mgcondbr.dot");
    irop_passmgr.add_pass<hrl::irgen::EliminateDeadBasicBlockPass>(
        "EliminateDeadBasicBlockPass",
        data.filename + "-edbb.hrasm",
        data.filename + "-edbb.dot");
    irop_passmgr.add_pass<hrl::irgen::AnalyzeLivenessPass>("AnalyzeLivenessPass");
    irop_passmgr.add_pass<hrl::irgen::BuildSSAPass>(
        "BuildSSAPass",
        data.filename + "-ssa.hrasm",
        data.filename + "-ssa.dot");
    irop_passmgr.add_pass<hrl::irgen::RenumberVariableIdPass>(
        "SSARenumberVariableId",
        data.filename + "-ssa-renum.hrasm",
        data.filename + "-ssa-renum.dot");
    irop_passmgr.add_pass<hrl::irgen::VerifySSAPass>("VerifySSA");

    int irop_result = irop_passmgr.run(true);
    ErrorManager::instance().print_all();

    ASSERT_EQ(irop_result, 0) << "Expected IR optimizer to pass but it failed";
}
