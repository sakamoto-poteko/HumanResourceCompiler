#include "WithIR.h"
#include "ErrorManager.h"
#include "IRGenOptions.h"
#include "IROptimizationPassManager.h"

void WithIR::setup_ir(int optimize, const TestCaseData &data, bool &result)
{
    setup_semantic_analyze(optimize != 0, data, result);

    hrl::irgen::IRGenOptions irgen_opt;
    if (optimize > 0) {
        irgen_opt = hrl::irgen::IRGenOptions::ForSpeed();
    } else if (optimize < 0) {
        irgen_opt = hrl::irgen::IRGenOptions::ForCodeSize();
    }

    auto irop_passmgr = hrl::irgen::IROptimizationPassManager::create_with_default_pass_configuration(program, irgen_opt, true, "build/ir/");

    int irop_result = irop_passmgr.run(true);
    ErrorManager::instance().print_all();

    ASSERT_EQ(irop_result, 0) << "Expected IR optimizer to pass but it failed";
}
