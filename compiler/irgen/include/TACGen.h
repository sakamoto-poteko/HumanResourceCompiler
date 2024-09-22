#ifndef TACGEN_H
#define TACGEN_H

#include "ASTNodeForward.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "ThreeAddressCode.h"
#include "WithSymbolTable.h"
#include "irgen_global.h"
#include <list>
#include <map>
#include <string>

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
    int _current_subroutine_var_id = 0;
    std::list<TACPtr> _current_subroutine_tac;
    std::list<TACPtr> _global_tacs;
    // map<func name, IRs>
    std::map<std::string, std::list<TACPtr>> _subroutine_tacs;
    // map<label, IR iter>
    std::map<std::string, std::list<TACPtr>::iterator> _labels;
    // map<AST Node, result Operand>
    std::map<parser::ASTNodePtr, Operand> _node_var_id_result;
    // map<Symbol, Var Operand>
    std::map<semanalyzer::SymbolPtr, Operand> _symbol_to_var_map;

    int take_var_id_numbering() { return _current_subroutine_var_id++; }

    void restart_var_id_numbering() { _current_subroutine_var_id = 0; }

    template <HighLevelIROps op>
    int visit_binary_expression(const parser::AbstractBinaryExpressionASTNodePtr &node);
};

CLOSE_IRGEN_NAMESPACE

#endif