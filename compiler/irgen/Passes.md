# Passes

| Pass Name                    | Dependencies         | Output Generated    | Invalidates         | Notes   |
|------------------------------|----------------------|---------------------|---------------------|---------|
| AnalyzeLivenessPass          | CFG                  | IN, OUT, DEF, USE   | None                |         |
| BuildSSAPass                 | CFG, IN, OUT, DEF, USE | CFG, DOM          | IN, OUT, DEF, USE   |         |
| BuildControlFlowGraphPass    | None                 | CFG                 | None                |         |
| EliminateDeadBasicBlockPass  | CFG                  | None                | DOM                 |         |
| MergeConditionalBranchPass   | None                 | None                | None                |         |
| RenumberVariableIdPass       | None                 | None                | IN, OUT, DEF, USE   |         |
| StripEmptyBasicBlockPass     | None                 | None                | CFG, DOM            |         |
| StripUselessInstructionPass  | None                 | None                | None                |         |
| VerifySSAPass                | SSA                  | None                | None                |         |

## Pass Order

1. Remove NOPs and empty basic blocks (`StripUselessInstructionPass`, `StripEmptyBasicBlockPass`)
2. Merge conditional branch instructions (`MergeConditionalBranchPass`)
3. Construct the Control Flow Graph (CFG) (`BuildControlFlowGraphPass`)
4. Remove dead basic blocks (`EliminateDeadBasicBlockPass`)
5. Perform variable liveness analysis (`AnalyzeLivenessPass`)
6. Build Static Single Assignment (SSA) form (`BuildSSAPass`)
7. Renumber variables (`RenumberVariableIdPass`)
8. Perform liveness analysis for SSA (`AnalyzeLivenessPass`)
9. Verify the correctness of SSA (`VerifySSAPass`)
