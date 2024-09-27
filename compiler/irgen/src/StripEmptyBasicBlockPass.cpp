#include <list>
#include <string>

#include "IROps.h"
#include "IRProgramStructure.h"
#include "Operand.h"
#include "StripEmptyBasicBlockPass.h"
#include "ThreeAddressCode.h"

OPEN_IRGEN_NAMESPACE

int StripEmptyBasicBlockPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    // map<erased label, next valid label>
    std::map<std::string, std::string> label_map;
    std::string last_valid_label;
    std::list<BasicBlockPtr> &basic_blocks = subroutine->get_basic_blocks();

    // not removing it here. erase reverse_iterator is very troublesome
    for (auto bb_it = basic_blocks.rbegin(); bb_it != basic_blocks.rend(); ++bb_it) {
        const BasicBlockPtr &bb = *bb_it;
        // iif this empty bb is not the last of subroutine
        // avoid jumping to somewhere that's all empty blocks
        if (bb->get_instructions().empty() && !last_valid_label.empty()) {
            label_map[bb->get_label()] = last_valid_label;
        } else {
            last_valid_label = bb->get_label();
        }
    }

    basic_blocks.remove_if([&](const BasicBlockPtr &bb) {
        return label_map.contains(bb->get_label());
    });

    for (const BasicBlockPtr &bb : basic_blocks) {
        for (TACPtr &instr : bb->get_instructions()) {
            // we're jumping to somewhere
            IROperation op = instr->get_op();
            const Operand &tgt = instr->get_tgt();
            if (is_branch_operation(op) && tgt.get_type() == Operand::OperandType::Label) {
                auto erased_valid_pair = label_map.find(tgt.get_label());
                // and this jumps to somewhere it's erased already
                if (erased_valid_pair != label_map.end()) {
                    auto new_instr = ThreeAddressCode::create(op, Operand(erased_valid_pair->second), instr->get_src1(), instr->get_src2(), instr->get_ast_node());
                    instr = new_instr;
                }
            }
        }
    }

    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
