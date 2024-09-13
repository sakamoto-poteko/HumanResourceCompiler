#ifndef WITHSYMBOLTABLE_H
#define WITHSYMBOLTABLE_H

#include "SymbolTable.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class WithSymbolTable {
public:
    WithSymbolTable() = default;
    virtual ~WithSymbolTable() = default;

    /**
     * @brief Set the symbol table object
     *
     * @param symbol_table  The existing table. This can be useful when the program has imports.
     */
    void set_symbol_table(SymbolTablePtr &symbol_table) { _symbol_table = symbol_table; }

    const SymbolTablePtr &get_symbol_table() const { return _symbol_table; }

protected:
    void init_symbol_table()
    {
        if (!_symbol_table) {
            _symbol_table = std::make_shared<SymbolTable>();
        }
    }

    SymbolTablePtr _symbol_table;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif