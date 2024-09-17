#ifndef SEMANTICANALYSISPASSMANAGER_H
#define SEMANTICANALYSISPASSMANAGER_H

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "SemanticAnalysisPass.h"
#include "SymbolTable.h"
#include "WithSymbolTable.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class SemanticAnalysisPassManager {
public:
    SemanticAnalysisPassManager(parser::CompilationUnitASTNodePtr root, StringPtr filename)
        : _root(std::move(root))
        , _filename(std::move(filename))
        , _symbol_table(std::make_shared<SymbolTable>())
    {
    }

    ~SemanticAnalysisPassManager();

    template <typename PassT>
        requires std::is_base_of_v<SemanticAnalysisPass, PassT>
    std::shared_ptr<PassT> add_pass(
        const std::string &pass_name,
        const std::string &after_pass_graph_path = "",
        const std::set<int> enabled_attributes = std::set<int>())
    {
        std::shared_ptr<PassT> pass = std::make_shared<PassT>(_filename, _root);

        if constexpr (std::is_base_of_v<WithSymbolTable, PassT>) {
            pass->set_symbol_table(_symbol_table);
        }

        add_pass(pass, pass_name, after_pass_graph_path, enabled_attributes);
        return pass;
    }

    int run(bool fail_fast = true);

    void set_symbol_table(const SymbolTablePtr symtbl);

    const SymbolTablePtr &get_symbol_table() const;

private:
    parser::CompilationUnitASTNodePtr _root;
    StringPtr _filename;
    std::vector<std::string> _pass_graph_filepaths;
    std::vector<std::set<int>> _pass_graph_enabled_attrs;
    std::vector<std::string> _pass_names;
    std::vector<SemanticAnalysisPassPtr> _passes;
    SymbolTablePtr _symbol_table;

    void add_pass(
        SemanticAnalysisPassPtr pass,
        const std::string &pass_name,
        const std::string &after_pass_graph_path = "",
        const std::set<int> enabled_attributes = std::set<int>());
};

CLOSE_SEMANALYZER_NAMESPACE

#endif