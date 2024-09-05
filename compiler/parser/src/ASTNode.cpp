#include "ASTNode.h"
#include "ASTNodeVisitor.h"
#include "parser_global.h"
#include <type_traits>

OPEN_PARSER_NAMESPACE

#define VISIT_NODE() \
    return visitor->visit(shared_from_this_casted<std::remove_reference_t<decltype(*this)>>())

// begin

int BooleanASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int IntegerASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int VariableDeclarationASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int VariableAssignmentASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int VariableAccessASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int FloorAssignmentASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int FloorAccessASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int NegativeExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int IncrementExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int DecrementExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int NotExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int AddExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int SubExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int MulExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int DivExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int ModExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int EqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int NotEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int GreaterThanExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int GreaterEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int LessThanExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int LessEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int AndExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int OrExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int InvocationExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int IfStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int WhileStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int ForStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int ReturnStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int StatementBlockASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int SubprocDefinitionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int FunctionDefinitionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int CompilationUnitASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int EmptyStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int FloorBoxInitStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

CLOSE_PARSER_NAMESPACE

// end
