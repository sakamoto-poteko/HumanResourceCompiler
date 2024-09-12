#include <algorithm>
#include <fstream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <boost/graph/graphviz.hpp>

#include "ASTNodeAttribute.h"
#include "ASTNodeForward.h"
#include "ASTNodeGraphvizBuilder.h"
#include "hrl_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

ASTNodeGraphvizBuilder::ASTNodeGraphvizBuilder(CompilationUnitASTNodePtr ast)
    : _root(std::move(ast))
{
}

int ASTNodeGraphvizBuilder::leave()
{
    _ancestors.pop();
    return 0;
}

ASTNodeGraphvizBuilder::Vertex ASTNodeGraphvizBuilder::enter_and_create_vertex(const StringPtr &label, NodeType type, const ASTNodePtr &node)
{
    return enter_and_create_vertex(*label, type, node);
}

ASTNodeGraphvizBuilder::Vertex ASTNodeGraphvizBuilder::enter_and_create_vertex(const std::string &label, NodeType type, const ASTNodePtr &node)
{
    std::vector<std::string> attrs;

    if (node) {
        // filter out non-existent attr in node and null result

        /*
        std::ranges::copy(
            _enabled_attributes
                | std::views::transform([&node](const int &attr_id) -> std::optional<std::string> {
                      ASTNodeAttributePtr out;
                      if (node->get_attribute(attr_id, out)) {
                          return std::make_optional(out->to_string());
                      } else {
                          return std::nullopt;
                      }
                  })
                | std::views::filter([](const std::optional<std::string> &attr_opt) -> bool {
                      return attr_opt.has_value();
                  })
                | std::views::transform([](const std::optional<std::string> &attr_opt) {
                      return attr_opt.value();
                  }),
            std::back_inserter(attrs));
        */
        // It's shit above. Apparently below is cleaner.
        for (int attr_id : _enabled_attributes) {
            ASTNodeAttributePtr out;
            if (node->get_attribute(attr_id, out) && out) {
                attrs.push_back(out->to_string());
            }
        }
    }

    Vertex vertex = _graph.add_vertex(NodeProperty {
        .label = label,
        .type = type,
        .attributes = attrs,
    });

    if (!_ancestors.empty()) {
        _graph.add_edge(_ancestors.top(), vertex);
    }

    _ancestors.push(vertex);
    return vertex;
}

std::string ASTNodeGraphvizBuilder::escape_graphviz_html(const std::string &text)
{
    std::string escaped;
    for (char ch : text) {
        switch (ch) {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        case '\'':
            escaped += "&#39;";
            break;
        default:
            escaped += ch;
            break;
        }
    }
    return escaped;
}

std::string ASTNodeGraphvizBuilder::escape_graphviz(const std::string &text)
{
    std::string escaped;

    for (char it : text) {
        switch (it) {
        case '\\':
            escaped.append("\\\\");
            break;
        case '\n':
            escaped.append("\\n");
            break;
        case '"':
            escaped.append("\\\"");
            break;
        default:
            escaped.push_back(it);
            break;
        }
    }

    return escaped;
}

