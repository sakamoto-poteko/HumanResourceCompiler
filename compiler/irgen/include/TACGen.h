#ifndef TACGEN_H
#define TACGEN_H

#include <list>
#include <map>
#include <stack>
#include <string>

#include <boost/bimap.hpp>

#include "ASTNodeForward.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class TACGen : public semanalyzer::SemanticAnalysisPass {
public:
    ~TACGen() = default;

    TACGen(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : semanalyzer::SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    virtual int run() override;

    int get_max_floor();

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
    unsigned int _current_subroutine_var_id = 0;
    unsigned int _current_block_label_id = 0;
    std::string _current_subroutine_name;

    std::list<TACPtr> _current_subroutine_tac;
    std::list<TACPtr> _global_tacs;
    // map<func name, IRs>
    std::map<std::string, std::list<TACPtr>> _subroutine_tacs;

    // map<label, IR iter>
    struct tac_list_iter_comparator {
        bool operator()(const std::list<TACPtr>::iterator &it1, const std::list<TACPtr>::iterator &it2) const
        {
            return &(*it1) < &(*it2); // Compare based on the memory address of the pointed-to objects
        }
    };

    boost::bimap<std::string, boost::bimaps::set_of<std::list<TACPtr>::iterator, tac_list_iter_comparator>> _labels;
    // map<AST Node, result Operand>
    std::map<parser::ASTNodePtr, Operand> _node_var_id_result;
    // map<Symbol, Var Operand>
    std::map<semanalyzer::SymbolPtr, Operand> _symbol_to_var_map;

    std::stack<std::string> _loop_break_dest;
    std::stack<std::string> _loop_continue_dest;

    int take_var_id_numbering() { return _current_subroutine_var_id++; }

    std::string take_block_label();
    std::string take_block_label(const std::string &msg);
    // Create the label, set the node to _labels
    std::list<TACPtr>::iterator create_noop();
    std::list<TACPtr>::iterator create_jmp(const std::string &label);
    std::list<TACPtr>::iterator create_jnz(const Operand &operand, const std::string &label);
    std::list<TACPtr>::iterator create_jz(const Operand &operand, const std::string &label);
    std::list<TACPtr>::iterator create_instr(const TACPtr &instr);

    template <HighLevelIROps op>
    int visit_binary_expression(const parser::AbstractBinaryExpressionASTNodePtr &node);

    template <HighLevelIROps op>
    void create_binary_instr(const Operand &tgt, const Operand &src1, const Operand &src2)
    {
        if constexpr (op >= HighLevelIROps::ADD && op <= HighLevelIROps::MOD) {
            create_instr(ThreeAddressCode::createArithmetic(op, tgt, src1, src2));
        } else if constexpr (op >= HighLevelIROps::EQ && op <= HighLevelIROps::GE) {
            create_instr(ThreeAddressCode::createComparison(op, tgt, src1, src2));
        } else if constexpr (op == HighLevelIROps::AND || op == HighLevelIROps::OR) {
            create_instr(ThreeAddressCode::createLogical(op, tgt, src1, src2));
        } else {
            throw;
        }
    }

    int visit_subroutine(const parser::AbstractSubroutineASTNodePtr &node);
    void print_subroutine(const std::string &name, std::list<TACPtr> &tacs);
};

CLOSE_IRGEN_NAMESPACE

#endif