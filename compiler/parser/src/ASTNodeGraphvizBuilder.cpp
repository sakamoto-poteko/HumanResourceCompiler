#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/graph/graphviz.hpp>
#include <utility>

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

ASTNodeGraphvizBuilder::Vertex ASTNodeGraphvizBuilder::enter_and_create_vertex(const StringPtr &label, NodeType type)
{
    return enter_and_create_vertex(*label, type);
}

ASTNodeGraphvizBuilder::Vertex ASTNodeGraphvizBuilder::enter_and_create_vertex(const std::string &label, NodeType type)
{
    Vertex vertex = _graph.add_vertex(NodeProperty {
        .label = label,
        .type = type,
    });

    if (!_ancestors.empty()) {
        _graph.add_edge(_ancestors.top(), vertex);
    }

    _ancestors.push(vertex);
    return vertex;
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

std::string ASTNodeGraphvizBuilder::generate_graphviz()
{
    _root->accept(this);

    std::stringstream dotfile;
    boost::write_graphviz(
        dotfile,
        _graph,
        // vertex
        [this](std::ostream &out, Vertex &v) {
            NodeProperty &node = _graph[v];

            switch (node.type) {
            case Literal:
                out << "[label=\"" << escape_graphviz(node.label)
                    << R"(" shape=note style="filled" fillcolor=lightcoral fontname=Courier])";
                break;
            case Expression:
                out << "[label=\"" << escape_graphviz(node.label)
                    << R"(" shape=rect style="rounded,filled" fillcolor=lightblue fontname=Helvetica])";
                break;
            case Statement:
                out << "[label=\"" << escape_graphviz(node.label)
                    << R"(" shape=rect style="rounded,filled" fillcolor=lightgreen fontname=Helvetica])";
                break;
            case Structure:
                out << "[label=\"" << escape_graphviz(node.label)
                    << R"(" shape=diamond style="filled" fillcolor=orange fontname=Helvetica])";
                break;
            case Operator:
                out << "[label=\"" << escape_graphviz(node.label)
                    << R"(" shape=circle style="filled" fillcolor=lightblue fontname=Helvetica])";
                break;
            default:
                throw;
            }
        },
        // edge
        [](std::ostream &out, const Edge &e) {
            //
        },
        // graph

        [](std::ostream &out) { out << "node[ordering=out];\n"; });

    std::cout << std::endl
              << dotfile.str()
              << std::endl;

    std::ofstream out("build/ast.dot");
    out << dotfile.str();
    out.close();

    return dotfile.str();
}

