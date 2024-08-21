#include <boost/graph/adjacency_list.hpp>
#include <stack>

#include "ASTDepGraphBuilderVisitor.h"

using namespace boost;

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, ASTNodePtr> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

Graph _dependency_graph;
std::stack<Vertex> _vertices;

ASTDepGraphBuilderVisitor::ASTDepGraphBuilderVisitor() {}

ASTDepGraphBuilderVisitor::~ASTDepGraphBuilderVisitor() {}

void ASTDepGraphBuilderVisitor::accept(std::shared_ptr<SyntaxNode> node) {
    Vertex v = boost::add_vertex(node, _dependency_graph);
    _vertices.push(v);
}
