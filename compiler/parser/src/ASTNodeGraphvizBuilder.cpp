#include <cstdlib>

#include <fstream>
#include <iostream>
#include <string>

#include <boost/graph/graphviz.hpp>

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
    boost::write_graphviz(dotfile, _graph, [this](std::ostream &out, auto &v) {
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
        }
    });

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
    node->get_var_name()->accept(this);
    node->get_expr()->accept(this);
    leave();
}

void ASTNodeGraphvizBuilder::visit(VariableAssignmentNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_var_name()->accept(this);
    node->get_expr()->accept(this);
    leave();
}

void ASTNodeGraphvizBuilder::visit(FloorAssignmentNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_floor_access()->accept(this);
    node->get_expr()->accept(this);
    leave();
}

void ASTNodeGraphvizBuilder::visit(BinaryExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());

    node->get_left()->accept(this);
    node->get_op()->accept(this);
    node->get_right()->accept(this);

    leave();
}

void ASTNodeGraphvizBuilder::visit(IncrementExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_var_name()->accept(this);
    leave();
}

void ASTNodeGraphvizBuilder::visit(DecrementExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_var_name()->accept(this);
    leave();
}

void ASTNodeGraphvizBuilder::visit(FloorAccessNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_index_expr()->accept(this);
    leave();
}

void ASTNodeGraphvizBuilder::visit(ParenthesizedExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_expr()->accept(this);
    leave();
}

void ASTNodeGraphvizBuilder::visit(InvocationExpressionNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_func_name()->accept(this);

    enter_and_create_vertex("Args");
    traverse(node->get_args());
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(IfStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    enter_and_create_vertex("Cond");
    node->get_condition()->accept(this);
    leave();

    enter_and_create_vertex("Then");
    node->get_then_stmt()->accept(this);
    leave();

    enter_and_create_vertex("Else");
    node->get_else_stmt()->accept(this);
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(WhileStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    enter_and_create_vertex("Cond");
    node->get_condition()->accept(this);
    leave();

    enter_and_create_vertex("Body");
    node->get_body()->accept(this);
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(ForStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    if (node->get_init_stmt()) {
        enter_and_create_vertex("Init");
        node->get_init_stmt()->accept(this);
        leave();
    }

    enter_and_create_vertex("Cond");
    node->get_condition()->accept(this);
    leave();

    enter_and_create_vertex("Update");
    node->get_update_stmt()->accept(this);
    leave();

    enter_and_create_vertex("Body");
    node->get_body()->accept(this);
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(ReturnStatementNodePtr node)
{
    enter_and_create_vertex(node->type());
    if (node->get_expr()) {
        node->get_expr()->accept(this);
    }
    leave();
}

void ASTNodeGraphvizBuilder::visit(FloorBoxInitStatementNodePtr node)
{
    enter_and_create_vertex(node->type());

    enter_and_create_vertex("Index");
    node->get_index()->accept(this);
    leave();

    enter_and_create_vertex("Value");
    node->get_value()->accept(this);
    leave();

    leave();
}

void ASTNodeGraphvizBuilder::visit(FloorMaxInitStatementNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_value()->accept(this);
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

    node->get_function_name()->accept(this);
    traverse(node->get_formal_parameters());
    node->get_body()->accept(this);

    leave();
}

void ASTNodeGraphvizBuilder::visit(FunctionDefinitionNodePtr node)
{
    enter_and_create_vertex(node->type());

    node->get_function_name()->accept(this);
    enter_and_create_vertex("Args");
    traverse(node->get_formal_parameters());
    leave();
    node->get_body()->accept(this);

    leave();
}

void ASTNodeGraphvizBuilder::visit(ImportDirectiveNodePtr node)
{
    enter_and_create_vertex(node->type());
    node->get_module_name()->accept(this);
    leave();
}

void ASTNodeGraphvizBuilder::visit(CompilationUnitNodePtr node)
{
    enter_and_create_vertex(node->type());

    node->get_floor_max()->accept(this);

    traverse(node->get_functions());
    traverse(node->get_imports());
    traverse(node->get_subprocs());
    traverse(node->get_top_level_decls());
    traverse(node->get_floor_inits());

    leave();
}

void ASTNodeGraphvizBuilder::visit(VariableDeclarationStatementNodePtr node)
{
    node->get_variable_decl()->accept(this);
}

void ASTNodeGraphvizBuilder::visit(VariableAssignmentStatementNodePtr node)
{
    node->get_variable_assignment()->accept(this);
}

void ASTNodeGraphvizBuilder::visit(FloorAssignmentStatementNodePtr node)
{
    node->get_floor_assignment()->accept(this);
};

CLOSE_PARSER_NAMESPACE
// end
