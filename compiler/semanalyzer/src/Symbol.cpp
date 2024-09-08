#include <boost/format.hpp>

#include "Symbol.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

std::string Symbol::to_string()
{
    std::string type_str;
    switch (type) {

    case SymbolType::VARIABLE:
        type_str = "Variable";
        break;
    case SymbolType::SUBROUTINE:
        type_str = "Subroutine";
        break;
    }
    auto str = boost::format("symbol: <%1%> %2%") % type_str % name;
    return str.str();
}

CLOSE_SEMANALYZER_NAMESPACE
