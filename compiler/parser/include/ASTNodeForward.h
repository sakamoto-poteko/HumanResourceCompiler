#ifndef ASTNODE_FORWARD_H
#define ASTNODE_FORWARD_H

#include <memory>
#include <vector>

#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNode;
class IntegerASTNode;
class BooleanASTNode;

class VariableDeclarationASTNode;
class VariableAssignmentASTNode;
class VariableAccessASTNode;
class FloorAssignmentASTNode;
class FloorAccessASTNode;
class EmptyStatementASTNode;

class AbstractExpressionASTNode;
class AbstractUnaryExpressionASTNode;
class AbstractPrimaryExpressionASTNode;
class AbstractBinaryExpressionASTNode;
class NegativeExpressionASTNode;
class NotExpressionASTNode;
class IncrementExpressionASTNode;
class DecrementExpressionASTNode;
class AddExpressionASTNode;
class SubExpressionASTNode;
class MulExpressionASTNode;
class DivExpressionASTNode;
class ModExpressionASTNode;

class EqualExpressionASTNode;
class NotEqualExpressionASTNode;
class GreaterThanExpressionASTNode;
class GreaterEqualExpressionASTNode;
class LessThanExpressionASTNode;
class LessEqualExpressionASTNode;
class AndExpressionASTNode;
class OrExpressionASTNode;

class InvocationExpressionASTNode;

class AbstractStatementASTNode;
class AbstractEmbeddedStatementASTNode;
class AbstractSelectionStatementASTNode;
class AbstractIterationStatementASTNode;
class AbstractSubroutineASTNode;

class IfStatementASTNode;
class WhileStatementASTNode;
class ForStatementASTNode;
class ReturnStatementASTNode;
class FloorBoxInitStatementASTNode;
class StatementBlockASTNode;

class SubprocDefinitionASTNode;
class FunctionDefinitionASTNode;

class CompilationUnitASTNode;


using ASTNodePtr = std::shared_ptr<ASTNode>;
using IntegerASTNodePtr = std::shared_ptr<IntegerASTNode>;
using BooleanASTNodePtr = std::shared_ptr<BooleanASTNode>;

using VariableDeclarationASTNodePtr = std::shared_ptr<VariableDeclarationASTNode>;
using VariableAssignmentASTNodePtr = std::shared_ptr<VariableAssignmentASTNode>;
using VariableAccessASTNodePtr = std::shared_ptr<VariableAccessASTNode>;
using FloorAssignmentASTNodePtr = std::shared_ptr<FloorAssignmentASTNode>;
using FloorAccessASTNodePtr = std::shared_ptr<FloorAccessASTNode>;

using AbstractExpressionASTNodePtr = std::shared_ptr<AbstractExpressionASTNode>;
using AbstractUnaryExpressionASTNodePtr = std::shared_ptr<AbstractUnaryExpressionASTNode>;
using AbstractPrimaryExpressionASTNodePtr = std::shared_ptr<AbstractPrimaryExpressionASTNode>;
using AbstractBinaryExpressionASTNodePtr = std::shared_ptr<AbstractBinaryExpressionASTNode>;
using NegativeExpressionASTNodePtr = std::shared_ptr<NegativeExpressionASTNode>;
using NotExpressionASTNodePtr = std::shared_ptr<NotExpressionASTNode>;
using IncrementExpressionASTNodePtr = std::shared_ptr<IncrementExpressionASTNode>;
using DecrementExpressionASTNodePtr = std::shared_ptr<DecrementExpressionASTNode>;
using AddExpressionASTNodePtr = std::shared_ptr<AddExpressionASTNode>;
using SubExpressionASTNodePtr = std::shared_ptr<SubExpressionASTNode>;
using MulExpressionASTNodePtr = std::shared_ptr<MulExpressionASTNode>;
using DivExpressionASTNodePtr = std::shared_ptr<DivExpressionASTNode>;
using ModExpressionASTNodePtr = std::shared_ptr<ModExpressionASTNode>;

using EqualExpressionASTNodePtr = std::shared_ptr<EqualExpressionASTNode>;
using NotEqualExpressionASTNodePtr = std::shared_ptr<NotEqualExpressionASTNode>;
using GreaterThanExpressionASTNodePtr = std::shared_ptr<GreaterThanExpressionASTNode>;
using GreaterEqualExpressionASTNodePtr = std::shared_ptr<GreaterEqualExpressionASTNode>;
using LessThanExpressionASTNodePtr = std::shared_ptr<LessThanExpressionASTNode>;
using LessEqualExpressionASTNodePtr = std::shared_ptr<LessEqualExpressionASTNode>;
using AndExpressionASTNodePtr = std::shared_ptr<AndExpressionASTNode>;
using OrExpressionASTNodePtr = std::shared_ptr<OrExpressionASTNode>;

using InvocationExpressionASTNodePtr = std::shared_ptr<InvocationExpressionASTNode>;
using EmptyStatementASTNodePtr = std::shared_ptr<EmptyStatementASTNode>;
using AbstractStatementASTNodePtr = std::shared_ptr<AbstractStatementASTNode>;
using AbstractEmbeddedStatementASTNodePtr = std::shared_ptr<AbstractEmbeddedStatementASTNode>;
using AbstractSelectionStatementASTNodePtr = std::shared_ptr<AbstractSelectionStatementASTNode>;
using AbstractIterationStatementASTNodePtr = std::shared_ptr<AbstractIterationStatementASTNode>;
using AbstractSubroutineASTNodePtr = std::shared_ptr<AbstractSubroutineASTNode>;

using StatementsVector = std::vector<AbstractStatementASTNodePtr>;
using IfStatementASTNodePtr = std::shared_ptr<IfStatementASTNode>;
using WhileStatementASTNodePtr = std::shared_ptr<WhileStatementASTNode>;
using ForStatementASTNodePtr = std::shared_ptr<ForStatementASTNode>;
using ReturnStatementASTNodePtr = std::shared_ptr<ReturnStatementASTNode>;
using FloorBoxInitStatementASTNodePtr = std::shared_ptr<FloorBoxInitStatementASTNode>;
using StatementBlockASTNodePtr = std::shared_ptr<StatementBlockASTNode>;

using AbstractSubroutineASTNodePtr = std::shared_ptr<AbstractSubroutineASTNode>;
using SubprocDefinitionASTNodePtr = std::shared_ptr<SubprocDefinitionASTNode>;
using FunctionDefinitionASTNodePtr = std::shared_ptr<FunctionDefinitionASTNode>;

using CompilationUnitASTNodePtr = std::shared_ptr<CompilationUnitASTNode>;

CLOSE_PARSER_NAMESPACE

#endif