#ifndef STRIPATTRIBUTEPASS_H
#define STRIPATTRIBUTEPASS_H

#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class StripAttributePass : public hrl::semanalyzer::SemanticAnalysisPass {
public:
    StripAttributePass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~StripAttributePass() = default;

    void add_attribute(int attr_id) { _attributes.insert(attr_id); }

    void remove_attribute(int attr_id) { _attributes.erase(attr_id); }

    int run() override;

protected:
    void enter_node(const parser::ASTNodePtr &node) override;

private:
    std::set<int> _attributes;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif