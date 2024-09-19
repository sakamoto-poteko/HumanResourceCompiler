#include <algorithm>
#include <cassert>
#include <memory>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "Interpreter.h"
#include "InterpreterExceptions.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "hrl_global.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

#define BEGIN_VISIT() \
    enter_node(node); \
    int rc = 0;       \
    UNUSED(rc);

#define END_VISIT() \
    leave_node();   \
    return rc;

#define RETURN_IF_FAIL_IN_VISIT(rc) \
    if (rc > 0) {                   \
        leave_node();               \
        return rc;                  \
    }

#define RETURN_IF_ABNORMAL_RC_IN_VISIT(rc) \
    if (rc != 0) {                         \
        leave_node();                      \
        return rc;                         \
    }

Interpreter::~Interpreter()
{
}

int Interpreter::visit(const parser::IntegerASTNodePtr &node)
{
    BEGIN_VISIT();
    _accumulator.set_register(node->get_value());
    END_VISIT();
}

int Interpreter::visit(const parser::BooleanASTNodePtr &node)
{
    BEGIN_VISIT();
    _accumulator.set_register(node->get_value() ? 1 : 0);
    END_VISIT();
}

int Interpreter::visit(const parser::VariableDeclarationASTNodePtr &node)
{
    BEGIN_VISIT();

    spdlog::debug("declared variable '{}'", *node->get_name());
    rc = traverse(node->get_assignment());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);

    END_VISIT();
}

int Interpreter::visit(const parser::VariableAssignmentASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);

    rc = traverse(node->get_value());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);

    spdlog::debug("assigned variable '{}' with value {}", symbol->name, _accumulator.get_register());
    _accumulator.copy_to(symbol);

    END_VISIT();
}

int Interpreter::visit(const parser::VariableAccessASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);
    _accumulator.copy_from(symbol);

    spdlog::debug("loaded variable '{}' with value {}", symbol->name, _accumulator.get_register());

    END_VISIT();
}

int Interpreter::visit(const parser::FloorBoxInitStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_assignment());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);

    END_VISIT();
}

int Interpreter::visit(const parser::FloorAssignmentASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_value());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    int value = _accumulator.get_register();

    rc = traverse(node->get_floor_number());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    int flrid = _accumulator.get_register();

    _accumulator.set_register(value);
    _accumulator.copy_to_floor(flrid);
    spdlog::debug("set floor[{}] with value {}", flrid, value);

    END_VISIT();
}

int Interpreter::visit(const parser::FloorAccessASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_index_expr());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    int idx = _accumulator.get_register();

    _accumulator.copy_from_floor(idx);
    spdlog::debug("loaded floor[{}] with value {}", idx, _accumulator.get_register());

    END_VISIT();
}

int Interpreter::visit(const parser::NegativeExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_operand());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);

    _accumulator.set_register(-_accumulator.get_register());
    spdlog::debug("negated {}. result: {}", -_accumulator.get_register(), _accumulator.get_register());

    END_VISIT();
}

int Interpreter::visit(const parser::NotExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_operand());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    int val = _accumulator.get_register();
    _accumulator.set_register(val ? 0 : 1);
    spdlog::debug("notted {}. result: {}", val, _accumulator.get_register());

    END_VISIT();
}

int Interpreter::visit(const parser::IncrementExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    auto symbol = semanalyzer::Symbol::get_from(node);
    assert(symbol);
    _accumulator.bumpup(symbol);
    spdlog::debug("bumped up {} to {}", symbol->name, _accumulator.get_register());
    END_VISIT();
}

int Interpreter::visit(const parser::DecrementExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    auto symbol = semanalyzer::Symbol::get_from(node);
    assert(symbol);
    _accumulator.bumpdn(symbol);
    spdlog::debug("bumped down {} to {}", symbol->name, _accumulator.get_register());
    END_VISIT();
}

int Interpreter::visit(const parser::AddExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::SubExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::MulExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::DivExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::ModExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::EqualExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::NotEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::GreaterThanExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::GreaterEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::LessThanExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::LessEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::AndExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit(const parser::OrExpressionASTNodePtr &node)
{
    return visit_binary_expression(node);
}

int Interpreter::visit_binary_expression(const parser::AbstractBinaryExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_left());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);

    int left = _accumulator.get_register();

    rc = traverse(node->get_right());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);

    int right = _accumulator.get_register();

    int result = 0;
    auto op = node->get_op();
    switch (op) {
    case parser::ASTBinaryOperator::ADD:
        result = left + right;
        break;
    case parser::ASTBinaryOperator::SUB:
        result = left - right;
        break;
    case parser::ASTBinaryOperator::MUL:
        result = left * right;
        break;
    case parser::ASTBinaryOperator::DIV:
        ensure_non_zero(right);
        result = left / right;
        break;
    case parser::ASTBinaryOperator::MOD:
        ensure_non_zero(right);
        result = left % right;
        break;
    case parser::ASTBinaryOperator::AND:
        result = left && right ? 1 : 0;
        break;
    case parser::ASTBinaryOperator::OR:
        result = left || right ? 1 : 0;
        break;
    case parser::ASTBinaryOperator::GT:
        result = left > right ? 1 : 0;
        break;
    case parser::ASTBinaryOperator::GE:
        result = left >= right ? 1 : 0;
        break;
    case parser::ASTBinaryOperator::LT:
        result = left < right ? 1 : 0;
        break;
    case parser::ASTBinaryOperator::LE:
        result = left <= right ? 1 : 0;
        break;
    case parser::ASTBinaryOperator::EQ:
        result = left == right ? 1 : 0;
        break;
    case parser::ASTBinaryOperator::NE:
        result = left != right ? 1 : 0;
        break;
    }

    _accumulator.set_register(result);
    spdlog::debug("binary operation {} performed with {} and {}, result {}", parser::get_ast_binary_operator_str(op), left, right, result);

    END_VISIT();
}

int Interpreter::visit(const parser::InvocationExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    spdlog::debug("preparing invocation to {}", *node->get_func_name());

    auto &arg_node = node->get_argument();
    if (arg_node) {
        rc = traverse(arg_node);
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
        spdlog::debug("evaulated invocation parameter {}", _accumulator.get_register());
    }

    auto symbol = semanalyzer::Symbol::get_from(node);
    // special handling for lib func
    if (_symbol_table->is_library_function(symbol)) {
        spdlog::debug("invoking library function {}", symbol->name);
        rc = invoke_library_function(symbol->name);
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    } else {
        auto subroutine_node = WEAK_TO_SHARED(symbol->definition);
        assert(subroutine_node);

        auto type = subroutine_node->get_node_type();

        if (type == parser::ASTNodeType::FunctionDefinition) {
            spdlog::debug("invoking function {}", symbol->name);
            auto func = std::dynamic_pointer_cast<parser::FunctionDefinitionASTNode>(subroutine_node);
            _call_stack.push(func);
            rc = visit(func);
        } else if (type == parser::ASTNodeType::SubprocDefinition) {
            spdlog::debug("invoking subproc {}", symbol->name);
            auto sub = std::dynamic_pointer_cast<parser::SubprocDefinitionASTNode>(subroutine_node);
            _call_stack.push(sub);
            rc = visit(sub);
        } else {
            spdlog::critical("Unknwon ASTNode type {}. {}", static_cast<int>(type), __PRETTY_FUNCTION__);
            throw;
        }
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
        _call_stack.pop();
        spdlog::debug("return from invocation to {}", *node->get_func_name());
    }

    END_VISIT();
}

int Interpreter::visit(const parser::EmptyStatementASTNodePtr &node)
{
    UNUSED(node);
    // do nothing
    return 0;
}

int Interpreter::visit(const parser::IfStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_condition());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    spdlog::debug("evaluated if condition: {}", _accumulator.get_register());

    if (_accumulator.is_true()) {
        spdlog::debug("branching to if-then");
        rc = traverse(node->get_then_branch());
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    } else {
        spdlog::debug("branching to if-else");
        rc = traverse(node->get_else_branch());
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    }

    END_VISIT();
}

int Interpreter::visit(const parser::WhileStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    do {
        rc = traverse(node->get_condition());
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
        spdlog::debug("evaluated while condition: {}", _accumulator.get_register());

        if (!_accumulator.is_true()) {
            // condition evaluated to false
            spdlog::debug("while condition evaluated to false");
            break;
        }

        spdlog::debug("looping...");

        // condition is true
        rc = traverse(node->get_body());
        RETURN_IF_FAIL_IN_VISIT(rc);

        if (rc == CF_BreakRequested) {
            // for hit break
            rc = 0;
            spdlog::debug("break requested");
            END_VISIT();
        }

        if (rc == CF_ContinueRequested) {
            // for hit continue. enter next loop
            // there's nothing special inside this if. it's the same as body is normally finished
            rc = 0;
            spdlog::debug("continue requested");
        }
    } while (true);

    END_VISIT();
}

int Interpreter::visit(const parser::ForStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_init());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);

    do {
        // evaluate the condition of for
        rc = traverse(node->get_condition());
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
        spdlog::debug("evaluated for condition: {}", _accumulator.get_register());

        if (!_accumulator.is_true()) {
            // condition evaluated to false
            spdlog::debug("for condition evaluated to false");
            break;
        }

        // condition is true
        rc = traverse(node->get_body());
        RETURN_IF_FAIL_IN_VISIT(rc);

        if (rc == CF_BreakRequested) {
            // for hit break
            rc = 0;
            spdlog::debug("break requested");
            END_VISIT();
        }

        if (rc == CF_ContinueRequested) {
            // for hit continue. update and enter next loop
            // there's nothing special inside this if. it's the same as body is normally finished
            spdlog::debug("continue requested");
        }

        rc = traverse(node->get_update());
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
        spdlog::debug("for state updated");
    } while (true);
    END_VISIT();
}

int Interpreter::visit(const parser::ReturnStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    auto &retexpr = node->get_expression();
    if (retexpr) {
        rc = traverse(retexpr);
        RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
        spdlog::debug("return requested: {}", _accumulator.get_register());
    } else {
        spdlog::debug("return requested");
    }

    rc = CF_ReturnRequested;
    END_VISIT();
}

int Interpreter::visit(const parser::BreakStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = CF_BreakRequested;
    END_VISIT();
}

int Interpreter::visit(const parser::ContinueStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = CF_ContinueRequested;
    END_VISIT();
}

int Interpreter::visit(const parser::StatementBlockASTNodePtr &node)
{
    BEGIN_VISIT();
    spdlog::debug("entered statement block");
    parser::StatementsVector &stmts = node->get_statements();
    rc = traverse(stmts);
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);
    spdlog::debug("left statement block");

    END_VISIT();
}

int Interpreter::visit_subroutine(const parser::AbstractSubroutineASTNodePtr &node)
{
    BEGIN_VISIT();
    spdlog::debug("entered subroutine '{}' {} param.", *node->get_name(), node->get_parameter() ? "with" : "without");

    auto &param = node->get_parameter();
    if (param) {
        // store the parameter value
        semanalyzer::SymbolPtr sym = semanalyzer::Symbol::get_from(param);
        assert(sym);
        _accumulator.copy_to(sym);
    }
    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);
    rc = 0;
    // for rc < 0 we just return

    spdlog::debug("left subroutine '{}'.", *node->get_name());
    END_VISIT();
}

int Interpreter::visit(const parser::SubprocDefinitionASTNodePtr &node)
{
    return visit_subroutine(node);
}

int Interpreter::visit(const parser::FunctionDefinitionASTNodePtr &node)
{
    return visit_subroutine(node);
}

int Interpreter::visit(const parser::CompilationUnitASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse_multiple(node->get_floor_inits(), node->get_var_decls());
    RETURN_IF_ABNORMAL_RC_IN_VISIT(rc);

    auto subroutines = node->get_subroutines();
    auto start_func = std::find_if(subroutines.cbegin(), subroutines.cend(), [](const parser::AbstractSubroutineASTNodePtr &subroutine) {
        return *subroutine->get_name() == "start";
    });

    if (auto start = std::dynamic_pointer_cast<parser::SubprocDefinitionASTNode>(*start_func)) {
        rc = visit(start);
    } else if (auto start = std::dynamic_pointer_cast<parser::FunctionDefinitionASTNode>(*start_func)) {
        rc = visit(start);
    } else {
        spdlog::critical("Unknwon subroutine type {}. {}", static_cast<int>(start_func->get()->get_node_type()), __PRETTY_FUNCTION__);
        throw;
    }

    END_VISIT();
}

int Interpreter::run()
{
    return visit(_root);
}

void Interpreter::enter_node(const parser::ASTNodePtr &node)
{
    spdlog::debug("Entered node {} on {}:{}", ast_node_type_to_string(node->get_node_type()), node->lineno(), node->colno());
    semanalyzer::SemanticAnalysisPass::enter_node(node);
}

void Interpreter::leave_node()
{
    semanalyzer::SemanticAnalysisPass::leave_node();
}

void Interpreter::ensure_non_zero(int value)
{
    if (value == 0) {
        throw InterpreterException(InterpreterException::ErrorType::ValueIsZero, "Value cannot be zero");
    }
}

int Interpreter::invoke_inbox()
{
    _accumulator.inbox();
    return 0;
}

int Interpreter::invoke_outbox()
{
    _accumulator.outbox();
    return 0;
}

int Interpreter::invoke_library_function(const std::string &name)
{
    if (name == "inbox") {
        return invoke_inbox();
    } else if (name == "outbox") {
        return invoke_outbox();
    } else {
        spdlog::critical("Unknwon library function {}. {}", name, __PRETTY_FUNCTION__);
        throw;
    }
}

CLOSE_INTERPRETER_NAMESPACE

// end
