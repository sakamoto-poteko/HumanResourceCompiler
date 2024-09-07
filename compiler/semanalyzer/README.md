# Semantic Analysis Phase in Compiler

## TODO List for Semantic Analysis Phase

The following is a breakdown of pending tasks to be implemented in the semantic analysis phase:

### Symbol Resolution
- [x] Build the symbol table.
- [x] Ensure all variables and functions are declared before use.
- [x] Detect and handle illegal redefinitions of variables and functions.
- [x] Add support for block-scoped variables.
- [ ] Ensure proper handling of shadowing rules (for both variables and functions).
- [ ] Implement function signature matching verification.
- [ ] Add support for resolving imported module symbols.
- [ ] Enforce distinction between global and local scopes.

### Early Constant Folding and Propagation
- [ ] Optimize constant expressions during the analysis phase (constant folding).
- [ ] Propagate constants where applicable in simple expressions (constant propagation).
- [ ] Check div by constant zero error

### Symbol Use Analysis
- [ ] Enforce the variable assignment before use
- [ ] Detect unused variables

### Control Flow Analysis
- [ ] Implement detection of unreachable code and issue appropriate warnings.
- [ ] Validate that all control paths in non-void functions return a value.
- [ ] Verify correct usage of `break`, `continue`, and `return` statements.
- [ ] Ensure that all code paths in a function lead to a valid return statement.
- [ ] Check that all functions with a return statement actually return a value, and flag unreachable after return.

