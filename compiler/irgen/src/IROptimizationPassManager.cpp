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

        int rc = pass->run();
        if (!pass_asm_path.empty()) {
            
        }

        if (!pass_graph_path.empty()) {
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
