#ifndef THREEADDRESSCODE_H
#define THREEADDRESSCODE_H

#include <list>
#include <memory>
#include <string>

#include "IROps.h"
#include "Operand.h"
#include "irgen_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE
class ASTNode;
CLOSE_PARSER_NAMESPACE

OPEN_IRGEN_NAMESPACE

class ThreeAddressCode : public std::enable_shared_from_this<ThreeAddressCode> {
public:
    ~ThreeAddressCode() = default;

    IROperation get_op() const { return _op; }

    const Operand &get_src1() const { return _src1; }

    const Operand &get_src2() const { return _src2; }

    const Operand &get_tgt() const { return _tgt; }

    const std::shared_ptr<parser::ASTNode> &get_ast_node() const { return _ast; }

    std::string to_string(bool with_color = false) const;

    static std::shared_ptr<ThreeAddressCode> create_arithmetic(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_arithmetic(IROperation op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_comparison(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_logical(IROperation op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_logical(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_branching(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_branching(IROperation op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_branching(const Operand &tgt, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_data_movement(IROperation op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_load_immediate(const Operand &tgt, int imm, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_special(IROperation op, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_io(IROperation op, const Operand &val, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_call(const Operand &label, const Operand &param, const Operand &ret, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_call(const Operand &label, const Operand &ret, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_enter(const Operand &tgt, std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_return(std::shared_ptr<parser::ASTNode> ast = nullptr);
    static std::shared_ptr<ThreeAddressCode> create_return(const Operand &ret, std::shared_ptr<parser::ASTNode> ast = nullptr);

private:
    ThreeAddressCode(IROperation op, const Operand &tgt = Operand(), const Operand &src1 = Operand(), const Operand &src2 = Operand(), std::shared_ptr<parser::ASTNode> ast = nullptr)
        : _op(op)
        , _src1(src1)
        , _src2(src2)
        , _tgt(tgt)
        , _ast(ast)
    {
    }

    IROperation _op;
    Operand _src1;
    Operand _src2;
    Operand _tgt;
    std::shared_ptr<parser::ASTNode> _ast;
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