# Semantic Analysis Phase in Compiler

## TODO List for Semantic Analysis Phase

The following is a breakdown of pending tasks to be implemented in the semantic analysis phase:

### Symbol Resolution
- [x] Build the symbol table.
- [x] Ensure all variables and functions are declared before use.
- [x] Detect and handle illegal redefinitions of variables and functions.
- [x] Support block-scoped variable and check for shadowing.
- [x] Implement function signature matching verification.
- [ ] Add support for resolving imported module symbols.

### Early Constant Folding and Propagation
- [x] Optimize constant expressions during the analysis phase (constant folding).
- [ ] Propagate constants where applicable in simple expressions (constant propagation).
- [x] Check div by constant zero error

### Symbol Use Analysis
- [x] Enforce the variable assignment before use
- [ ] Detect unused variables

### Control Flow Analysis
- [x] Implement detection of unreachable code and issue appropriate warnings.
- [ ] Validate that all control paths in non-void functions return a value.
- [ ] Verify correct usage of `break`, `continue`, and `return` statements.
- [ ] Ensure that all code paths in a function lead to a valid return statement.
- [ ] Check that all functions with a return statement actually return a value, and flag unreachable after return.
