#ifndef HIGHIRPROGRAM_H
#define HIGHIRPROGRAM_H

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/bimap.hpp>
#include <boost/graph/directed_graph.hpp>

#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

using tac_list_iter = std::list<TACPtr>::iterator;

class BasicBlock : public std::enable_shared_from_this<BasicBlock> {
public:
    BasicBlock(std::string label, const std::list<TACPtr> &instructions)
        : _label(label)
        , _instructions(instructions)
    {
    }

    ~BasicBlock() = default;

    std::list<TACPtr> &get_instructions() { return _instructions; }

    const std::list<TACPtr> &get_instructions() const { return _instructions; }

    const std::string &get_label() const { return _label; }

private:
    std::string _label;
    std::list<TACPtr> _instructions;
};

using BasicBlockPtr = std::shared_ptr<BasicBlock>;
using ControlFlowGraph = boost::directed_graph<BasicBlockPtr>;
using ControlFlowVertex = ControlFlowGraph::vertex_descriptor;
using ControlFlowEdge = ControlFlowGraph::edge_descriptor;

class Subroutine : public std::enable_shared_from_this<Subroutine> {
public:
    Subroutine(const std::string &func_name, bool has_param, bool has_return, const std::list<BasicBlockPtr> &basic_blocks, const ControlFlowGraph &cfg)
        : _func_name(func_name)
        , _basic_blocks(basic_blocks)
        , _has_param(has_param)
        , _has_return(has_return)
        , _cfg(cfg)
    {
    }

    const std::string &get_func_name() const { return _func_name; }

    std::list<BasicBlockPtr> &get_basic_blocks() { return _basic_blocks; }

    const std::list<BasicBlockPtr> &get_basic_blocks() const { return _basic_blocks; }

    ControlFlowGraph &get_cfg() { return _cfg; }

    const ControlFlowGraph &get_cfg() const { return _cfg; }

    bool has_param() const { return _has_param; }

    bool has_return() const { return _has_return; }

private:
    std::string _func_name;
    std::list<BasicBlockPtr> _basic_blocks;
    ControlFlowGraph _cfg;

    bool _has_param;
    bool _has_return;
};

using SubroutinePtr = std::shared_ptr<Subroutine>;

class Program {
public:
    Program(const std::list<SubroutinePtr> &subroutines)
        : _subroutines(subroutines)
    {
    }

    std::list<SubroutinePtr> &get_subroutines() { return _subroutines; }

    const std::list<SubroutinePtr> &get_subroutines() const { return _subroutines; }

private:
    std::list<SubroutinePtr> _subroutines;
};

CLOSE_IRGEN_NAMESPACE
#endif