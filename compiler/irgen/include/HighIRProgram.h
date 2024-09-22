#ifndef HIGHIRPROGRAM_H
#define HIGHIRPROGRAM_H

#include <list>
#include <map>

#include <boost/bimap.hpp>

#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class HighIRProgram {
public:
    using label_instr_iter_bimap = boost::bimap<std::string, boost::bimaps::set_of<std::list<TACPtr>::iterator, tac_list_iter_comparator>>;
    using instr_iter = std::list<TACPtr>::iterator;

    HighIRProgram(const std::map<std::string, std::list<TACPtr>> &subroutine_ir, const label_instr_iter_bimap &label_map);
    ~HighIRProgram();

    std::list<TACPtr> &get_subroutine(const std::string &subroutine_name);
    void set_label(const std::string &label, const instr_iter &iter);

private:
    // map<func name, IRs>
    std::map<std::string, std::list<TACPtr>> _subroutine_tacs;
    // map<label, IR iter>
    label_instr_iter_bimap _labels;
};

CLOSE_IRGEN_NAMESPACE
#endif