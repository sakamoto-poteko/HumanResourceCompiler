#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>

#include "ASTBuilder.h"
#include "ASTNodeGraphvizBuilder.h"
#include "ErrorManager.h"
#include "HRLLexer.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeGraphvizBuilder.h"
#include "RecursiveDescentParser.h"
#include "Tests.h"
#include "WithParsed.h"

void WithParsed::print_test_info(const TestCaseData &data)
{
    data.print_setup();
}

void WithParsed::setup_parse(const TestCaseData &data, bool &result)
{
    result = false;

    print_test_info(data);

    ErrorManager &errmgr = ErrorManager::instance();
    errmgr.clear();

    // Redirect spdlog output
    auto captured_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(captured_outstream);
    captured_sink->set_level(spdlog::level::trace);
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(data.filename, captured_sink));
    spdlog::flush_on(spdlog::level::trace);

    // Open HRML file
    FILE *file = std::fopen(data.path.c_str(), "r");
    ASSERT_NE(file, nullptr) << "Failed to open HRML " << data.path;

    // Lexing
    hrl::lexer::HRLLexer lexer;
    std::vector<hrl::lexer::TokenPtr> tokens;
    bool ok = lexer.lex(file, data.path, tokens);
    errmgr.print_all();
    ASSERT_TRUE(ok)
        << "Lexical analysis failed\n"
        << captured_outstream.str();
    ASSERT_FALSE(errmgr.has_errors())
        << "Lexing has errors:\n"
        << captured_outstream.str();
    fclose(file);

    // Parsing
    hrl::parser::RecursiveDescentParser parser(data.path, tokens);
    hrl::parser::CompilationUnitPTNodePtr compilation_unit;
    bool parsed = parser.parse(compilation_unit);
    errmgr.print_all();
    ASSERT_TRUE(parsed)
        << "Parsing failed\n"
        << captured_outstream.str();
    ASSERT_FALSE(errmgr.has_errors())
        << "Parsing has errors:\n"
        << captured_outstream.str();
    hrl::parser::ParseTreeNodeGraphvizBuilder graphviz(compilation_unit);
    graphviz.generate_graphviz(data.filename + "-pt.dot");

    // Building AST
    hrl::parser::ASTBuilder builder(compilation_unit);
    bool built = builder.build(ast);
    errmgr.print_all();
    ASSERT_TRUE(built)
        << "Failed to build AST\n"
        << captured_outstream.str();
    ASSERT_FALSE(errmgr.has_errors())
        << "Building AST has errors\n"
        << captured_outstream.str();
    hrl::parser::ASTNodeGraphvizBuilder graphviz_ast(ast);
    graphviz_ast.generate_graphviz(data.filename + "-ast.dot");

    result = true;
}
