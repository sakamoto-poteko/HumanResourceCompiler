#ifndef ParseTreeNODEFORWARD_H
#define ParseTreeNODEFORWARD_H

#include <memory>

class ParseTreeNode;
class SyntaxNode;
class ProductionNode;
class ExpressionNode;
class TermNode;
class FactorNode;
class OptionalNode;
class RepeatedNode;
class GroupedNode;
class LiteralNode;
class IdentifierNode;
class EpsilonNode;

using ParseTreeNodePtr = std::shared_ptr<ParseTreeNode>;
using SyntaxNodePtr = std::shared_ptr<SyntaxNode>;
using ProductionNodePtr = std::shared_ptr<ProductionNode>;
using ExpressionNodePtr = std::shared_ptr<ExpressionNode>;
using TermNodePtr = std::shared_ptr<TermNode>;
using FactorNodePtr = std::shared_ptr<FactorNode>;
using OptionalNodePtr = std::shared_ptr<OptionalNode>;
using RepeatedNodePtr = std::shared_ptr<RepeatedNode>;
using GroupedNodePtr = std::shared_ptr<GroupedNode>;
using IdentifierNodePtr = std::shared_ptr<IdentifierNode>;
using LiteralNodePtr = std::shared_ptr<LiteralNode>;
using EpsilonNodePtr = std::shared_ptr<EpsilonNode>;

#endif // !ParseTreeNODEFORWARD_H
