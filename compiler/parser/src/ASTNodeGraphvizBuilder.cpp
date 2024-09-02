#include <cstdlib>

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include <boost/graph/graphviz.hpp>

#include "ASTNode.h"
#include "ASTNodeGraphvizBuilder.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

ASTNodeGraphvizBuilder::Vertex ASTNodeGraphvizBuilder::enter_and_create_vertex(const std::string &name, const std::string &value, bool terminal)
{
    Vertex vertex = _graph.add_vertex(NodeProperty {
        .name = name,
        .terminal = terminal,
        .value = value });

    if (!_ancestors.empty()) {
        _graph.add_edge(_ancestors.top(), vertex);
    }

    _ancestors.push(vertex);
    return vertex;
}

ASTNodeGraphvizBuilder::Vertex ASTNodeGraphvizBuilder::enter_and_create_vertex(const std::string &name, bool terminal)
{
    return enter_and_create_vertex(name, std::string(), terminal);
}

void ASTNodeGraphvizBuilder::leave()
{
    _ancestors.pop();
}

std::string ASTNodeGraphvizBuilder::escape_graphviz(const std::string &text)
{
    std::string escaped;

    for (auto it = text.begin(); it != text.end(); ++it) {
        switch (*it) {
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
            escaped.push_back(*it);
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

        std::string label;
        if (node.value.empty()) {
            label = node.name;
        } else {
            label = node.value;
        }

        if (node.terminal) {
            out << "[label=\"" << escape_graphviz(label)
                << "\" shape=note style=\"filled\" fillcolor=lightcoral fontname=Courier]";
        } else {
            out << "[label=\"" << escape_graphviz(label)
                << "\" shape=rect style=\"rounded,filled\" fillcolor=lightgreen fontname=Helvetica]";
        } },
        // edge
        [](std::ostream &out, const Edge &e) {
            //
        },
        // graph

        [](std::ostream &out) { out << "node[ordering=out];\n"; });

    std::cout << std::endl
              << dotfile.str()
              << std::endl;

    std::ofstream out("build/out.dot");
    out << dotfile.str();
    out.close();

    return dotfile.str();
}

ASTNodeGraphvizBuilder::ASTNodeGraphvizBuilder(const CompilationUnitNodePtr &root)
    : _root(root)
{
}

ASTNodeGraphvizBuilder::~ASTNodeGraphvizBuilder()
{
}

void ASTNodeGraphvizBuilder::visit(IdentifierNodePtr node)
{
    enter_and_create_vertex(node->type(), *node->get_value(), true);
    leave();
}

void ASTNodeGraphvizBuilder::visit(IntegerLiteralNodePtr node)
{
    enter_and_create_vertex(node->type(), std::to_string(node->get_value()), true);
    leave();
}

void ASTNodeGraphvizBuilder::visit(BooleanLiteralNodePtr node)
{
    enter_and_create_vertex(node->type(), node->get_value() ? "true" : "false", true);
    leave();
}

void ASTNodeGraphvizBuilder::visit(BinaryOperatorNodePtr node)
{
    enter_and_create_vertex(node->type(), BinaryOperatorNode::get_binary_operator_string(node->get_op()), true);
    leave();
}

void ASTNodeGraphvizBuilder::visit(VariableDeclarationNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_var_name());
    traverse(node->get_expr());
    leave();
}

void ASTNodeGraphvizBuilder::visit(VariableAssignmentNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_var_name());
    traverse(node->get_expr());
    leave();
}

void ASTNodeGraphvizBuilder::visit(FloorAssignmentNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_floor_access());
    traverse(node->get_expr());
    leave();
}

void ASTNodeGraphvizBuilder::visit(BinaryExpressionNodePtr node)
{
    enter_and_create_vertex(node->type(), BinaryOperatorNode::get_binary_operator_string(node->get_op()->get_op()), false);

    traverse(node->get_left());
    traverse(node->get_right());

    leave();
}

void ASTNodeGraphvizBuilder::visit(IncrementExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_var_name());
    leave();
}

void ASTNodeGraphvizBuilder::visit(DecrementExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_var_name());
    leave();
}

void ASTNodeGraphvizBuilder::visit(FloorAccessNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_index_expr());
    leave();
}

void ASTNodeGraphvizBuilder::visit(ParenthesizedExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
}

void ASTNodeGraphvizBuilder::visit(InvocationExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_func_name());

    enter_and_create_vertex("Arg");
    traverse(node->get_arg());
    // traverse(node->get_arg());
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(IfStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    enter_and_create_vertex("Cond");
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Then");
    traverse(node->get_then_stmt());
    leave();

    enter_and_create_vertex("Else");
    traverse(node->get_else_stmt());
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(WhileStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    enter_and_create_vertex("Cond");
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Body");
    traverse(node->get_body());
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(ForStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    enter_and_create_vertex("Init");
    traverse(node->get_init_stmt());
    leave();

    enter_and_create_vertex("Cond");
    traverse(node->get_condition());
    leave();

    enter_and_create_vertex("Update");
    traverse(node->get_update_stmt());
    leave();

    enter_and_create_vertex("Body");
    traverse(node->get_body());
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(ReturnStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_expr());

    leave();
}

void ASTNodeGraphvizBuilder::visit(FloorBoxInitStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    enter_and_create_vertex("Index");
    traverse(node->get_index());
    leave();

    enter_and_create_vertex("Value");
    traverse(node->get_value());
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(FloorMaxInitStatementNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_value());
    leave();
}

void ASTNodeGraphvizBuilder::visit(EmptyStatementNodePtr node)
{
    enter_and_create_vertex(node->type());
    leave();
}

void ASTNodeGraphvizBuilder::visit(StatementBlockNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_statements());

    leave();
}

void ASTNodeGraphvizBuilder::visit(SubprocDefinitionNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_function_name());

    enter_and_create_vertex("Arg");
    traverse(node->get_formal_parameter());
    leave();

    traverse(node->get_body());

    leave();
}

void ASTNodeGraphvizBuilder::visit(FunctionDefinitionNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_function_name());

    enter_and_create_vertex("Arg");
    traverse(node->get_formal_parameter());
    leave();

    traverse(node->get_body());

    leave();
}

void ASTNodeGraphvizBuilder::visit(ImportDirectiveNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_module_name());
    leave();
}

void ASTNodeGraphvizBuilder::visit(CompilationUnitNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_floor_max());
    traverse(node->get_functions());
    traverse(node->get_imports());
    traverse(node->get_subprocs());
    traverse(node->get_top_level_decls());
    traverse(node->get_floor_inits());

    leave();
}

void ASTNodeGraphvizBuilder::visit(VariableDeclarationStatementNodePtr node)
{
    traverse(node->get_variable_decl());
}

void ASTNodeGraphvizBuilder::visit(VariableAssignmentStatementNodePtr node)
{
    traverse(node->get_variable_assignment());
}

void ASTNodeGraphvizBuilder::visit(FloorAssignmentStatementNodePtr node)
{
    traverse(node->get_floor_assignment());
};

void ASTNodeGraphvizBuilder::visit(NegativeExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
}

void ASTNodeGraphvizBuilder::visit(PositiveExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
}

void ASTNodeGraphvizBuilder::visit(NotExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
}

void ASTNodeGraphvizBuilder::visit(InvocationStatementNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
};

CLOSE_PARSER_NAMESPACE
// end
