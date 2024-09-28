#ifndef IRPROGRAMSTRUCTURE_H
#define IRPROGRAMSTRUCTURE_H

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

    ~BasicBlock();

    std::list<TACPtr> &get_instructions() { return _instructions; }

    const std::list<TACPtr> &get_instructions() const { return _instructions; }

    const std::string &get_label() const { return _label; }

private:
    std::string _label;
    std::list<TACPtr> _instructions;
};

using BasicBlockPtr = std::shared_ptr<BasicBlock>;
using ControlFlowGraph = boost::directed_graph<BasicBlockPtr>;
using ControlFlowGraphPtr = std::shared_ptr<ControlFlowGraph>;
using ControlFlowVertex = ControlFlowGraph::vertex_descriptor;
using ControlFlowEdge = ControlFlowGraph::edge_descriptor;

class Subroutine : public std::enable_shared_from_this<Subroutine> {
public:
    Subroutine(const std::string &func_name, bool has_param, bool has_return, const std::list<BasicBlockPtr> &basic_blocks)
        : _func_name(func_name)
        , _basic_blocks(basic_blocks)
        , _has_param(has_param)
        , _has_return(has_return)
        , _cfg(std::make_shared<ControlFlowGraph>())
        , _start_block(_cfg->null_vertex())
    {
    }

    Subroutine(std::string &&func_name, bool has_param, bool has_return, std::list<BasicBlockPtr> &&basic_blocks)
        : _func_name(std::move(func_name))
        , _basic_blocks(std::move(basic_blocks))
        , _has_param(has_param)
        , _has_return(has_return)
        , _cfg(std::make_shared<ControlFlowGraph>())
        , _start_block(_cfg->null_vertex())
    {
    }

    const std::string &get_func_name() const { return _func_name; }

    std::list<BasicBlockPtr> &get_basic_blocks() { return _basic_blocks; }

    const std::list<BasicBlockPtr> &get_basic_blocks() const { return _basic_blocks; }

    void set_cfg(const ControlFlowGraphPtr &cfg) { _cfg = cfg; }

    const ControlFlowGraphPtr &get_cfg() const { return _cfg; }

    void set_start_block(const ControlFlowVertex &start_block) { _start_block = start_block; }

    const ControlFlowVertex &get_start_block() const { return _start_block; }

    bool has_param() const { return _has_param; }

    bool has_return() const { return _has_return; }

    std::string generate_graphviz_cfg();

private:
    std::string _func_name;
    std::list<BasicBlockPtr> _basic_blocks;
    bool _has_param;
    bool _has_return;
    ControlFlowGraphPtr _cfg;
    ControlFlowVertex _start_block;
};

using SubroutinePtr = std::shared_ptr<Subroutine>;

class ProgramMetadata {
public:
    ProgramMetadata(int max_floor, const std::map<int, int> &floor_inits)
        : _floor_max(max_floor)
        , _floor_inits(floor_inits)
    {
    }

    void set_label_alias(const std::string &src, const std::string &tgt);
    bool get_label_alias(const std::string &src, std::string &tgt);

    const std::map<int, int> &get_floor_inits() const { return _floor_inits; }

    std::map<int, int> get_floor_inits() { return _floor_inits; }

    int get_floor_max() const { return _floor_max; };

private:
    std::map<std::string, std::string> _label_aliases;
    int _floor_max;
    std::map<int, int> _floor_inits;
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

    std::string generaet_graphviz();
    std::string to_string(bool with_color = false);

private:
    std::list<SubroutinePtr> _subroutines;
    ProgramMetadata _metadata;
};

using ProgramPtr = std::shared_ptr<Program>;

CLOSE_IRGEN_NAMESPACE
#endif