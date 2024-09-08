#include "ConstantFoldingPass.h"
#include "ASTNodeForward.h"
#include "SemanticAnalysisPass.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

ConstantFoldingPass::ConstantFoldingPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
    : SemanticAnalysisPass(std::move(filename), std::move(root))
{
}

ConstantFoldingPass::~ConstantFoldingPass()
{
}

int ConstantFoldingPass::visit(IntegerASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(BooleanASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(VariableDeclarationASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(VariableAssignmentASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(VariableAccessASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(FloorBoxInitStatementASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(FloorAssignmentASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(FloorAccessASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(NegativeExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(NotExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(IncrementExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(DecrementExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(AddExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(SubExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(MulExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(DivExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(ModExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(EqualExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(NotEqualExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(GreaterThanExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(GreaterEqualExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(LessThanExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(LessEqualExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(AndExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(OrExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(InvocationExpressionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(EmptyStatementASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(IfStatementASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(WhileStatementASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(ForStatementASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(ReturnStatementASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(BreakStatementASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(ContinueStatementASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(StatementBlockASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(SubprocDefinitionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(FunctionDefinitionASTNodePtr node)
{
    //
}

int ConstantFoldingPass::visit(CompilationUnitASTNodePtr node)
{
    //
}

int ConstantFoldingPass::run()
{
    return 1;
}

CLOSE_SEMANALYZER_NAMESPACE
// end
