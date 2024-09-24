#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

void ProgramMetadata::set_label_alias(const std::string &src, const std::string &tgt)
{
    _label_aliases[src] = tgt;
}

bool ProgramMetadata::get_label_alias(const std::string &src, std::string &tgt)
{
    auto it = _label_aliases.find(src);
    if (it != _label_aliases.end()) {
        tgt = it->second;
        return true;
    } else {
        return false;
    }
}

CLOSE_IRGEN_NAMESPACE
// end
