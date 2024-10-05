#ifndef IRPROGRAMSTRUCTURE_H
#define IRPROGRAMSTRUCTURE_H

#include <list>
#include <map>
#include <memory>
#include <string>

#include <boost/bimap.hpp>
#include <boost/graph/directed_graph.hpp>

#include "HRBox.h"
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

    unsigned int get_max_reg_id() const;

    /**
     * @brief Get the IN set: the set of variables that must be live at the entry of the block, either because they are used in the block or needed by its successors.
     *
     * @return const std::set<unsigned int>&
     */
    const std::set<unsigned int> &get_in_variables() const { return _in_variables; }

    /**
     * \copybrief get_in_variables
     *
     * @return std::set<unsigned int>&
     */
    std::set<unsigned int> &get_in_variables() { return _in_variables; }

    /**
     * @brief Get the OUT set: the set of variables that must be live at the exit of the block, meaning they are needed by its successor blocks.
     *
     * @return const std::set<unsigned int>&
     */
    const std::set<unsigned int> &get_out_variables() const { return _out_variables; }

    /**
     * \copybrief get_out_variables
     *
     * @return std::set<unsigned int>&
     */
    std::set<unsigned int> &get_out_variables() { return _out_variables; }

    /**
     * @brief Get the DEF set: The set of variables that are assigned values within the block.
     *
     * @return std::set<unsigned int>&
     */
    std::set<unsigned int> &get_def_variables() { return _def_variables; }

    /**
     * \copybrief get_def_variables
     *
     * @return const std::set<unsigned int>&
     */
    const std::set<unsigned int> &get_def_variables() const { return _def_variables; }

    /**
     * @brief Get the USE set: The set of variables that are used before being defined in the block.
     *
     * @return std::set<unsigned int>&
     */
    std::set<unsigned int> &get_use_variables() { return _use_variables; }

    /**
     * \copybrief get_use_variables
     *
     * @return const std::set<unsigned int>&
     */
    const std::set<unsigned int> &get_use_variables() const { return _use_variables; }

private:
    std::string _label;
    std::list<TACPtr> _instructions;

    std::set<unsigned int> _in_variables;
    std::set<unsigned int> _out_variables;

    std::set<unsigned int> _def_variables;
    std::set<unsigned int> _use_variables;
};

using BasicBlockPtr = std::shared_ptr<BasicBlock>;
using BBGraph = boost::directed_graph<BasicBlockPtr>;
using BBGraphPtr = std::shared_ptr<BBGraph>;
using BBGraphVertex = BBGraph::vertex_descriptor;
using BBGraphEdge = BBGraph::edge_descriptor;

class Subroutine : public std::enable_shared_from_this<Subroutine> {
public:
    Subroutine(const std::string &func_name, bool has_param, bool has_return, const std::list<BasicBlockPtr> &basic_blocks)
        : _func_name(func_name)
        , _basic_blocks(basic_blocks)
        , _has_param(has_param)
        , _has_return(has_return)
        , _cfg(std::make_shared<BBGraph>())
        , _cfg_entry(_cfg->null_vertex())
    {
    }

    Subroutine(std::string &&func_name, bool has_param, bool has_return, std::list<BasicBlockPtr> &&basic_blocks)
        : _func_name(std::move(func_name))
        , _basic_blocks(std::move(basic_blocks))
        , _has_param(has_param)
        , _has_return(has_return)
        , _cfg(std::make_shared<BBGraph>())
        , _cfg_entry(_cfg->null_vertex())
    {
    }

    const std::string &get_func_name() const { return _func_name; }

    std::list<BasicBlockPtr> &get_basic_blocks() { return _basic_blocks; }

    const std::list<BasicBlockPtr> &get_basic_blocks() const { return _basic_blocks; }

    void set_cfg(const BBGraphPtr &cfg, const BBGraphVertex entry_node)
    {
        _cfg = cfg;
        _cfg_entry = entry_node;
    }

    const BBGraphPtr &get_cfg() const { return _cfg; }

    const BBGraphVertex &get_cfg_entry() const { return _cfg_entry; }

    void set_dominance_tree(const BBGraphPtr &dom_tree, const BBGraphVertex root_node)
    {
        _domtree = dom_tree;
        _dom_tree_root = root_node;
    }

    const BBGraphPtr &get_dominance_tree() const { return _domtree; }

    const BBGraphVertex &get_dominance_root() const { return _dom_tree_root; }

    bool has_param() const { return _has_param; }

    bool has_return() const { return _has_return; }

    unsigned int get_max_reg_id() const;

    std::string generate_graphviz_cfg();

    std::map<unsigned int, std::set<BasicBlockPtr>> &get_def_variables() { return _def_variables; }

    const std::map<unsigned int, std::set<BasicBlockPtr>> &get_def_variables() const { return _def_variables; }

    std::map<unsigned int, std::set<BasicBlockPtr>> &get_use_variables() { return _use_variables; }

    const std::map<unsigned int, std::set<BasicBlockPtr>> &get_use_variables() const { return _use_variables; }

private:
    std::string _func_name;
    std::list<BasicBlockPtr> _basic_blocks;
    bool _has_param;
    bool _has_return;
    BBGraphPtr _cfg;
    BBGraphPtr _domtree;
    BBGraphVertex _cfg_entry = nullptr;
    BBGraphVertex _dom_tree_root = nullptr;

    std::map<unsigned int, std::set<BasicBlockPtr>> _def_variables;
    std::map<unsigned int, std::set<BasicBlockPtr>> _use_variables;
};

using SubroutinePtr = std::shared_ptr<Subroutine>;

class ProgramMetadata {
public:
    ProgramMetadata(int max_floor, const std::map<int, HRBox> &floor_inits)
        : _floor_max(max_floor)
        , _floor_inits(floor_inits)
    {
    }

    void set_label_alias(const std::string &src, const std::string &tgt);
    bool get_label_alias(const std::string &src, std::string &tgt);

    const std::map<int, HRBox> &get_floor_inits() const { return _floor_inits; }

    std::map<int, HRBox> get_floor_inits() { return _floor_inits; }

    int get_floor_max() const { return _floor_max; };

private:
    std::map<std::string, std::string> _label_aliases;
    int _floor_max;
    std::map<int, HRBox> _floor_inits;
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