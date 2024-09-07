#ifndef SEMANTICANALYSISPASSMANAGER_H
#define SEMANTICANALYSISPASSMANAGER_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ASTNodeForward.h"
#include "ASTNodeGraphvizBuilder.h"
#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class SemanticAnalysisPassManager {
public:
    SemanticAnalysisPassManager(parser::CompilationUnitASTNodePtr root)
        : _root(std::move(root))
    {
    }

    ~SemanticAnalysisPassManager();

    template <typename PassT>
        requires std::is_base_of_v<SemanticAnalysisPass, PassT>
    std::shared_ptr<PassT> add_pass(
        const std::string &pass_name,
        const std::string &filename,
        const std::string &after_pass_graph_path = "",
        const std::set<int> enabled_attributes = std::set<int>())
    {
        SemanticAnalysisPassPtr pass = std::make_shared<PassT>(filename);
        add_pass(pass, pass_name, after_pass_graph_path, enabled_attributes);
        return pass;
    }

    void add_pass(
        SemanticAnalysisPassPtr pass,
        const std::string &pass_name,
        const std::string &after_pass_graph_path = "",
        const std::set<int> enabled_attributes = std::set<int>());

    int run(bool fail_fast = true);

private:
    parser::CompilationUnitASTNodePtr _root;
    std::vector<std::string> _pass_graph_filepaths;
    std::vector<std::set<int>> _pass_graph_enabled_attrs;
    std::vector<std::string> _pass_names;
    std::vector<SemanticAnalysisPassPtr> _passes;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif