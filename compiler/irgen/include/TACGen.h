#ifndef TACGEN_H
#define TACGEN_H

#include <list>
#include <map>
#include <stack>
#include <string>

#include <boost/bimap.hpp>

#include "ASTNodeForward.h"
#include "HRBox.h"
#include "IRProgramStructure.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "ThreeAddressCode.h"
#include "WithSymbolTable.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class TACGen : public semanalyzer::SemanticAnalysisPass, public semanalyzer::WithSymbolTable {
public:
    ~TACGen() = default;

    TACGen(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : semanalyzer::SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    virtual int run() override;

    ProgramPtr get_built_program() { return _built_program; }

    void print();

protected:
    // For all visit, the return value of 0 indicate success.
    int visit(const parser::IntegerASTNodePtr &node) override;
    int visit(const parser::BooleanASTNodePtr &node) override;
    int visit(const parser::VariableDeclarationASTNodePtr &node) override;
    int visit(const parser::VariableAssignmentASTNodePtr &node) override;
    int visit(const parser::VariableAccessASTNodePtr &node) override;
    int visit(const parser::FloorBoxInitStatementASTNodePtr &node) override;
    int visit(const parser::FloorAssignmentASTNodePtr &node) override;
    int visit(const parser::FloorAccessASTNodePtr &node) override;
    int visit(const parser::NegativeExpressionASTNodePtr &node) override;
    int visit(const parser::NotExpressionASTNodePtr &node) override;
    int visit(const parser::IncrementExpressionASTNodePtr &node) override;
    int visit(const parser::DecrementExpressionASTNodePtr &node) override;
    int visit(const parser::AddExpressionASTNodePtr &node) override;
    int visit(const parser::SubExpressionASTNodePtr &node) override;
    int visit(const parser::MulExpressionASTNodePtr &node) override;
    int visit(const parser::DivExpressionASTNodePtr &node) override;
    int visit(const parser::ModExpressionASTNodePtr &node) override;
    int visit(const parser::EqualExpressionASTNodePtr &node) override;
    int visit(const parser::NotEqualExpressionASTNodePtr &node) override;
    int visit(const parser::GreaterThanExpressionASTNodePtr &node) override;
    int visit(const parser::GreaterEqualExpressionASTNodePtr &node) override;
    int visit(const parser::LessThanExpressionASTNodePtr &node) override;
    int visit(const parser::LessEqualExpressionASTNodePtr &node) override;
    int visit(const parser::AndExpressionASTNodePtr &node) override;
    int visit(const parser::OrExpressionASTNodePtr &node) override;
    int visit(const parser::InvocationExpressionASTNodePtr &node) override;
    int visit(const parser::EmptyStatementASTNodePtr &node) override;
    int visit(const parser::IfStatementASTNodePtr &node) override;
    int visit(const parser::WhileStatementASTNodePtr &node) override;
    int visit(const parser::ForStatementASTNodePtr &node) override;
    int visit(const parser::ReturnStatementASTNodePtr &node) override;
    int visit(const parser::BreakStatementASTNodePtr &node) override;
    int visit(const parser::ContinueStatementASTNodePtr &node) override;
    int visit(const parser::StatementBlockASTNodePtr &node) override;
    int visit(const parser::SubprocDefinitionASTNodePtr &node) override;
    int visit(const parser::FunctionDefinitionASTNodePtr &node) override;
    int visit(const parser::CompilationUnitASTNodePtr &node) override;

private:
    // [Group] used for building
    unsigned int _current_subroutine_var_id = 0;
    unsigned int _current_block_label_id = 0;
    bool _in_global_var_decl = false;
    std::string _current_subroutine_name;
    std::list<TACPtr> _current_subroutine_tac;

    // map<Symbol, Var Operand>
    std::map<semanalyzer::SymbolPtr, Operand> _symbol_to_var_map;
    // map<AST Node, result Operand>
    std::map<parser::ASTNodePtr, Operand> _node_var_id_result;

    // the dest label for loop break
    std::stack<std::string> _loop_break_dest;
    // the dest label for loop continue
    std::stack<std::string> _loop_continue_dest;

    // [Group] program representatoin
    // map<func name, IRs>
    std::map<std::string, std::list<TACPtr>> _subroutine_tacs;
    // map<label, IR iter>
    boost::bimap<std::string, boost::bimaps::set_of<InstructionListIter>> _labels;
    // map<floor id, value>
    std::map<int, HRBox> _floor_inits;

    int take_var_id_numbering();
    std::string take_block_label();
    std::string take_block_label(const std::string &msg);

    // Create the label, set the node to _labels
    InstructionListIter create_noop(const parser::ASTNodePtr &node);
    InstructionListIter create_jmp(const std::string &label, const parser::ASTNodePtr &node);
    InstructionListIter create_jnz(const Operand &operand, const std::string &label, const parser::ASTNodePtr &node);
    InstructionListIter create_jz(const Operand &operand, const std::string &label, const parser::ASTNodePtr &node);
    InstructionListIter create_instr(const TACPtr &instr);

    template <IROperation op>
    int visit_binary_expression(const parser::AbstractBinaryExpressionASTNodePtr &node);

    template <IROperation op>
    void create_binary_instr(const Operand &tgt, const Operand &src1, const Operand &src2, const parser::ASTNodePtr &node)
    {
        if constexpr (op >= IROperation::ADD && op <= IROperation::MOD) {
            create_instr(ThreeAddressCode::create_arithmetic(op, tgt, src1, src2, node));
        } else if constexpr (op >= IROperation::EQ && op <= IROperation::GE) {
            create_instr(ThreeAddressCode::create_comparison(op, tgt, src1, src2, node));
        } else if constexpr (op == IROperation::AND || op == IROperation::OR) {
            create_instr(ThreeAddressCode::create_logical(op, tgt, src1, src2, node));
        } else {
            throw;
        }
    }

    int visit_subroutine(const parser::AbstractSubroutineASTNodePtr &node);
    void print_subroutine(const std::string &name, std::list<TACPtr> &tacs);

    std::list<BasicBlockPtr> build_subroutine_split_tacs_to_basic_blocks(const std::string &subroutine_name, std::list<TACPtr> &tacs);
    int build_ir_program();
    ProgramPtr _built_program;
};

CLOSE_IRGEN_NAMESPACE

#endif