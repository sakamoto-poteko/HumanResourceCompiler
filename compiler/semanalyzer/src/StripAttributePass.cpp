#include "StripAttributePass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

int StripAttributePass::run()
{
    return SemanticAnalysisPass::visit(_root);
}

void StripAttributePass::enter_node(const parser::ASTNodePtr &node)
{
    for (int attr : _attributes) {
        node->remove_attribute(attr);
    }
    SemanticAnalysisPass::enter_node(node);
}

CLOSE_SEMANALYZER_NAMESPACE