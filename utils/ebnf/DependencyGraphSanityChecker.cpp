#include "DependencyGraphSanityChecker.h"

bool DependencyGraphSanityChecker::check()
{
    _state = std::make_unique<VisitState>();

    for (int i = 0; i < _graph.num_vertices(); i++) {
        Vertex vertex = boost::vertex(i, _graph);
        const auto &current = _graph[vertex];
        if (const auto &ptr =
                std::dynamic_pointer_cast<ProductionNode>(current)) {
            _state->productions[ptr->id] = vertex;

            if (ptr->id == _root_symbol_name) {
                _state->root = vertex;
            }
        }
    }

    if (_state->root == Graph::null_vertex()) {
        throw; // TODO: err msg root syntax does not exist
    }

    // perform DFS with circular dependency check for all nodes
    soft_dfs(_state->root, Graph::null_vertex());

    // is there any nodes untraversed?
    for (int i = 0; i < _graph.num_vertices(); i++) {
        Vertex vertex = boost::vertex(i, _graph);
        if (_state->visited.find(vertex) == _state->visited.end()) {
            const auto &current = _graph[vertex];
            if (auto ptr = std::dynamic_pointer_cast<ProductionNode>(current)) {
                // unreachable production only. child elements of unreachable
                // are ignored as user doesn't care.
                _state->unreachable.push_back(
                    InfoWithLoc(current->lineno(), current->colno(), ptr));
            }
        }
    }

    return false;
}

int DependencyGraphSanityChecker::accept(SyntaxNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(ProductionNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(ExpressionNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(TermNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(RepeatedNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(FactorNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(OptionalNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(GroupedNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(IdentifierNodePtr node)
{
    return 0;
}

int DependencyGraphSanityChecker::accept(LiteralNodePtr node)
{
    return 0;
}

void DependencyGraphSanityChecker::soft_dfs(Vertex current, Vertex parent)
{
    const auto &node = _graph[current];

    if (_state->visited.find(current) != _state->visited.end()) {
        // already visited
        return;
    }

    if (_state->mark.find(current) != _state->mark.end()) {
        auto c = std::dynamic_pointer_cast<ProductionNode>(_graph[current]);
        auto p = std::dynamic_pointer_cast<IdentifierNode>(_graph[parent]);
        // circular dependency
        if (!c || !p) {
            throw;
            // TODO: this should not happen. our codepath only allow
            // current=ProductionNodePtr and parent=IdentifierNodePtr.
            // This limit is set in builder, because I built this dependency
            // myself!
        }
        // use the lineno for rule that's referencing it may be better
        _state->circular.push_back(InfoWithLoc(p->lineno(), p->colno(), std::make_pair(c, p)));
        return;
    }

    _state->mark.insert(current);

    auto out_edges_pair = boost::out_edges(current, _graph);
    for (auto it = out_edges_pair.first; it != out_edges_pair.second; ++it) {
        Vertex target = boost::target(*it, _graph);

        soft_dfs(target, current);
    }

    _state->mark.erase(current);
    _state->visited.insert(current);
    _state->topo.push_back(current);
}
