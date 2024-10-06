# Optimizations

## Based on SSA

Static Single Assignment (SSA) form is widely used in compilers because it simplifies many optimizations by making the flow of values through the program explicit. Once your intermediate representation (IR) is in SSA form, you can perform several advanced optimizations more effectively. Here are some common optimizations that are easier or more efficient to implement using SSA:

### 1. **Constant Propagation**
   - [ ] Implemented?
   - **What it does**: Replaces the use of variables with known constant values.
   - **Benefit from SSA**: In SSA, each variable is assigned exactly once, making it easy to trace constants back to their definitions. This reduces the complexity of identifying where a value remains constant.

### 2. **Dead Code Elimination (DCE)**
   - [ ] Implemented?
   - **What it does**: Removes code that computes values never used.
   - **Benefit from SSA**: SSA simplifies the analysis of which variables are live, as each assignment has only one definition and can be easily tracked. If a variable's value is never used, the assignment can be removed.

### 3. **Sparse Conditional Constant Propagation (SCCP)**
   - [ ] Implemented?
   - **What it does**: Combines constant propagation with control flow analysis to remove unreachable code.
   - **Benefit from SSA**: SSA simplifies the propagation of constants across the control flow graph (CFG), allowing SCCP to easily detect and remove dead branches or blocks.

### 4. **Global Value Numbering (GVN)**
   - [ ] Implemented?
   - **What it does**: Eliminates redundant computations by identifying equivalent expressions and replacing one with the other.
   - **Benefit from SSA**: In SSA, because variables are only assigned once, it's easier to identify when different variables hold the same value. This makes finding equivalent expressions straightforward.

### 5. **Common Subexpression Elimination (CSE)**
   - [ ] Implemented?
   - **What it does**: Identifies and eliminates expressions that are computed multiple times with the same values.
   - **Benefit from SSA**: The single-assignment property of SSA makes it easier to recognize identical expressions, allowing the optimizer to replace them with a single computation.

### 6. **Loop-Invariant Code Motion (LICM)**
   - [ ] Implemented?
   - **What it does**: Moves computations that are constant within a loop outside the loop to avoid repeated calculations.
   - **Benefit from SSA**: SSA helps easily identify variables that do not change during the execution of a loop, allowing the optimizer to hoist these computations outside the loop.

### 7. **Strength Reduction**
   - [ ] Implemented?
   - **What it does**: Replaces expensive operations (like multiplication) with cheaper ones (like addition or shifts) where possible.
   - **Benefit from SSA**: Since SSA form provides a clear and simple data flow, the compiler can more easily identify opportunities for transforming operations into more efficient ones.

### 8. **Partial Redundancy Elimination (PRE)**
   - [ ] Implemented?
   - **What it does**: Removes computations that are redundant on some, but not necessarily all, paths through a program.
   - **Benefit from SSA**: SSA allows the compiler to more easily reason about expressions that are redundant across multiple paths, which can then be optimized globally rather than locally.

### 9. **Phi-Node Elimination**
   - [ ] Implemented?
   - **What it does**: In some cases, after other optimizations (like dead code elimination or constant propagation), certain `phi` nodes become redundant and can be removed.
   - **Benefit from SSA**: `phi` nodes are a fundamental part of SSA, and simplifying or eliminating them can lead to further simplifications in the IR.

### 10. **Copy Propagation**
   - [ ] Implemented?
   - **What it does**: Eliminates assignments where one variable is simply a copy of another, replacing the use of the copied variable with the original.
   - **Benefit from SSA**: In SSA, each variable is assigned only once, making it trivial to replace uses of copied variables with their original definitions.

### 11. **Memory-to-Register Promotion**
   - [ ] Implemented?
   - **What it does**: Converts variables that are stored in memory into variables that are held in registers, eliminating unnecessary memory loads and stores.
   - **Benefit from SSA**: SSA often leads to fewer variables and clearer dependencies, making it easier to promote stack-based or heap-based variables into registers.

### 12. **Aggressive Dead Store Elimination**
   - [ ] Implemented?
   - **What it does**: Removes stores to memory locations that are not subsequently read before being overwritten.
   - **Benefit from SSA**: SSA form makes it easier to track variable definitions and usages, helping detect stores that can be safely eliminated.

### 13. **Control Flow Simplification**
   - [ ] Implemented?
   - **What it does**: Simplifies the control flow by merging basic blocks, removing unnecessary jumps, and optimizing branch conditions.
   - **Benefit from SSA**: SSA makes control dependencies explicit, so simplifying the control flow becomes easier to reason about and implement.

### 14. **Induction Variable Simplification**
   - [ ] Implemented?
   - **What it does**: Simplifies and optimizes induction variables, often related to loop indices, which are incremented or decremented in loops.
   - **Benefit from SSA**: SSA allows the induction variable to be represented clearly, making it easier to apply simplifications like strength reduction or loop unrolling.

---

SSA form enables these optimizations to be implemented more effectively by reducing the complexity of tracking value definitions and uses. This allows for more aggressive and accurate optimizations, resulting in better performance and lower code size.
