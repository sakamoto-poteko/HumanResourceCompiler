#ifndef IRPROGRAMSTRUCTURE_H
#define IRPROGRAMSTRUCTURE_H

#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <string>

#include <boost/bimap.hpp>
#include <boost/graph/directed_graph.hpp>

#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class BasicBlock : public std::enable_shared_from_this<BasicBlock> {
public:
    BasicBlock(std::string label, const std::list<TACPtr> &instructions)
        : _label(label)
        , _instructions(instructions)
    {
    }

    BasicBlock(std::string &&label, std::list<TACPtr> &&instructions)
        : _label(std::move(label))
        , _instructions(std::move(instructions))
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
    Subroutine(const std::string &func_name, bool has_param, bool has_return, const std::list<BasicBlockPtr> &basic_blocks, const ControlFlowGraph &cfg, ControlFlowVertex start)
        : _func_name(func_name)
        , _basic_blocks(basic_blocks)
        , _has_param(has_param)
        , _has_return(has_return)
        , _cfg(cfg)
        , _start_block(start)
    {
    }

    Subroutine(std::string &&func_name, bool has_param, bool has_return, std::list<BasicBlockPtr> &&basic_blocks, ControlFlowGraph &&cfg, ControlFlowVertex start_block)
        : _func_name(std::move(func_name))
        , _basic_blocks(std::move(basic_blocks))
        , _has_param(has_param)
        , _has_return(has_return)
        , _cfg(std::move(cfg))
        , _start_block(start_block)
    {
    }

    const std::string &get_func_name() const { return _func_name; }

    std::list<BasicBlockPtr> &get_basic_blocks() { return _basic_blocks; }

    const std::list<BasicBlockPtr> &get_basic_blocks() const { return _basic_blocks; }

    ControlFlowGraph &get_cfg() { return _cfg; }

    const ControlFlowGraph &get_cfg() const { return _cfg; }

    bool has_param() const { return _has_param; }

    bool has_return() const { return _has_return; }

    std::string generate_graphviz_cfg();

private:
    std::string _func_name;
    std::list<BasicBlockPtr> _basic_blocks;
    ControlFlowGraph _cfg;
    ControlFlowVertex _start_block;

    bool _has_param;
    bool _has_return;
};

using SubroutinePtr = std::shared_ptr<Subroutine>;

class ProgramMetadata {
public:
    void set_label_alias(const std::string &src, const std::string &tgt);
    bool get_label_alias(const std::string &src, std::string &tgt);

private:
    std::map<std::string, std::string> _label_aliases;
};

class Program {
public:
    Program(const std::list<SubroutinePtr> &subroutines, const ProgramMetadata &metadata)
        : _subroutines(subroutines)
        , _metadata(metadata)
    {
    }

    std::list<SubroutinePtr> &get_subroutines() { return _subroutines; }

    const std::list<SubroutinePtr> &get_subroutines() const { return _subroutines; }

    const ProgramMetadata &get_metadata() const { return _metadata; }

    ProgramMetadata &get_metadata() { return _metadata; }

private:
    std::list<SubroutinePtr> _subroutines;
    ProgramMetadata _metadata;
};

using ProgramPtr = std::shared_ptr<Program>;

CLOSE_IRGEN_NAMESPACE
#endif