std::string ASTNodeGraphvizBuilder::generate_graphviz(const std::string &filepath, const std::set<int> enabled_attributes)
{
    _enabled_attributes = enabled_attributes;
    while (!_ancestors.empty()) {
        _ancestors.pop();
    }

    _graph.clear();
    _root->accept(this);

    std::stringstream dotfile;
    boost::write_graphviz(
        dotfile,
        _graph,
        // vertex
        [this](std::ostream &out, Vertex &v) {
            NodeProperty &node = _graph[v];

            std::vector<std::string> escaped_attrs;
            std::ranges::transform(node.attributes, std::back_inserter(escaped_attrs), [](const std::string &str) {
                return escape_graphviz_html(str);
            });

            std::string attrs = boost::join(escaped_attrs, "<BR/>");

            boost::format label_xlabel;
            if (attrs.empty()) {
                label_xlabel = boost::format(R"(label="%1%")")
                    % escape_graphviz(node.label);
            } else {
                label_xlabel = boost::format(R"(label=<%1%<BR/><FONT POINT-SIZE="10" COLOR="#8B4513">%2%</FONT>>)")
                    % escape_graphviz_html(node.label) % attrs;
            }

            switch (node.type) {
            case Literal:
                out << "[" << label_xlabel.str() << R"( shape=note style="filled" fillcolor=lightcoral fontname=Courier])";
                break;
            case Expression:
                out << "[" << label_xlabel.str() << R"( shape=rect style="rounded,filled" fillcolor=lightblue fontname=Helvetica])";
                break;
            case Statement:
                out << "[" << label_xlabel.str() << R"( shape=rect style="rounded,filled" fillcolor=lightgreen fontname=Helvetica])";
                break;
            case Structure:
                out << "[" << label_xlabel.str() << R"( shape=diamond style="filled" fillcolor=orange fontname=Helvetica])";
                break;
            case Operator:
                out << "[" << label_xlabel.str() << R"( shape=ellipse style="filled" fillcolor=lightblue fontname=Helvetica])";
                break;
            case Flow:
                out << "[" << label_xlabel.str() << R"( shape=cds style="filled" fillcolor=peachpuff fontname=Helvetica])";
                break;
            default:
                throw;
            }
        },
        // edge
        [](std::ostream &out, const Edge &e) {
            //
            UNUSED(out);
            UNUSED(e);
        },
        // graph
        [](std::ostream &out) { out << "node[ordering=out];\n"; });

    std::ofstream out(filepath);
    out << dotfile.str();
    out.close();

    return dotfile.str();
}

int ASTNodeGraphvizBuilder::visit(IntegerASTNodePtr node)
{
    // Graphviz logic for IntegerASTNode
    enter_and_create_vertex(std::to_string(node->get_value()), Literal, node);
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(BooleanASTNodePtr node)
{
    // Graphviz logic for BooleanASTNode
    enter_and_create_vertex(node->get_value() ? "TRUE" : "FALSE", Literal, node);
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(VariableDeclarationASTNodePtr node)
{
    // Graphviz logic for VariableDeclarationASTNode
    enter_and_create_vertex("Variable Declaration", Statement, node);

    enter_and_create_vertex(node->get_name(), Literal, nullptr);
    leave();

    traverse(node->get_assignment());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(VariableAssignmentASTNodePtr node)
{
    // Graphviz logic for VariableAssignmentASTNode
    enter_and_create_vertex("=", Statement, node);

    enter_and_create_vertex(node->get_name(), Literal, nullptr);
    leave();

    traverse(node->get_value());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(VariableAccessASTNodePtr node)
{
    // Graphviz logic for VariableAccessASTNode
    enter_and_create_vertex(node->get_name(), Literal, node);
    leave();

    return 0;
}

int ASTNodeGraphvizBuilder::visit(FloorBoxInitStatementASTNodePtr node)
{
    // Graphviz logic for FloorBoxInitStatementASTNode
    enter_and_create_vertex("Floor Init", Statement, node);

    traverse(node->get_assignment());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(FloorAssignmentASTNodePtr node)
{
    // Graphviz logic for FloorAssignmentASTNode
    enter_and_create_vertex("Floor=", Statement, node);

    traverse(node->get_floor_number());
    traverse(node->get_value());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(FloorAccessASTNodePtr node)
{
    // Graphviz logic for FloorAccessASTNode
    enter_and_create_vertex("floor[]", Expression, node);

    traverse(node->get_index_expr());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(NegativeExpressionASTNodePtr node)
{
    // Graphviz logic for NegativeExpressionASTNode
    enter_and_create_vertex("-", Operator, node);
    traverse(node->get_operand());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(NotExpressionASTNodePtr node)
{
    // Graphviz logic for NotExpressionASTNode
    enter_and_create_vertex("!", Operator, node);
    traverse(node->get_operand());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(IncrementExpressionASTNodePtr node)
{
    // Graphviz logic for IncrementExpressionASTNode
    enter_and_create_vertex("++", Operator, node);
    enter_and_create_vertex(node->get_var_name(), Literal, nullptr);
    leave();
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(DecrementExpressionASTNodePtr node)
{
    // Graphviz logic for DecrementExpressionASTNode
    enter_and_create_vertex("--", Operator, node);
    enter_and_create_vertex(node->get_var_name(), Literal, nullptr);
    leave();
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(AddExpressionASTNodePtr node)
{
    // Graphviz logic for AddExpressionASTNode
    enter_and_create_vertex("+", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(SubExpressionASTNodePtr node)
{
    // Graphviz logic for SubExpressionASTNode
    enter_and_create_vertex("-", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(MulExpressionASTNodePtr node)
{
    // Graphviz logic for MulExpressionASTNode
    enter_and_create_vertex("*", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(DivExpressionASTNodePtr node)
{
    // Graphviz logic for DivExpressionASTNode
    enter_and_create_vertex("/", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(ModExpressionASTNodePtr node)
{
    // Graphviz logic for ModExpressionASTNode
    enter_and_create_vertex("%", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(EqualExpressionASTNodePtr node)
{
    // Graphviz logic for EqualExpressionASTNode
    enter_and_create_vertex("==", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(NotEqualExpressionASTNodePtr node)
{
    // Graphviz logic for NotEqualExpressionASTNode
    enter_and_create_vertex("!=", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(GreaterThanExpressionASTNodePtr node)
{
    // Graphviz logic for GreaterThanExpressionASTNode
    enter_and_create_vertex(">", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(GreaterEqualExpressionASTNodePtr node)
{
    // Graphviz logic for GreaterEqualExpressionASTNode
    enter_and_create_vertex(">=", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(LessThanExpressionASTNodePtr node)
{
    // Graphviz logic for LessThanExpressionASTNode
    enter_and_create_vertex("<", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(LessEqualExpressionASTNodePtr node)
{
    // Graphviz logic for LessEqualExpressionASTNode
    enter_and_create_vertex("<=", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(AndExpressionASTNodePtr node)
{
    // Graphviz logic for AndExpressionASTNode
    enter_and_create_vertex("&&", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(OrExpressionASTNodePtr node)
{
    // Graphviz logic for OrExpressionASTNode
    enter_and_create_vertex("||", Operator, node);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(InvocationExpressionASTNodePtr node)
{
    // Graphviz logic for InvocationExpressionASTNode
    enter_and_create_vertex("Invocation", Expression, node);

    enter_and_create_vertex(node->get_func_name(), Literal, nullptr);
    leave();

    traverse(node->get_argument());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(EmptyStatementASTNodePtr node)
{
    // Graphviz logic for EmptyStatementASTNode

    enter_and_create_vertex("Empty", Statement, node);
    leave();

    return 0;
}

int ASTNodeGraphvizBuilder::visit(IfStatementASTNodePtr node)
{
    // Graphviz logic for IfStatementASTNode

    enter_and_create_vertex("If Stmt", Structure, node);

    enter_and_create_vertex("Cond", Statement, nullptr);
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Then", Statement, nullptr);
    traverse(node->get_then_branch());
    leave();

    enter_and_create_vertex("Else", Statement, nullptr);
    traverse(node->get_else_branch());
    leave();

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(WhileStatementASTNodePtr node)
{
    // Graphviz logic for WhileStatementASTNode
    enter_and_create_vertex("While Stmt", Structure, node);

    enter_and_create_vertex("Cond", Statement, nullptr);
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Body", Statement, nullptr);
    traverse(node->get_body());
    leave();

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(ForStatementASTNodePtr node)
{
    // Graphviz logic for ForStatementASTNode
    enter_and_create_vertex("For Stmt", Structure, node);

    enter_and_create_vertex("Init", Statement, nullptr);
    // it's either init_decl or init_assign
    traverse(node->get_init());
    leave();

    enter_and_create_vertex("Cond", Statement, nullptr);
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Update", Statement, nullptr);
    traverse(node->get_update());
    leave();

    enter_and_create_vertex("Body", Statement, nullptr);
    traverse(node->get_body());
    leave();

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(ReturnStatementASTNodePtr node)
{
    // Graphviz logic for ReturnStatementASTNode
    enter_and_create_vertex("Return", Flow, node);

    traverse(node->get_expression());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(BreakStatementASTNodePtr node)
{
    enter_and_create_vertex("Break", Flow, node);
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(ContinueStatementASTNodePtr node)
{
    enter_and_create_vertex("Continue", Flow, node);
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(StatementBlockASTNodePtr node)
{
    // Graphviz logic for StatementBlockASTNode
    enter_and_create_vertex("Block", Structure, node);

    traverse(node->get_statements());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(SubprocDefinitionASTNodePtr node)
{
    // Graphviz logic for SubprocDefinitionASTNode
    enter_and_create_vertex("Subproc", Structure, node);

    enter_and_create_vertex(node->get_name(), Literal, nullptr);
    leave();

    if (node->get_parameter()) {
        enter_and_create_vertex("Arg", Statement, nullptr);
        enter_and_create_vertex(node->get_parameter(), Literal, nullptr);
        leave();
        leave();
    }

    traverse(node->get_body());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(FunctionDefinitionASTNodePtr node)
{
    // Graphviz logic for FunctionDefinitionASTNode
    enter_and_create_vertex("Function", Structure, node);

    enter_and_create_vertex(node->get_name(), Literal, nullptr);
    leave();

    if (node->get_parameter()) {
        enter_and_create_vertex("Arg", Statement, nullptr);
        enter_and_create_vertex(node->get_parameter(), Literal, nullptr);
        leave();
        leave();
    }

    traverse(node->get_body());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(CompilationUnitASTNodePtr node)
{
    // Graphviz logic for CompilationUnitASTNode
    enter_and_create_vertex("Program", Structure, node);

    enter_and_create_vertex("Imports", Statement, nullptr);
    for (const auto &import : node->get_imports()) {
        enter_and_create_vertex(import, Literal, nullptr);
        leave();
    }
    leave();

    if (node->get_floor_max()) {
        enter_and_create_vertex("Floor Max", Statement, nullptr);
        enter_and_create_vertex(std::to_string(node->get_floor_max().value()), Literal, nullptr);
        leave();
        leave();
    }

    enter_and_create_vertex("Globals", Statement, nullptr);
    traverse(node->get_floor_inits());
    traverse(node->get_var_decls());
    leave();

    enter_and_create_vertex("Subroutines", Statement, nullptr);
    traverse(node->get_subroutines());
    leave();

    leave();
    return 0;
}

CLOSE_PARSER_NAMESPACE

// end
