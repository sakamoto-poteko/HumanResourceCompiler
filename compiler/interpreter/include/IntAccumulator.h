#ifndef INT_ACCUMULATOR_H
#define INT_ACCUMULATOR_H

#include <optional>

#include "Symbol.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class IOManager;
class MemoryManager;

class Accumulator {
public:
    Accumulator(MemoryManager &memory, IOManager &ioman)
        : _memory(memory)
        , _ioman(ioman)
    {
    }

    ~Accumulator();

    void inbox();
    void outbox();

    enum class ArithmeticOperator {
        Add,
        Sub,
    };

    void arithmetic_operation(ArithmeticOperator op, const hrl::semanalyzer::SymbolPtr &symbol);
    void arithmetic_operation(ArithmeticOperator op, unsigned int floor_id);
    bool is_zero();
    bool is_not_zero();
    bool is_negative();
    bool is_true();
    int get_register();

    // this func is to enforce calling with int, instead of other implicitly convertible, such as bool
    template <typename T, typename = std::enable_if_t<std::is_same_v<T, int> && !std::is_same_v<T, bool>>>
    void set_register(T value)
    {
        __set_register(value);
    }

    void bumpup(const hrl::semanalyzer::SymbolPtr &symbol);
    void bumpup(unsigned int floor_id);
    void bumpdn(const hrl::semanalyzer::SymbolPtr &symbol);
    void bumpdn(unsigned int floor_id);

    void copy_to(const hrl::semanalyzer::SymbolPtr &symbol);
    void copy_to_floor(unsigned int floor_id);
    void copy_from(const hrl::semanalyzer::SymbolPtr &symbol);
    void copy_from_floor(unsigned int floor_id);

private:
    MemoryManager &_memory;
    IOManager &_ioman;
    std::optional<int> _register;

    void __set_register(int value);
    int __get_register();
    void __arithmetic_operation(ArithmeticOperator op, int target);


    void ensure_range(int value);
};

CLOSE_INTERPRETER_NAMESPACE

#endif