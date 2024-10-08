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
    AnalyzeLivenessPass(const ProgramPtr &program, const IRGenOptions &options)
        : IROptimizationPass(program, options)
    {
    }

    ~AnalyzeLivenessPass() = default;

    std::string get_additional_metadata_text(unsigned int task_index, const std::string &path) override;

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
    static void traverse_cfg(BBGraphVertex vertex, const BBGraph &cfg, std::set<BBGraphVertex> &visited, std::vector<BBGraphVertex> &result);
    void calculate_in_out(const BBGraph &cfg, const std::vector<BBGraphVertex> &vertices);
    void calculate_def_use(const SubroutinePtr &subroutine);
};

CLOSE_IRGEN_NAMESPACE

#endif