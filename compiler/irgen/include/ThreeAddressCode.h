#ifndef THREEADDRESSCODE_H
#define THREEADDRESSCODE_H

#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>

#include "IROps.h"
#include "Operand.h"
#include "irgen_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE
class ASTNode;
CLOSE_PARSER_NAMESPACE

OPEN_IRGEN_NAMESPACE

class BasicBlock;
using BasicBlockPtr = std::shared_ptr<BasicBlock>;

class ThreeAddressCode : public std::enable_shared_from_this<ThreeAddressCode> {
public:
    ~ThreeAddressCode() = default;

    IROperation get_op() const { return _op; }

    const Operand &get_src1() const { return _src1; }

    const Operand &get_src2() const { return _src2; }

    const Operand &get_tgt() const { return _tgt; }

    const std::shared_ptr<parser::ASTNode> &get_ast_node() const { return _ast; }

    void set_phi_incoming(const BasicBlockPtr &predecessor, unsigned int var_id, const BasicBlockPtr &var_def_block) { _phi_incoming[predecessor] = std::make_tuple(var_id, var_def_block); }

    std::tuple<unsigned int, BasicBlockPtr> &get_phi_incoming(const BasicBlockPtr incoming) { return _phi_incoming.at(incoming); }

    std::map<BasicBlockPtr, std::tuple<unsigned int, BasicBlockPtr>> &get_phi_incomings() { return _phi_incoming; }

    void set_phi_incomings(const std::map<BasicBlockPtr, std::tuple<unsigned int, BasicBlockPtr>> &incomings) { _phi_incoming = incomings; }

    // get the variable(reg) use, which is useful in SSA
    std::set<Operand> get_variable_uses() const;
    // get the variable(reg) def, which is useful in SSA
    std::optional<Operand> get_variable_def() const;

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
    static std::shared_ptr<ThreeAddressCode> create_phi(int var_id, std::shared_ptr<parser::ASTNode> ast = nullptr);

    /**
     * @brief Create an instruction without any check
     *
     * @param op
     * @param tgt
     * @param src1
     * @param src2
     * @param ast
     * @return std::shared_ptr<ThreeAddressCode>
     */
    static std::shared_ptr<ThreeAddressCode> create(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast = nullptr);

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
    // map<predecessor block, <var id, var def block>>
    std::map<BasicBlockPtr, std::tuple<unsigned int, BasicBlockPtr>> _phi_incoming;
    std::shared_ptr<parser::ASTNode> _ast;
};

using TACPtr = std::shared_ptr<ThreeAddressCode>;
using InstructionListIter = std::list<TACPtr>::iterator;

bool operator<(const InstructionListIter &it1, const InstructionListIter &it2);

CLOSE_IRGEN_NAMESPACE

#endif