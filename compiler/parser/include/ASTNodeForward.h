#ifndef ASTNODEFORWARD_H
#define ASTNODEFORWARD_H

#include <memory>

#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNode;
class IdentifierNode;
class IntegerLiteralNode;
class BooleanLiteralNode;
class BinaryOperatorNode;

class VariableDeclarationNode;
class VariableAssignmentNode;
class FloorAssignmentNode;

class AbstractExpressionNode;
class BinaryExpressionNode;
class AbstractUnaryExpressionNode;
class IncrementExpressionNode;
class DecrementExpressionNode;
class FloorAccessNode;
class AbstractPrimaryExpressionNode;
class ParenthesizedExpressionNode;
class InvocationExpressionNode;

class AbstractStatementNode;
class AbstractEmbeddedStatementNode;
class AbstractSelectionStatementNode;
class AbstractIterationStatementNode;

class IfStatementNode;
class WhileStatementNode;
class ForStatementNode;
class ReturnStatementNode;
class FloorBoxInitStatementNode;
class FloorMaxInitStatementNode;
class EmptyStatementNode;
class StatementBlockNode;

class RoutineDefinitionCommonNode;
class SubprocDefinitionNode;
class FunctionDefinitionNode;

class ImportDirectiveNode;
class CompilationUnitNode;

using ASTNodePtr = std::shared_ptr<ASTNode>;
using IdentifierNodePtr = std::shared_ptr<IdentifierNode>;
using IntegerLiteralNodePtr = std::shared_ptr<IntegerLiteralNode>;
using BooleanLiteralNodePtr = std::shared_ptr<BooleanLiteralNode>;
using BinaryOperatorNodePtr = std::shared_ptr<BinaryOperatorNode>;

using VariableDeclarationNodePtr = std::shared_ptr<VariableDeclarationNode>;
using VariableAssignmentNodePtr = std::shared_ptr<VariableAssignmentNode>;
using FloorAssignmentNodePtr = std::shared_ptr<FloorAssignmentNode>;

using AbstractExpressionNodePtr = std::shared_ptr<AbstractExpressionNode>;
using BinaryExpressionNodePtr = std::shared_ptr<BinaryExpressionNode>;
using AbstractUnaryExpressionNodePtr = std::shared_ptr<AbstractUnaryExpressionNode>;
using IncrementExpressionNodePtr = std::shared_ptr<IncrementExpressionNode>;
using DecrementExpressionNodePtr = std::shared_ptr<DecrementExpressionNode>;
using FloorAccessNodePtr = std::shared_ptr<FloorAccessNode>;
using PrimaryExpressionNodePtr = std::shared_ptr<AbstractPrimaryExpressionNode>;
using ParenthesizedExpressionNodePtr = std::shared_ptr<ParenthesizedExpressionNode>;
using InvocationExpressionNodePtr = std::shared_ptr<InvocationExpressionNode>;

using AbstractStatementNodePtr = std::shared_ptr<AbstractStatementNode>;
using AbstractEmbeddedStatementNodePtr = std::shared_ptr<AbstractEmbeddedStatementNode>;
using AbstractSelectionStatementNodePtr = std::shared_ptr<AbstractSelectionStatementNode>;
using AbstractIterationStatementNodePtr = std::shared_ptr<AbstractIterationStatementNode>;

using IfStatementNodePtr = std::shared_ptr<IfStatementNode>;
using WhileStatementNodePtr = std::shared_ptr<WhileStatementNode>;
using ForStatementNodePtr = std::shared_ptr<ForStatementNode>;
using ReturnStatementNodePtr = std::shared_ptr<ReturnStatementNode>;
using FloorBoxInitStatementNodePtr = std::shared_ptr<FloorBoxInitStatementNode>;
using FloorMaxInitStatementNodePtr = std::shared_ptr<FloorMaxInitStatementNode>;
using EmptyStatementNodePtr = std::shared_ptr<EmptyStatementNode>;
using StatementBlockNodePtr = std::shared_ptr<StatementBlockNode>;

using FunctionDefinitionNodePtr = std::shared_ptr<FunctionDefinitionNode>;
using SubprocDefinitionNodePtr = std::shared_ptr<SubprocDefinitionNode>;

using ImportDirectiveNodePtr = std::shared_ptr<ImportDirectiveNode>;
using CompilationUnitNodePtr = std::shared_ptr<CompilationUnitNode>;

CLOSE_PARSER_NAMESPACE

#endif