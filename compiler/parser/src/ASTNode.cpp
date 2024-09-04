#include "ASTNode.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

int BooleanASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int VariableDeclarationASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int VariableAssignmentASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int VariableAccessASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int FloorAssignmentASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int FloorAccessASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int NegativeExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int IncrementExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int DecrementExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int AddExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int SubExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int MulExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int DivExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int ModExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int EqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int NotEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int GreaterThanExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int GreaterEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int LessThanExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int LessEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int AndExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int OrExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int InvocationExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int IfStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int WhileStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int ForStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int ReturnStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int StatementBlockASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int SubprocDefinitionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int FunctionDefinitionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

int CompilationUnitASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(visitor); }

CLOSE_PARSER_NAMESPACE
// end
