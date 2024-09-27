#include <memory>
#include <spdlog/spdlog.h>

#include "ControlFlowGraphBuilder.h"
#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

ControlFlowGraphBuilder::~ControlFlowGraphBuilder()
{
}

int hrl::irgen::ControlFlowGraphBuilder::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    const std::list<BasicBlockPtr> &basic_blocks = subroutine->get_basic_blocks();

    ControlFlowGraphPtr cfgptr = std::make_shared<ControlFlowGraph>();
    ControlFlowGraph &cfg = *cfgptr;
    std::map<std::string, ControlFlowVertex> label_to_block;
    ControlFlowVertex start_block = ControlFlowGraph::null_vertex();

    // 1st pass: map label to bb vert
    // 2nd pass: build cfg
    for (auto bb_it = basic_blocks.begin(); bb_it != basic_blocks.end(); ++bb_it) {
        auto vert = cfg.add_vertex(*bb_it);
        label_to_block[(*bb_it)->get_label()] = vert;
    }

    for (auto bb_it = basic_blocks.begin(); bb_it != basic_blocks.end(); ++bb_it) {
        const auto &bb = *bb_it;
        ControlFlowVertex bb_vert = label_to_block[bb->get_label()];

        bool connect_next = false;
        bool connect_target = false;
        std::string target;

        // is the bb empty? if it is, we're connecting to next bb.
        if (bb->get_instructions().empty()) {
            connect_next = true;
            connect_target = false;
        } else {
            // bb is not empty. we'll need to check the final instr
            TACPtr flow_xfer_instr = *std::prev(bb->get_instructions().end());
            switch (flow_xfer_instr->get_op()) {
                // conditional branch
            case IROperation::JE:
            case IROperation::JNE:
            case IROperation::JGT:
            case IROperation::JLT:
            case IROperation::JGE:
            case IROperation::JLE:
            case IROperation::JZ:
            case IROperation::JNZ:
                connect_next = true;
                connect_target = true;
                target = flow_xfer_instr->get_tgt().get_label();
                break;

                // branch
            case IROperation::JMP:
                connect_next = false;
                connect_target = true;
                target = flow_xfer_instr->get_tgt().get_label();
                break;

            case IROperation::RET:
            case IROperation::HALT:
                connect_next = false;
                connect_target = false;
                break;

                // nothing yet. not drawing func graph yet.
            case IROperation::CALL:
            default:
                connect_next = true;
                connect_target = false;
                break;
            }
        }

        if (connect_next) {
            auto next_bb_it = std::next(bb_it);
            if (next_bb_it != basic_blocks.end()) {
                cfg.add_edge(bb_vert, label_to_block[(*next_bb_it)->get_label()]);
            }
        }

        if (connect_target) {
            auto tgt_it = label_to_block.find(target);
            if (tgt_it == label_to_block.end()) {
                spdlog::critical("BUG: target label '{}' does not exist in this block. {}", target, __PRETTY_FUNCTION__);
                throw;
            }
            cfg.add_edge(bb_vert, tgt_it->second);
        }
    }

    if (!basic_blocks.empty()) {
        start_block = label_to_block[basic_blocks.front()->get_label()];
        assert(cfg[start_block]);
    }

    subroutine->set_cfg(cfgptr);
    subroutine->set_start_block(start_block);
    return 0;
}

int ControlFlowGraphBuilder::run()
{
    return IROptimizationPass::run();
}

CLOSE_IRGEN_NAMESPACE
// end
