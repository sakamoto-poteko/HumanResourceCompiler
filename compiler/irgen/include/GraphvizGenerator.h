#ifndef GRAPHVIZGENERATOR_H
#define GRAPHVIZGENERATOR_H

#include <string>

#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class GraphvizGenerator {
public:
    static std::string generate_graphviz_cfg_for_subroutine(const ControlFlowGraph &cfg, const ControlFlowVertex start_block, const std::string &func_name);

private:
};

CLOSE_IRGEN_NAMESPACE

#endif