int ASTNodeGraphvizBuilder::visit(IntegerASTNodePtr node)
{
    // Graphviz logic for IntegerASTNode
    enter_and_create_vertex(std::to_string(node->get_value()), Literal);
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(BooleanASTNodePtr node)
{
    // Graphviz logic for BooleanASTNode
    enter_and_create_vertex(node->get_value() ? "TRUE" : "FALSE", Literal);
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(VariableDeclarationASTNodePtr node)
{
    // Graphviz logic for VariableDeclarationASTNode
    enter_and_create_vertex("Variable Declaration", Statement);

    enter_and_create_vertex(node->get_name(), Literal);
    leave();

    traverse(node->get_assignment());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(VariableAssignmentASTNodePtr node)
{
    // Graphviz logic for VariableAssignmentASTNode
    enter_and_create_vertex("=", Statement);

    enter_and_create_vertex(node->get_name(), Literal);
    leave();

    traverse(node->get_value());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(VariableAccessASTNodePtr node)
{
    // Graphviz logic for VariableAccessASTNode
    enter_and_create_vertex(node->get_name(), Literal);
    leave();

    return 0;
}

int ASTNodeGraphvizBuilder::visit(FloorBoxInitStatementASTNodePtr node)
{
    // Graphviz logic for FloorBoxInitStatementASTNode
    enter_and_create_vertex("Floor Init", Statement);

    traverse(node->get_assignment());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(FloorAssignmentASTNodePtr node)
{
    // Graphviz logic for FloorAssignmentASTNode
    enter_and_create_vertex("Floor=", Statement);

    traverse(node->get_floor_number());
    traverse(node->get_value());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(FloorAccessASTNodePtr node)
{
    // Graphviz logic for FloorAccessASTNode
    enter_and_create_vertex("floor[]", Expression);

    traverse(node->get_index_expr());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(NegativeExpressionASTNodePtr node)
{
    // Graphviz logic for NegativeExpressionASTNode
    enter_and_create_vertex("-", Operator);
    traverse(node->get_operand());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(NotExpressionASTNodePtr node)
{
    // Graphviz logic for NotExpressionASTNode
    enter_and_create_vertex("!", Operator);
    traverse(node->get_operand());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(IncrementExpressionASTNodePtr node)
{
    // Graphviz logic for IncrementExpressionASTNode
    enter_and_create_vertex("++", Operator);
    enter_and_create_vertex(node->get_var_name(), Literal);
    leave();
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(DecrementExpressionASTNodePtr node)
{
    // Graphviz logic for DecrementExpressionASTNode
    enter_and_create_vertex("--", Operator);
    enter_and_create_vertex(node->get_var_name(), Literal);
    leave();
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(AddExpressionASTNodePtr node)
{
    // Graphviz logic for AddExpressionASTNode
    enter_and_create_vertex("+", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(SubExpressionASTNodePtr node)
{
    // Graphviz logic for SubExpressionASTNode
    enter_and_create_vertex("-", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(MulExpressionASTNodePtr node)
{
    // Graphviz logic for MulExpressionASTNode
    enter_and_create_vertex("*", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(DivExpressionASTNodePtr node)
{
    // Graphviz logic for DivExpressionASTNode
    enter_and_create_vertex("/", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(ModExpressionASTNodePtr node)
{
    // Graphviz logic for ModExpressionASTNode
    enter_and_create_vertex("%", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(EqualExpressionASTNodePtr node)
{
    // Graphviz logic for EqualExpressionASTNode
    enter_and_create_vertex("==", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(NotEqualExpressionASTNodePtr node)
{
    // Graphviz logic for NotEqualExpressionASTNode
    enter_and_create_vertex("!=", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(GreaterThanExpressionASTNodePtr node)
{
    // Graphviz logic for GreaterThanExpressionASTNode
    enter_and_create_vertex(">", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(GreaterEqualExpressionASTNodePtr node)
{
    // Graphviz logic for GreaterEqualExpressionASTNode
    enter_and_create_vertex(">=", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(LessThanExpressionASTNodePtr node)
{
    // Graphviz logic for LessThanExpressionASTNode
    enter_and_create_vertex("<", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(LessEqualExpressionASTNodePtr node)
{
    // Graphviz logic for LessEqualExpressionASTNode
    enter_and_create_vertex("<=", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(AndExpressionASTNodePtr node)
{
    // Graphviz logic for AndExpressionASTNode
    enter_and_create_vertex("&&", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(OrExpressionASTNodePtr node)
{
    // Graphviz logic for OrExpressionASTNode
    enter_and_create_vertex("||", Operator);
    traverse(node->get_left());
    traverse(node->get_right());
    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(InvocationExpressionASTNodePtr node)
{
    // Graphviz logic for InvocationExpressionASTNode
    enter_and_create_vertex("Invocation", Expression);

    enter_and_create_vertex(node->get_func_name(), Literal);
    leave();

    traverse(node->get_argument());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(EmptyStatementASTNodePtr node)
{
    // Graphviz logic for EmptyStatementASTNode

    // Empty. Do nothing
    return 0;
}

int ASTNodeGraphvizBuilder::visit(IfStatementASTNodePtr node)
{
    // Graphviz logic for IfStatementASTNode

    enter_and_create_vertex("If Stmt", Structure);

    enter_and_create_vertex("Cond", Statement);
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Then", Statement);
    traverse(node->get_then_branch());
    leave();

    enter_and_create_vertex("Else", Statement);
    traverse(node->get_else_branch());
    leave();

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(WhileStatementASTNodePtr node)
{
    // Graphviz logic for WhileStatementASTNode
    enter_and_create_vertex("While Stmt", Structure);

    enter_and_create_vertex("Cond", Statement);
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Body", Statement);
    traverse(node->get_body());
    leave();

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(ForStatementASTNodePtr node)
{
    // Graphviz logic for ForStatementASTNode
    enter_and_create_vertex("For Stmt", Structure);

    enter_and_create_vertex("Init", Statement);
    // it's either init_decl or init_assign
    traverse(node->get_init());
    leave();

    enter_and_create_vertex("Cond", Statement);
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Update", Statement);
    traverse(node->get_update());
    leave();

    enter_and_create_vertex("Body", Statement);
    traverse(node->get_body());
    leave();

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(ReturnStatementASTNodePtr node)
{
    // Graphviz logic for ReturnStatementASTNode
    enter_and_create_vertex("Return", Statement);

    traverse(node->get_expression());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(StatementBlockASTNodePtr node)
{
    // Graphviz logic for StatementBlockASTNode
    enter_and_create_vertex("Block", Structure);

    traverse(node->get_statements());

    leave();
    return 0;
}

int ASTNodeGraphvizBuilder::visit(SubprocDefinitionASTNodePtr node)
{
    // Graphviz logic for SubprocDefinitionASTNode
    enter_and_create_vertex("Subproc", Structure);

    enter_and_create_vertex(node->get_name(), Literal);
    leave();

    if (node->get_parameter()) {
        enter_and_create_vertex("Arg", Statement);
        enter_and_create_vertex(node->get_parameter(), Literal);
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
    enter_and_create_vertex("Function", Structure);

    enter_and_create_vertex(node->get_name(), Literal);
    leave();

    if (node->get_parameter()) {
        enter_and_create_vertex("Arg", Statement);
        enter_and_create_vertex(node->get_parameter(), Literal);
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
    enter_and_create_vertex("Program", Structure);

    enter_and_create_vertex("Imports", Statement);
    for (const auto &import : node->get_imports()) {
        enter_and_create_vertex(import, Literal);
        leave();
    }
    leave();

    if (node->get_floor_max()) {
        enter_and_create_vertex("Floor Max", Statement);
        enter_and_create_vertex(std::to_string(node->get_floor_max().value()), Literal);
        leave();
        leave();
    }

    enter_and_create_vertex("Globals", Statement);
    traverse(node->get_floor_inits());
    traverse(node->get_var_decls());
    leave();

    enter_and_create_vertex("Subroutines", Statement);
    traverse(node->get_subroutines());
    leave();

    leave();
    return 0;
}

CLOSE_PARSER_NAMESPACE

// end
