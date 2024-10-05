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
