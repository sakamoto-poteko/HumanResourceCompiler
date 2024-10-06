#include <fstream>

#include <spdlog/spdlog.h>

#include "AnalyzeLivenessPass.h"
#include "BuildControlFlowGraphPass.h"
#include "BuildSSAPass.h"
#include "EliminateDeadBasicBlockPass.h"
#include "IROptimizationPassManager.h"
#include "MergeConditionalBranchPass.h"
#include "PropagateCopyPass.h"
#include "RemoveDeadInstructionsPass.h"
#include "RenumberVariableIdPass.h"
#include "StripEmptyBasicBlockPass.h"
#include "VerifySSAPass.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

int IROptimizationPassManager::run(bool fail_fast)
{
    int result = 0;

    assert(_passes.size() == _pass_names.size() && _pass_names.size() == _pass_graph_filepaths.size());

    for (std::size_t i = 0; i < _passes.size(); ++i) {
        const auto &pass = _passes.at(i);
        const auto &pass_name = _pass_names.at(i);
        const auto &pass_asm_path = _pass_asm_filepaths.at(i);
        const auto &pass_graph_path = _pass_graph_filepaths.at(i);
        const auto &additional_path = _additional_save_paths.at(i);

        spdlog::info("Running IR opt pass {}...", pass_name);
        int rc = pass->run();
        if (!pass_asm_path.empty()) {
            std::ofstream out(pass_asm_path);
            out << _program->to_string(false);
            out.close();
        }

        if (!pass_graph_path.empty()) {
            std::ofstream out(pass_graph_path);
            out << _program->generaet_graphviz();
            out.close();
        }

        for (std::size_t j = 0; j < additional_path.size(); ++j) {
            std::ofstream out(additional_path.at(j));
            out << pass->get_additional_metadata_text(static_cast<unsigned int>(j), additional_path.at(j));
            out.close();
        }

        if (rc != 0) {
            spdlog::error("IR opt pass {} failed", pass_name);
            if (fail_fast) {
                return rc;
            } else {
                result = rc;
            }
        }
    }

    return result;
}

IROptimizationPassManager IROptimizationPassManager::create_with_default_pass_configuration(
    const ProgramPtr &program,
    const IRGenOptions &options,
    bool enable_output,
    const std::string &output_file_prefix)
{
    IROptimizationPassManager irop_passmgr(program, options);
    irop_passmgr.add_pass<RemoveDeadInstructionsPass>(
        "StripNoOpAndEnterPass",
        enable_output ? output_file_prefix + "nonopenter.hrasm" : "",
        enable_output ? output_file_prefix + "nonopenter.dot" : "");
    irop_passmgr.add_pass<StripEmptyBasicBlockPass>(
        "StripEmptyBasicBlockPass",
        enable_output ? output_file_prefix + "noebb.hrasm" : "",
        enable_output ? output_file_prefix + "noebb.dot" : "");
    irop_passmgr.add_pass<MergeConditionalBranchPass>(
        "MergeCondBrPass",
        enable_output ? output_file_prefix + "mgcondbr.hrasm" : "",
        enable_output ? output_file_prefix + "mgcondbr.dot" : "");
    irop_passmgr.add_pass<BuildControlFlowGraphPass>(
        "ControlFlowGraphBuilderPass",
        enable_output ? output_file_prefix + "cfgbuilder.hrasm" : "",
        enable_output ? output_file_prefix + "cfgbuilder.dot" : "");
    irop_passmgr.add_pass<EliminateDeadBasicBlockPass>(
        "EliminateDeadBasicBlockPass",
        enable_output ? output_file_prefix + "edbb.hrasm" : "",
        enable_output ? output_file_prefix + "edbb.dot" : "");
    irop_passmgr.add_pass<AnalyzeLivenessPass>(
        "AnalyzeLivenessPreSSAPass",
        enable_output ? output_file_prefix + "liveness.hrasm" : "",
        enable_output ? output_file_prefix + "liveness.dot" : "",
        enable_output ? output_file_prefix + "liveness.yml" : "");
    irop_passmgr.add_pass<BuildSSAPass>(
        "BuildSSAPass",
        enable_output ? output_file_prefix + "ssa.hrasm" : "",
        enable_output ? output_file_prefix + "ssa.dot" : "",
        enable_output ? output_file_prefix + "ssa.domtree.dot" : "");
    irop_passmgr.add_pass<RenumberVariableIdPass>(
        "RenumberVariableIdPostSSAPass",
        enable_output ? output_file_prefix + "renum-ssa.hrasm" : "",
        enable_output ? output_file_prefix + "renum-ssa.dot" : "");
    irop_passmgr.add_pass<VerifySSAPass>("VerifySSA");
    irop_passmgr.add_pass<PropagateCopyPass>(
        "PropagateCopyHighIRSSAPass",
        enable_output ? output_file_prefix + "propcopy-hirssa.hrasm" : "",
        enable_output ? output_file_prefix + "propcopy-hirssa.dot" : "");
    irop_passmgr.add_pass<RemoveDeadInstructionsPass>(
        "RemoveDeadInstructionPass",
        enable_output ? output_file_prefix + "nodeadinstr.hrasm" : "",
        enable_output ? output_file_prefix + "nodeadinstr.dot" : "");
    irop_passmgr.add_pass<AnalyzeLivenessPass>(
        "AnalyzeLivenessPostSSAPass",
        enable_output ? output_file_prefix + "liveness-ssa.hrasm" : "",
        enable_output ? output_file_prefix + "liveness-ssa.dot" : "",
        enable_output ? output_file_prefix + "liveness-ssa.yml" : "");

    return irop_passmgr;
}

CLOSE_IRGEN_NAMESPACE
// end
