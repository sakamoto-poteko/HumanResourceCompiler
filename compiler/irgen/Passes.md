# Passes

| Pass Name                    | Dependencies         | Output Generated    | Invalidates         | Notes   |
|------------------------------|----------------------|---------------------|---------------------|---------|
| AnalyzeLivenessPass          | CFG                  | IN, OUT, DEF, USE   | None                |         |
| BuildSSAPass                 | CFG, IN, OUT, DEF, USE | CFG, DOM          | IN, OUT, DEF, USE   |         |
| BuildControlFlowGraphPass    | None                 | CFG                 | None                |         |
| EliminateDeadAssignment      | SSA                  | None                | IN, OUT, DEF, USE   |         |
| EliminateDeadBasicBlockPass  | CFG                  | None                | DOM                 |         |
| MergeConditionalBranchPass   | None                 | None                | None                |         |
| PropagateCopyPass            | SSA, DOM             | None                | IN, OUT, DEF, USE   |         |
| RenumberVariableIdPass       | None                 | None                | IN, OUT, DEF, USE   |         |
| StripEmptyBasicBlockPass     | None                 | None                | CFG, DOM            |         |
| StripUselessInstructionPass  | None                 | None                | None                |         |
| VerifySSAPass                | SSA                  | None                | None                |         |

## Pass Order

1. Remove NOPs and empty basic blocks (`StripUselessInstructionPass`, `StripEmptyBasicBlockPass`).
2. Merge conditional branch instructions (`MergeConditionalBranchPass`).
3. Construct the Control Flow Graph (CFG) (`BuildControlFlowGraphPass`). Valid: CFG.
4. Remove dead basic blocks (`EliminateDeadBasicBlockPass`). Valid: CFG.
5. Perform variable liveness analysis (`AnalyzeLivenessPass`). Valid: CFG, IN, OUT, DEF, USE.
6. Build Static Single Assignment (SSA) form (`BuildSSAPass`). Valid: CFG, SSA, DOM.
7. Renumber variables (`RenumberVariableIdPass`). Valid: CFG, SSA, DOM.
8. Verify the correctness of SSA (`VerifySSAPass`). Valid: CFG, SSA, DOM.
9. Propagate register copy (`PropagateCopyPass`). Valid: CFG, SSA, DOM.
10. Eliminate dead assignments (`EliminateDeadAssignmentPass`). Valid: CFG, SSA, DOM
11. Perform liveness analysis for SSA (`AnalyzeLivenessPass`). Valid: CFG, SSA, DOM, IN, OUT, DEF, USE
