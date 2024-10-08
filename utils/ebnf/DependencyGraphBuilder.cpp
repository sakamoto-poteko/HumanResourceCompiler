#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>

#include <boost/format.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "DependencyGraphBuilder.h"
#include "ebnf_global.h"

// Quick & dirty
static std::string escapeGraphviz(const std::string &origin)
{
    std::string escaped;

    for (auto it = origin.begin(); it != origin.end(); ++it) {
        switch (*it) {
        case '\\':
            escaped.append("\\\\");
            break;
        case '\n':
            escaped.append("\\\\n");
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

bool DependencyGraphBuilder::build()
{
    _state = std::make_unique<VisitState>();

    for (const auto &prod : _root_node->productions) {
        if (_state->productions.find(prod->id) != _state->productions.end()) {
            std::cerr << "Rules are conflicting: " << prod->id << std::endl;
            return false;
        }
        _state->productions[prod->id] = boost::add_vertex(prod, _state->dependency_graph);
    }
    accept(_root_node);

    for (const auto &warning : _state->warnings) {
        std::cout << warning << std::endl;
    }
    return true;
}

bool DependencyGraphBuilder::write_graphviz(const std::string &path)
{
    if (!_state) {
        return false;
    }

    std::ofstream dotfile(path);

    boost::write_graphviz(
        dotfile, _state->dependency_graph,
        [this](std::ostream &out, Vertex &v) {
            const ASTNodePtr &node = _state->dependency_graph[v];
            if (auto syntax = std::dynamic_pointer_cast<SyntaxNode>(node)) {
                out << "[label=\"Syntax\" shape=diamond]";
            } else if (auto literal = std::dynamic_pointer_cast<LiteralNode>(node)) {
                out << "[label=\"" << escapeGraphviz(literal->value)
                    << "\" shape=note "
                       "style=filled fillcolor=lightgrey "
                       "fontname=\"Consolas\"]";
            } else if (auto identifier = std::dynamic_pointer_cast<IdentifierNode>(node)) {
                out << "[label=\""
                    << escapeGraphviz(identifier->value)
                    // << "\" shape=cds "
                    << "\" shape=note "
                       "style=filled fillcolor=lightgrey "
                       "fontname=\"Helvetica\"]";
            } else if (auto factor = std::dynamic_pointer_cast<FactorNode>(node)) {
                out << "[shape=point " // ANCHOR: point label=\"Factor\"
                       "style=filled fillcolor=lightyellow]";
            } else if (auto prod = std::dynamic_pointer_cast<ProductionNode>(node)) {
                out << "[label=\"" << escapeGraphviz(prod->id)
                    << "\" shape=ellipse style=filled fillcolor=lightblue "
                       "fontname=\"Helvetica\"]";
            } else if (auto expression = std::dynamic_pointer_cast<ExpressionNode>(node)) {
                out << "[label=\"\" shape=circle style=filled " // ANCHOR: point
                                                                // label=\"Expr\"
                       "fillcolor=lightcyan]";
            } else if (auto term = std::dynamic_pointer_cast<TermNode>(node)) {
                out << "[label=\"\" shape=circle style=filled " // ANCHOR:
                                                                // point
                       "fillcolor=lightgreen]";
            } else if (auto optional = std::dynamic_pointer_cast<OptionalNode>(node)) {
                out << "[label=\"Optional\" shape=rect style=filled "
                       "fillcolor=plum]";
            } else if (auto repeated = std::dynamic_pointer_cast<RepeatedNode>(node)) {
                out << "[label=\"Repeated\" shape=rect style=filled "
                       "fillcolor=lightsalmon]";
            } else if (auto grouped = std::dynamic_pointer_cast<GroupedNode>(node)) {
                out << "[label=\"Grouped\" shape=rect style=filled "
                       "fillcolor=lightcoral]";
            } else if (auto epsilon = std::dynamic_pointer_cast<EpsilonNode>(node)) {
                out << "[label=\"Epsilon\" shape=rect style=filled "
                       "fillcolor=gold]";
            } else {
                // expression with side effects will be evaluated despite being
                // used as an operand to 'typeid'
                auto &p = *node; // workaround
                out << "[label=\"" << typeid(p).name()
                    << "\" shape=rect style=\"rounded,filled\" fillcolor=red]";
            }
        },
        [this](std::ostream &out, const Edge &e) {
            const auto &source = _state->dependency_graph[boost::source(
                e, _state->dependency_graph)];
            const auto &target = _state->dependency_graph[boost::target(
                e, _state->dependency_graph)];
            if (auto t = std::dynamic_pointer_cast<FactorNode>(target)) {
                out << "[arrowtail=none arrowhead=none]";
            }
            if (auto s = std::dynamic_pointer_cast<ExpressionNode>(source)) {
                // expr source means alternatives
                out << "[color=limegreen style=bold arrowtail=none arrowhead=vee]";
            }
            // if (auto t = std::dynamic_pointer_cast<TermNode>(target)) {
            //     out << "[arrowtail=none arrowhead=none]";
            // }
            if (auto s = std::dynamic_pointer_cast<SyntaxNode>(source)) {
                out << "[style=dotted arrowhead=empty color=grey]";
            }
            if (auto s = std::dynamic_pointer_cast<IdentifierNode>(source)) {
                out << "[style=dashed arrowtail=inv arrowhead=open "
                       "color=crimson]";
            }
        },
        [](std::ostream &out) {
            out << "subgraph cluster_legend {\n"
                   "    label = Legend\n"
                   "    color = antiquewhite;\n"
                   "    style = filled;\n"
                   "    node [style = filled; fontsize = 12; width = 0.75; "
                   "fixedsize = true;];\n"
                   "    Syntax [shape = diamond;label = \"Syntax\";style = "
                   "filled;];\n"
                   "    Literal [shape = note;label = \"Literal\";fillcolor = "
                   "lightgrey;fontname = \"Consolas\";];\n"
                   "    Identifier [shape = note;label = "
                   "\"Identifier\";fillcolor = lightgrey;fontname = "
                   "\"Helvetica\";];\n"
                   "    Factor [shape = ellipse;label = \"Factor\";fillcolor = "
                   "lightblue;];\n"
                   "    Expression [shape = circle;label = \"Expr\";fillcolor "
                   "= lightcyan;];\n"
                   "    Term [shape = circle;label = \"Term\";fillcolor = "
                   "lightgreen;style = \"filled\";];\n"
                   "    Optional [shape = rect;label = \"Optional\";fillcolor "
                   "= plum;];\n"
                   "    Repeated [shape = rect;label = \"Repeated\";fillcolor "
                   "= lightsalmon;];\n"
                   "    Grouped [shape = rect;label = \"Grouped\";fillcolor = "
                   "lightcoral;];\n"
                   "    Production [shape = ellipse;label = \"Prod\";fillcolor "
                   "= lightblue;fontname = \"Helvetica\";];\n"
                   "    rankdir = \"TB\";\n"
                   "    Syntax -> Production [style = dotted; color = grey; "
                   "arrowhead = empty;];\n"
                   "    Production -> Expression;\n"
                   "    Expression -> Term [label = \"alternatives\"; "
                   "arrowtail = none; arrowhead = vee; color = limegreen; "
                   "style = bold;];\n"
                   "    Term -> Factor [arrowtail = none; arrowhead = none;];\n"
                   "    Factor -> Identifier;\n"
                   "    Factor -> Literal;\n"
                   "    Factor -> Expression;\n"
                   "    Identifier -> Production [style = dashed; color = "
                   "crimson; arrowhead = open; arrowtail = inv;];\n"
                   "    Term -> Optional;\n"
                   "    Term -> Repeated;\n"
                   "    Term -> Grouped;\n"
                   "}\n"
                   "node[ordering=out]\n";
        });

    dotfile.close();

    return true;
}

int DependencyGraphBuilder::accept(SyntaxNodePtr node)
{
    // Not adding Syntax node

    // Vertex v = boost::add_vertex(node, _state->dependency_graph);
    // _state->vertices.push(v);

    for (const auto &production : node->productions) {
        production->accept(this);
    }

    // _state->vertices.pop();
    return 0;
}

int DependencyGraphBuilder::accept(ProductionNodePtr node)
{
    _state->current_rule = node->id;
    Vertex v = _state->productions[node->id];
    // Not adding Syntax node
    // boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);
    _state->vertices.push(v);

    node->expression->accept(this);

    _state->vertices.pop();
    return 0;
}

int DependencyGraphBuilder::accept(ExpressionNodePtr node)
{
    // Expression is used to compute `FIRST` so it should always be kept.
    // All Expressions under a Production needs to evaluate its own FIRST and
    // FOLLOW
    Vertex v = boost::add_vertex(node, _state->dependency_graph);
    boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);
    _state->vertices.push(v);

    for (const auto &term : node->terms) {
        term->accept(this);
    }

    _state->vertices.pop();
    return 0;
}

int DependencyGraphBuilder::accept(TermNodePtr node)
{
    Vertex v = boost::add_vertex(node, _state->dependency_graph);
    boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);
    _state->vertices.push(v);

    for (const auto &factor : node->factors) {
        factor->accept(this);
    }

    _state->vertices.pop();
    return 0;
}

int DependencyGraphBuilder::accept(FactorNodePtr node)
{
    if (node->literal) {
        node->literal->accept(this);
    } else if (node->identifier) {
        node->identifier->accept(this);
    } else if (node->node) {
        node->node->accept(this);
    } else {
        spdlog::critical("node is not literal nor identifier nor having another node. {}", __PRETTY_FUNCTION__);
        throw;
    }
    return 0;
}

int DependencyGraphBuilder::accept(OptionalNodePtr node)
{
    Vertex v = boost::add_vertex(node, _state->dependency_graph);
    boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);
    _state->vertices.push(v);

    node->expression->accept(this);

    _state->vertices.pop();
    return 0;
}

