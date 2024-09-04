#include <cstdlib>

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include <boost/graph/graphviz.hpp>

#include "ParseTreeNode.h"
#include "ParseTreeNodeGraphvizBuilder.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

ParseTreeNodeGraphvizBuilder::Vertex ParseTreeNodeGraphvizBuilder::enter_and_create_vertex(const std::string &name, const std::string &value, bool terminal)
{
    Vertex vertex = _graph.add_vertex(NodeProperty {
        .name = name,
        .value = value,
        .terminal = terminal,
    });

    if (!_ancestors.empty()) {
        _graph.add_edge(_ancestors.top(), vertex);
    }

    _ancestors.push(vertex);
    return vertex;
}

ParseTreeNodeGraphvizBuilder::Vertex ParseTreeNodeGraphvizBuilder::enter_and_create_vertex(const std::string &name, bool terminal)
{
    return enter_and_create_vertex(name, std::string(), terminal);
}

void ParseTreeNodeGraphvizBuilder::leave()
{
    _ancestors.pop();
}

std::string ParseTreeNodeGraphvizBuilder::escape_graphviz(const std::string &text)
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

std::string ParseTreeNodeGraphvizBuilder::generate_graphviz()
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

ParseTreeNodeGraphvizBuilder::ParseTreeNodeGraphvizBuilder(const CompilationUnitPTNodePtr &root)
    : _root(root)
{
}

ParseTreeNodeGraphvizBuilder::~ParseTreeNodeGraphvizBuilder()
{
}

void ParseTreeNodeGraphvizBuilder::visit(IdentifierPTNodePtr node)
{
    enter_and_create_vertex(node->type(), *node->get_value(), true);
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(IntegerLiteralPTNodePtr node)
{
    enter_and_create_vertex(node->type(), std::to_string(node->get_value()), true);
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(BooleanLiteralPTNodePtr node)
{
    enter_and_create_vertex(node->type(), node->get_value() ? "true" : "false", true);
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(BinaryOperatorPTNodePtr node)
{
    enter_and_create_vertex(node->type(), BinaryOperatorPTNode::get_binary_operator_string(node->get_op()), true);
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(VariableDeclarationPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_var_name());
    traverse(node->get_expr());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(VariableAssignmentPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_var_name());
    traverse(node->get_expr());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(FloorAssignmentPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_floor_access());
    traverse(node->get_expr());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(BinaryExpressionPTNodePtr node)
{
    enter_and_create_vertex(node->type(), BinaryOperatorPTNode::get_binary_operator_string(node->get_op()->get_op()), false);

    traverse(node->get_left());
    traverse(node->get_right());

    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(IncrementExpressionPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_var_name());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(DecrementExpressionPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_var_name());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(FloorAccessPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_index_expr());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(ParenthesizedExpressionPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(InvocationExpressionPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_func_name());

    enter_and_create_vertex("Arg");
    traverse(node->get_arg());
    // traverse(node->get_arg());
    leave();

    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(IfStatementPTNodePtr node)
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

void ParseTreeNodeGraphvizBuilder::visit(WhileStatementPTNodePtr node)
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

void ParseTreeNodeGraphvizBuilder::visit(ForStatementPTNodePtr node)
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

void ParseTreeNodeGraphvizBuilder::visit(ReturnStatementPTNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_expr());

    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(FloorBoxInitStatementPTNodePtr node)
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

void ParseTreeNodeGraphvizBuilder::visit(FloorMaxInitStatementPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_value());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(EmptyStatementPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(StatementBlockPTNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_statements());

    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(SubprocDefinitionPTNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_function_name());

    enter_and_create_vertex("Arg");
    traverse(node->get_formal_parameter());
    leave();

    traverse(node->get_body());

    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(FunctionDefinitionPTNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_function_name());

    enter_and_create_vertex("Arg");
    traverse(node->get_formal_parameter());
    leave();

    traverse(node->get_body());

    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(ImportDirectivePTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_module_name());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(CompilationUnitPTNodePtr node)
{
    enter_and_create_vertex(node->type());

    traverse(node->get_imports());
    traverse(node->get_floor_max());
    traverse(node->get_top_level_decls());
    traverse(node->get_floor_inits());
    traverse(node->get_subroutines());

    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(VariableDeclarationStatementPTNodePtr node)
{
    traverse(node->get_variable_decl());
}

void ParseTreeNodeGraphvizBuilder::visit(VariableAssignmentStatementPTNodePtr node)
{
    traverse(node->get_variable_assignment());
}

void ParseTreeNodeGraphvizBuilder::visit(FloorAssignmentStatementPTNodePtr node)
{
    traverse(node->get_floor_assignment());
};

void ParseTreeNodeGraphvizBuilder::visit(NegativeExpressionPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(PositiveExpressionPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(NotExpressionPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
}

void ParseTreeNodeGraphvizBuilder::visit(InvocationStatementPTNodePtr node)
{
    enter_and_create_vertex(node->type());
    traverse(node->get_expr());
    leave();
};

CLOSE_PARSER_NAMESPACE
// end
