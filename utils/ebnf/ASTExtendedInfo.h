#ifndef ASTEXTENDEDINFO_H
#define ASTEXTENDEDINFO_H

#include <map>
#include <string>

#include "ASTNode.h"

struct ASTExtendedInfo {
    std::map<std::string, ASTNodePtr> productions;
};

#endif