int DependencyGraphBuilder::accept(RepeatedNodePtr node)
{
    Vertex v = boost::add_vertex(node, _state->dependency_graph);
    boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);
    _state->vertices.push(v);

    node->expression->accept(this);

    _state->vertices.pop();
    return 0;
}

int DependencyGraphBuilder::accept(GroupedNodePtr node)
{
    Vertex v = boost::add_vertex(node, _state->dependency_graph);
    boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);
    _state->vertices.push(v);

    node->expression->accept(this);

    _state->vertices.pop();
    return 0;
}

int DependencyGraphBuilder::accept(IdentifierNodePtr node)
{
    Vertex v = boost::add_vertex(node, _state->dependency_graph);
    boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);

    auto ref = _state->productions.find(node->value);
    // Is it neither found in productions nor tokens?
    if (ref == _state->productions.end()) {
        if (_tokens.find(node->value) == _tokens.end()) {
            // it's not a token. raise error
            auto msg = boost::format("Rule '%1%' is referenced in '%2%' (line %3%) but not defined. Is it a token?")
                % node->value % _state->current_rule % node->lineno();
            _state->warnings.push_back(msg.str());
        }
        // else it's a token. simply skip it.
    } else {
        boost::add_edge(v, ref->second, _state->dependency_graph);
    }
    return 0;
}

int DependencyGraphBuilder::accept(LiteralNodePtr node)
{
    Vertex v = boost::add_vertex(node, _state->dependency_graph);
    boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);
    return 0;
}

int DependencyGraphBuilder::accept(EpsilonNodePtr node)
{
    Vertex v = boost::add_vertex(node, _state->dependency_graph);
    boost::add_edge(_state->vertices.top(), v, _state->dependency_graph);
    return 0;
}
