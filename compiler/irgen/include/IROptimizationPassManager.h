#ifndef IROPTIMIZATIONPASSMANAGER_H
#define IROPTIMIZATIONPASSMANAGER_H

#include <string>
#include <vector>

#include "IRGenOptions.h"
#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class IROptimizationPassManager {
public:
    IROptimizationPassManager(const ProgramPtr &program, const IRGenOptions &options)
        : _program(program)
        , _options(options)
    {
    }

    ~IROptimizationPassManager() = default;

    template <typename PassT, typename... StringArgs>
        requires(std::is_base_of_v<IROptimizationPass, PassT> && ((std::is_same_v<const std::string &, StringArgs> || std::is_convertible_v<StringArgs, std::string>) && ...))
    std::shared_ptr<PassT> add_pass(
        const std::string &pass_name,
        const std::string &after_pass_asm_path = "",
        const std::string &after_pass_graph_path = "",
        StringArgs &&...additional_save_paths)
    {
        std::shared_ptr<PassT> pass = std::make_shared<PassT>(_program, _options);

        _passes.push_back(pass);
        _pass_names.push_back(pass_name);
        _pass_asm_filepaths.push_back(after_pass_asm_path);
        _pass_graph_filepaths.push_back(after_pass_graph_path);
        std::vector<std::string> add_paths;
        add_paths.emplace_back(std::forward<StringArgs>(additional_save_paths)...);
        _additional_save_paths.push_back(std::move(add_paths));

        return pass;
    }

    int run(bool fail_fast = true);

    static IROptimizationPassManager create_with_default_pass_configuration(
        const ProgramPtr &program,
        const IRGenOptions &options,
        bool enable_output,
        const std::string &output_file_prefix = "");

private:
    ProgramPtr _program;
    std::vector<std::string> _pass_asm_filepaths;
    std::vector<std::string> _pass_graph_filepaths;
    std::vector<std::string> _pass_names;
    std::vector<std::vector<std::string>> _additional_save_paths;
    std::vector<IROptimizationPassPtr> _passes;
    const IRGenOptions &_options;
};

CLOSE_IRGEN_NAMESPACE

#endif