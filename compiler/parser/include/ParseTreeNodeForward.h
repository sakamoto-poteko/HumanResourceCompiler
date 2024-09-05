#ifndef PARSE_TREE_NODE_FORWARD_H
#define PARSE_TREE_NODE_FORWARD_H

#include <memory>

#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ParseTreeNode;
class IdentifierPTNode;
class IntegerLiteralPTNode;
class BooleanLiteralPTNode;
class BinaryOperatorPTNode;

class VariableDeclarationPTNode;
class VariableAssignmentPTNode;
class FloorAssignmentPTNode;

class PositiveExpressionPTNode;
class NegativeExpressionPTNode;
class NotExpressionPTNode;
class AbstractExpressionPTNode;
class BinaryExpressionPTNode;
class AbstractUnaryExpressionPTNode;
class IncrementExpressionPTNode;
class DecrementExpressionPTNode;
class FloorAccessPTNode;
class AbstractPrimaryExpressionPTNode;
class ParenthesizedExpressionPTNode;
class InvocationExpressionPTNode;

class AbstractStatementPTNode;
class AbstractEmbeddedStatementPTNode;
class AbstractSelectionStatementPTNode;
class AbstractIterationStatementPTNode;
class AbstractSubroutinePTNode;

class IfStatementPTNode;
class WhileStatementPTNode;
class ForStatementPTNode;
class ReturnStatementPTNode;
class BreakContinueStatementPTNode;
class FloorBoxInitStatementPTNode;
class FloorMaxInitStatementPTNode;
class EmptyStatementPTNode;
class StatementBlockPTNode;
class FloorAssignmentStatementPTNode;
class VariableAssignmentStatementPTNode;
class VariableDeclarationStatementPTNode;
class InvocationStatementPTNode;

class SubprocDefinitionPTNode;
class FunctionDefinitionPTNode;

class ImportDirectivePTNode;
class CompilationUnitPTNode;

using ParseTreePTNodePtr = std::shared_ptr<ParseTreeNode>;
using IdentifierPTNodePtr = std::shared_ptr<IdentifierPTNode>;
using IntegerLiteralPTNodePtr = std::shared_ptr<IntegerLiteralPTNode>;
using BooleanLiteralPTNodePtr = std::shared_ptr<BooleanLiteralPTNode>;
using BinaryOperatorPTNodePtr = std::shared_ptr<BinaryOperatorPTNode>;

using VariableDeclarationPTNodePtr = std::shared_ptr<VariableDeclarationPTNode>;
using VariableAssignmentPTNodePtr = std::shared_ptr<VariableAssignmentPTNode>;
using FloorAssignmentPTNodePtr = std::shared_ptr<FloorAssignmentPTNode>;
using FloorAssignmentStatementPTNodePtr = std::shared_ptr<FloorAssignmentStatementPTNode>;
using VariableAssignmentStatementPTNodePtr = std::shared_ptr<VariableAssignmentStatementPTNode>;
using VariableDeclarationStatementPTNodePtr = std::shared_ptr<VariableDeclarationStatementPTNode>;

using AbstractExpressionPTNodePtr = std::shared_ptr<AbstractExpressionPTNode>;
using BinaryExpressionPTNodePtr = std::shared_ptr<BinaryExpressionPTNode>;
using AbstractUnaryExpressionPTNodePtr = std::shared_ptr<AbstractUnaryExpressionPTNode>;
using IncrementExpressionPTNodePtr = std::shared_ptr<IncrementExpressionPTNode>;
using DecrementExpressionPTNodePtr = std::shared_ptr<DecrementExpressionPTNode>;
using FloorAccessPTNodePtr = std::shared_ptr<FloorAccessPTNode>;
using AbstractPrimaryExpressionPTNodePtr = std::shared_ptr<AbstractPrimaryExpressionPTNode>;
using ParenthesizedExpressionPTNodePtr = std::shared_ptr<ParenthesizedExpressionPTNode>;
using InvocationExpressionPTNodePtr = std::shared_ptr<InvocationExpressionPTNode>;
using PositiveExpressionPTNodePtr = std::shared_ptr<PositiveExpressionPTNode>;
using NegativeExpressionPTNodePtr = std::shared_ptr<NegativeExpressionPTNode>;
using NotExpressionPTNodePtr = std::shared_ptr<NotExpressionPTNode>;

using AbstractStatementPTNodePtr = std::shared_ptr<AbstractStatementPTNode>;
using AbstractEmbeddedStatementPTNodePtr = std::shared_ptr<AbstractEmbeddedStatementPTNode>;
using AbstractSelectionStatementPTNodePtr = std::shared_ptr<AbstractSelectionStatementPTNode>;
using AbstractIterationStatementPTNodePtr = std::shared_ptr<AbstractIterationStatementPTNode>;
using AbstractSubroutinePTNodePtr = std::shared_ptr<AbstractSubroutinePTNode>;

using IfStatementPTNodePtr = std::shared_ptr<IfStatementPTNode>;
using WhileStatementPTNodePtr = std::shared_ptr<WhileStatementPTNode>;
using ForStatementPTNodePtr = std::shared_ptr<ForStatementPTNode>;
using ReturnStatementPTNodePtr = std::shared_ptr<ReturnStatementPTNode>;
using BreakContinueStatementPTNodePtr = std::shared_ptr<BreakContinueStatementPTNode>;
using FloorBoxInitStatementPTNodePtr = std::shared_ptr<FloorBoxInitStatementPTNode>;
using FloorMaxInitStatementPTNodePtr = std::shared_ptr<FloorMaxInitStatementPTNode>;
using EmptyStatementPTNodePtr = std::shared_ptr<EmptyStatementPTNode>;
using StatementBlockPTNodePtr = std::shared_ptr<StatementBlockPTNode>;
using InvocationStatementPTNodePtr = std::shared_ptr<InvocationStatementPTNode>;

using FunctionDefinitionPTNodePtr = std::shared_ptr<FunctionDefinitionPTNode>;
using SubprocDefinitionPTNodePtr = std::shared_ptr<SubprocDefinitionPTNode>;

using ImportDirectivePTNodePtr = std::shared_ptr<ImportDirectivePTNode>;
using CompilationUnitPTNodePtr = std::shared_ptr<CompilationUnitPTNode>;

CLOSE_PARSER_NAMESPACE

#endif