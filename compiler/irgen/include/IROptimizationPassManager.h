#ifndef IROPTIMIZATIONPASSMANAGER_H
#define IROPTIMIZATIONPASSMANAGER_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class IROptimizationPassManager {
public:
    IROptimizationPassManager(const ProgramPtr &program)
        : _program(program)
    {
    }

    ~IROptimizationPassManager() = default;

    template <typename PassT>
        requires std::is_base_of_v<IROptimizationPass, PassT>
    std::shared_ptr<PassT> add_pass(
        const std::string &pass_name,
        const std::string &after_pass_asm_path = "",
        const std::string &after_pass_graph_path = "")
    {
        std::shared_ptr<PassT> pass = std::make_shared<PassT>(_program);

        _passes.push_back(pass);
        _pass_names.push_back(pass_name);
        _pass_asm_filepaths.push_back(after_pass_asm_path);
        _pass_graph_filepaths.push_back(after_pass_graph_path);
        return pass;
    }

    int run(bool fail_fast = true);

private:
    ProgramPtr _program;
    std::vector<std::string> _pass_asm_filepaths;
    std::vector<std::string> _pass_graph_filepaths;
    std::vector<std::string> _pass_names;
    std::vector<IROptimizationPassPtr> _passes;
};

CLOSE_IRGEN_NAMESPACE

#endif