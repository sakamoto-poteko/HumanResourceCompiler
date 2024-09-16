#ifndef SEMANTICANALYSISPASS_H
#define SEMANTICANALYSISPASS_H

#include <concepts>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <vector>

#include "ASTNode.h"
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
    }

    virtual ~SemanticAnalysisPass() = default;

    virtual int run() = 0;

    // For all visit, the return value of 0 indicate success.
    int visit(const parser::IntegerASTNodePtr &node) override;
    int visit(const parser::BooleanASTNodePtr &node) override;
    int visit(const parser::VariableDeclarationASTNodePtr &node) override;
    int visit(const parser::VariableAssignmentASTNodePtr &node) override;
    int visit(const parser::VariableAccessASTNodePtr &node) override;
    int visit(const parser::FloorBoxInitStatementASTNodePtr &node) override;
    int visit(const parser::FloorAssignmentASTNodePtr &node) override;
    int visit(const parser::FloorAccessASTNodePtr &node) override;
    int visit(const parser::NegativeExpressionASTNodePtr &node) override;
    int visit(const parser::NotExpressionASTNodePtr &node) override;
    int visit(const parser::IncrementExpressionASTNodePtr &node) override;
    int visit(const parser::DecrementExpressionASTNodePtr &node) override;
    int visit(const parser::AddExpressionASTNodePtr &node) override;
    int visit(const parser::SubExpressionASTNodePtr &node) override;
    int visit(const parser::MulExpressionASTNodePtr &node) override;
    int visit(const parser::DivExpressionASTNodePtr &node) override;
    int visit(const parser::ModExpressionASTNodePtr &node) override;
    int visit(const parser::EqualExpressionASTNodePtr &node) override;
    int visit(const parser::NotEqualExpressionASTNodePtr &node) override;
    int visit(const parser::GreaterThanExpressionASTNodePtr &node) override;
    int visit(const parser::GreaterEqualExpressionASTNodePtr &node) override;
    int visit(const parser::LessThanExpressionASTNodePtr &node) override;
    int visit(const parser::LessEqualExpressionASTNodePtr &node) override;
    int visit(const parser::AndExpressionASTNodePtr &node) override;
    int visit(const parser::OrExpressionASTNodePtr &node) override;
    int visit(const parser::InvocationExpressionASTNodePtr &node) override;
    int visit(const parser::EmptyStatementASTNodePtr &node) override;
    int visit(const parser::IfStatementASTNodePtr &node) override;
    int visit(const parser::WhileStatementASTNodePtr &node) override;
    int visit(const parser::ForStatementASTNodePtr &node) override;
    int visit(const parser::ReturnStatementASTNodePtr &node) override;
    int visit(const parser::BreakStatementASTNodePtr &node) override;
    int visit(const parser::ContinueStatementASTNodePtr &node) override;
    int visit(const parser::StatementBlockASTNodePtr &node) override;
    int visit(const parser::SubprocDefinitionASTNodePtr &node) override;
    int visit(const parser::FunctionDefinitionASTNodePtr &node) override;
    int visit(const parser::CompilationUnitASTNodePtr &node) override;

protected:
    StringPtr _filename;
    parser::CompilationUnitASTNodePtr _root;

    std::vector<parser::ASTNodePtr> _ancestors;

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

    const parser::ASTNodePtr &topmost_node() const { return _ancestors.back(); }

    virtual void enter_node(const parser::ASTNodePtr &node);

    virtual void leave_node();

    void request_to_replace_self(parser::ASTNodePtr to_be_replaced_with);

    void request_to_remove_self();

    template <typename PostProcessFunc = std::function<void(const parser::ASTNodePtr &)>>
    void set_global_postprocess_function(PostProcessFunc postproc)
    {
        _global_post_process = postproc;
    }

    void clear_global_postprocess_function()
    {
        _global_post_process = nullptr;
    }

    template <typename ContainerT, typename PostProcessFunc = std::function<void(const parser::ASTNodePtr &)>>
        requires(std::ranges::range<ContainerT> && parser::ConvertibleToASTNodePtr<std::ranges::range_value_t<ContainerT>>)
    int traverse(
        ContainerT &nodes,
        PostProcessFunc post_process = [](const parser::ASTNodePtr &) {})
    {
        int result = 0;
        for (auto &node : nodes) {
            int rc = traverse(node, post_process);
            if (rc != 0) {
                std::erase_if(nodes, [](const auto &ptr) { return !ptr; });
                return rc;
            }
        }
        // this is to ensure that we do not have null elements in a vector
        std::erase_if(nodes, [](const auto &ptr) { return !ptr; });
        return result;
    }

    template <typename T, typename PostProcessFunc = std::function<void(const parser::ASTNodePtr &)>>
        requires parser::ConvertibleToASTNodePtr<T>
    int traverse(
        T &node, PostProcessFunc post_process = [](const parser::ASTNodePtr &) {})
    {
        if (node) {
            int rc = node->accept(this);
            // default post_process is empty, so as long as global is invalid, we execute this
            if (!_global_post_process) {
                post_process(node);
            } else {
                _global_post_process(node);
            }

            // is the node removal or replacement requestd?
            // check removal first, then replacement
            if (_node_removal_requests.contains(node)) {
                node = nullptr;
                _node_removal_requests.erase(node);
                return rc;
            }

            auto replacement_it = _node_replacement_requests.find(node);
            if (replacement_it != _node_replacement_requests.end()) {
                using NodeType = typename T::element_type;
                auto replaced_with = std::dynamic_pointer_cast<NodeType>(replacement_it->second);
                assert(replaced_with);
                node = replaced_with;
                _node_replacement_requests.erase(replacement_it);
                return rc;
            }

            return rc;
        } else {
            return 0;
        }
    }

    template <typename PostProcessFunc, typename... T>
        requires std::invocable<PostProcessFunc, const parser::ASTNodePtr &>
    int traverse_multiple(PostProcessFunc post_process, T &...node_ptr)
    {
        int result = 0;
        auto process_result = [&, post_process](auto &node) {
            if constexpr (parser::ConvertibleToASTNodePtr<std::remove_reference_t<decltype(node)>>) {
                int rc = traverse(node, post_process);
                if (rc != 0) {
                    result = rc;
                }
            } else if constexpr (std::ranges::range<decltype(node)> && parser::ConvertibleToASTNodePtr<std::ranges::range_value_t<decltype(node)>>) {
                int rc = traverse(node, post_process);
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

    template <typename... T>
    int traverse_multiple(T &...node_ptr)
    {
        return traverse_multiple([](const auto &) {}, node_ptr...);
    }

private:
    std::function<void(const parser::ASTNodePtr &)> _global_post_process;

    std::map<parser::ASTNodePtr, parser::ASTNodePtr> _node_replacement_requests;
    std::set<parser::ASTNodePtr> _node_removal_requests;
};

using SemanticAnalysisPassPtr = std::shared_ptr<SemanticAnalysisPass>;

template <typename T>
concept ConvertibleToSemanticAnalysisPassPtr = requires {
    typename T::element_type;
    requires std::convertible_to<T, SemanticAnalysisPassPtr> && std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif