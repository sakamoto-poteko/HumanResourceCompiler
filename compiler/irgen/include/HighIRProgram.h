#ifndef HIGHIRPROGRAM_H
#define HIGHIRPROGRAM_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include <boost/bimap.hpp>

#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

using tac_list_iter = std::list<TACPtr>::iterator;

class BasicBlock {
public:
private:
    std::list<TACPtr> _instructions;
    std::string _label;
};

class HighIRProgram {
public:
    using label_instr_iter_bimap = boost::bimap<std::string, boost::bimaps::set_of<std::list<TACPtr>::iterator, tac_list_iter_comparator>>;

    HighIRProgram(const std::map<std::string, std::list<TACPtr>> &subroutine_ir, const label_instr_iter_bimap &label_map);
    ~HighIRProgram();

    std::list<TACPtr> &get_subroutine(const std::string &subroutine_name);
    // set the label map. replace old pointed with new iter
    void set_label(const std::string &label, const tac_list_iter &iter);
    // get the instr with label.
    bool get_instr(const std::string &label, tac_list_iter &iters);
    // get the label with instr
    bool get_label(const tac_list_iter &iter, std::vector<std::string> &labels);
    // remove the instr. if there's a label associated with, point the label to next instr. if it's the end, the label points to a new noop. return next instr
    tac_list_iter remove_instr(const tac_list_iter &iter);

private:
    // map<func name, IRs>
    std::map<std::string, std::list<TACPtr>> _subroutine_tacs;
    // map<label, IR iter>
    label_instr_iter_bimap _labels;
};

CLOSE_IRGEN_NAMESPACE
#endif