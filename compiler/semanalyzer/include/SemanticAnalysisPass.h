#ifndef SEMANTICANALYSISPASS_H
#define SEMANTICANALYSISPASS_H

#include <memory>

#include "ASTNode.h"
#include "ASTNodeVisitor.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class SemanticAnalysisPass : public parser::ASTNodeVisitor {
public:
    SemanticAnalysisPass() = default;

    virtual ~SemanticAnalysisPass() = default;

    void set_root(parser::CompilationUnitASTNodePtr root) { _root = root; }

    virtual int run() = 0;

protected:
    parser::CompilationUnitASTNodePtr _root;

private:
};

using SemanticAnalysisPassPtr = std::shared_ptr<SemanticAnalysisPass>;

template <typename T>
concept convertible_to_SemanticAnalysisPassPtr = requires {
    typename T::element_type;
    requires std::convertible_to<T, SemanticAnalysisPassPtr> && std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif