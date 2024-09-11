#ifndef SEMANTICANALYSISPASS_H
#define SEMANTICANALYSISPASS_H

#include <memory>
#include <stack>
#include <type_traits>
#include <vector>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ASTNodeVisitor.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class SemanticAnalysisPass : public parser::ASTNodeVisitor {
public:
    SemanticAnalysisPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : _filename(std::move(filename))
        , _root(std::move(root))
    {
        _ancestors.reserve(1000);
    }

    virtual ~SemanticAnalysisPass() = default;

    virtual int run() = 0;

    // For all visit, the return value of 0 indicate success.
    int visit(parser::IntegerASTNodePtr node) override;
    int visit(parser::BooleanASTNodePtr node) override;
    int visit(parser::VariableDeclarationASTNodePtr node) override;
    int visit(parser::VariableAssignmentASTNodePtr node) override;
    int visit(parser::VariableAccessASTNodePtr node) override;
    int visit(parser::FloorBoxInitStatementASTNodePtr node) override;
    int visit(parser::FloorAssignmentASTNodePtr node) override;
    int visit(parser::FloorAccessASTNodePtr node) override;
    int visit(parser::NegativeExpressionASTNodePtr node) override;
    int visit(parser::NotExpressionASTNodePtr node) override;
    int visit(parser::IncrementExpressionASTNodePtr node) override;
    int visit(parser::DecrementExpressionASTNodePtr node) override;
    int visit(parser::AddExpressionASTNodePtr node) override;
    int visit(parser::SubExpressionASTNodePtr node) override;
    int visit(parser::MulExpressionASTNodePtr node) override;
    int visit(parser::DivExpressionASTNodePtr node) override;
    int visit(parser::ModExpressionASTNodePtr node) override;
    int visit(parser::EqualExpressionASTNodePtr node) override;
    int visit(parser::NotEqualExpressionASTNodePtr node) override;
    int visit(parser::GreaterThanExpressionASTNodePtr node) override;
    int visit(parser::GreaterEqualExpressionASTNodePtr node) override;
    int visit(parser::LessThanExpressionASTNodePtr node) override;
    int visit(parser::LessEqualExpressionASTNodePtr node) override;
    int visit(parser::AndExpressionASTNodePtr node) override;
    int visit(parser::OrExpressionASTNodePtr node) override;
    int visit(parser::InvocationExpressionASTNodePtr node) override;
    int visit(parser::EmptyStatementASTNodePtr node) override;
    int visit(parser::IfStatementASTNodePtr node) override;
    int visit(parser::WhileStatementASTNodePtr node) override;
    int visit(parser::ForStatementASTNodePtr node) override;
    int visit(parser::ReturnStatementASTNodePtr node) override;
    int visit(parser::BreakStatementASTNodePtr node) override;
    int visit(parser::ContinueStatementASTNodePtr node) override;
    int visit(parser::StatementBlockASTNodePtr node) override;
    int visit(parser::SubprocDefinitionASTNodePtr node) override;
    int visit(parser::FunctionDefinitionASTNodePtr node) override;
    int visit(parser::CompilationUnitASTNodePtr node) override;

protected:
    StringPtr _filename;
    parser::CompilationUnitASTNodePtr _root;

    template <typename NodeT>
    requires std::is_base_of_v<parser::ASTNode, NodeT>

    bool is_parent_a()
    {
        if (_ancestors.empty()) {
            return false;
        } else {
            return parser::is_ptr_type<NodeT>(_ancestors.back());
        }
    }

    const parser::ASTNodePtr &parent_node() const { return _ancestors.back(); }

    virtual void enter_node(parser::ASTNodePtr node);

    virtual void leave_node();

    void request_to_replace_self(parser::ASTNodePtr to_be_replaced_with);

    bool replace_self_requested() { return _replace_node_asked_by_child_guard.top() == 1; }

    template <typename ContainerT>

    requires(std::ranges::range<ContainerT> &&parser::convertible_to_ASTNodePtr<std::ranges::range_value_t<ContainerT>>) int traverse(ContainerT &nodes)
    {
        int result = 0;
        for (auto &node : nodes) {
            int rc = traverse(node);
            if (rc != 0) {
                result = rc;
            }
        }
        return result;
    }

    template <typename T>
    requires parser::convertible_to_ASTNodePtr<T>

    int traverse(T &node)
    {
        if (node) {
            int rc = node->accept(this);
            // if (false) {
            if (!_replace_node_asked_by_child.empty()) {
                using NodeType = typename T::element_type;
                auto top = std::dynamic_pointer_cast<NodeType>(_replace_node_asked_by_child.top());
                if (!top) {
                    // this is a bug.
                    throw;
                }
                node = top;
                _replace_node_asked_by_child.pop();
            }
            return rc;
        } else {
            return 0;
        }
    }

    template <typename... T>
    int traverse_multiple(T &...node_ptr)
    {
        int result = 0;
        auto process_result = [&](auto &node) {
            if constexpr (parser::convertible_to_ASTNodePtr<std::remove_reference_t<decltype(node)>>) {
                int rc = traverse(node);
                if (rc != 0) {
                    result = rc;
                }
            } else if constexpr (std::ranges::range<decltype(node)> && parser::convertible_to_ASTNodePtr<std::ranges::range_value_t<decltype(node)>>) {
                int rc = traverse(node);
                if (rc != 0) {
                    result = rc;
                }
            } else {
                static_assert(std::is_same_v<decltype(node), void>, "Unsupported node type");
            }
        };

        (process_result(node_ptr), ...);
        return result;
    }

private:
    std::vector<parser::ASTNodePtr> _ancestors;

    std::stack<parser::ASTNodePtr> _replace_node_asked_by_child;
    std::stack<int> _replace_node_asked_by_child_guard; // the count guard for single replace node
};

using SemanticAnalysisPassPtr = std::shared_ptr<SemanticAnalysisPass>;

template <typename T>
concept convertible_to_SemanticAnalysisPassPtr = requires
{
    typename T::element_type;
    requires std::convertible_to<T, SemanticAnalysisPassPtr> && std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif