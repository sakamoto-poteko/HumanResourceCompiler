#include <fstream>

#include <spdlog/spdlog.h>

#include "IROptimizationPassManager.h"
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

CLOSE_IRGEN_NAMESPACE
// end
