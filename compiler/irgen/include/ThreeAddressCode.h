#ifndef THREEADDRESSCODE_H
#define THREEADDRESSCODE_H

#include <list>
#include <memory>
#include <string>

#include "IROps.h"
#include "Operand.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class ThreeAddressCode : public std::enable_shared_from_this<ThreeAddressCode> {
public:
    ~ThreeAddressCode() = default;

    HighLevelIROps get_op() const { return _op; }

    const Operand &get_src1() const { return _src1; }

    const Operand &get_src2() const { return _src2; }

    const Operand &get_tgt() const { return _tgt; }

    std::string to_string() const;

    static std::shared_ptr<ThreeAddressCode> create_arithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> create_arithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> create_comparison(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> create_logical(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> create_logical(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> create_branching(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> create_branching(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> create_branching(const Operand &tgt);
    static std::shared_ptr<ThreeAddressCode> create_data_movement(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> create_load_immediate(const Operand &tgt, int imm);
    static std::shared_ptr<ThreeAddressCode> create_special(HighLevelIROps op);
    static std::shared_ptr<ThreeAddressCode> create_io(HighLevelIROps op, const Operand &val);
    static std::shared_ptr<ThreeAddressCode> create_call(const Operand &label, const Operand &param, const Operand &ret);
    static std::shared_ptr<ThreeAddressCode> create_call(const Operand &label, const Operand &ret);
    static std::shared_ptr<ThreeAddressCode> create_enter(const Operand &tgt);
    static std::shared_ptr<ThreeAddressCode> create_return();
    static std::shared_ptr<ThreeAddressCode> create_return(const Operand &ret);

private:
    ThreeAddressCode(HighLevelIROps op, const Operand &tgt = Operand(), const Operand &src1 = Operand(), const Operand &src2 = Operand())
        : _op(op)
        , _src1(src1)
        , _src2(src2)
        , _tgt(tgt)
    {
    }

    HighLevelIROps _op;
    Operand _src1;
    Operand _src2;
    Operand _tgt;
};

using TACPtr = std::shared_ptr<ThreeAddressCode>;

struct tac_list_iter_comparator {
    bool operator()(const std::list<TACPtr>::iterator &it1, const std::list<TACPtr>::iterator &it2) const
    {
        return &(*it1) < &(*it2); // Compare based on the memory address of the pointed-to objects
    }
};

CLOSE_IRGEN_NAMESPACE

#endif