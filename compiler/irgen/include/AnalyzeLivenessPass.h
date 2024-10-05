#ifndef ANALYZELIVENESSPASS_H
#define ANALYZELIVENESSPASS_H

#include <vector>

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

// depends: CFG
// produces: IN, OUT, DEF, USE
class AnalyzeLivenessPass : public IROptimizationPass {
public:
    AnalyzeLivenessPass(const ProgramPtr &program);
    ~AnalyzeLivenessPass() = default;

    std::string get_additional_metadata_text(unsigned int task_index, const std::string &path) override;

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
    static void traverse_cfg(ControlFlowVertex vertex, const ControlFlowGraph &cfg, std::set<ControlFlowVertex> &visited, std::vector<ControlFlowVertex> &result);
    void calculate_in_out(const ControlFlowGraph &cfg, const std::vector<ControlFlowVertex> &vertices);
    void calculate_def_use(const SubroutinePtr &subroutine);
};

CLOSE_IRGEN_NAMESPACE

#endif