#include <cassert>

#include <ranges>

#include <spdlog/spdlog.h>

#include "SemanticAnalysisPassManager.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

SemanticAnalysisPassManager::~SemanticAnalysisPassManager()
{
}

void SemanticAnalysisPassManager::add_pass(
    SemanticAnalysisPassPtr pass,
    const std::string &pass_name,
    const std::string &after_pass_graph_path,
    const std::set<int> enabled_attributes)
{
    pass->set_root(_root);

    _passes.push_back(pass);
    _pass_names.push_back(pass_name);
    _pass_graph_filepaths.push_back(after_pass_graph_path);
    _pass_graph_enabled_attrs.push_back(enabled_attributes);
}

int SemanticAnalysisPassManager::run(bool fail_fast)
{
    int result = 0;

    assert(_passes.size() == _pass_names.size() == _pass_graph_filepaths.size() == _pass_graph_enabled_attrs.size());

    for (int i = 0; i < _passes.size(); ++i) {
        const auto &pass = _passes.at(i);
        const auto &pass_name = _pass_names.at(i);
        const auto &pass_path = _pass_graph_filepaths.at(i);
        const auto &pass_attr = _pass_graph_enabled_attrs.at(i);

        int rc = pass->run();
        if (!pass_path.empty()) {
            parser::ASTNodeGraphvizBuilder graph_builder(_root);
            graph_builder.generate_graphviz(pass_path, pass_attr);
        }

        if (rc != 0) {
            spdlog::error("Semantic pass {} failed", pass_name);
            if (fail_fast) {
                return result;
            } else {
                result = rc;
            }
        }
    }
    return 0;
}

CLOSE_SEMANALYZER_NAMESPACE
// end
