# How to build the SSA from Linear TAC and CFG?

_Partly written by ChatGPT_

Building Static Single Assignment (SSA) form is a powerful step in compiler design that simplifies optimization and analysis by ensuring each variable is assigned exactly once. Since you're already familiar with Linear Three-Address Code (TAC) and Control Flow Graphs (CFGs), we'll build upon that foundation. Here's a step-by-step guide to converting your existing TAC and CFG into SSA form:

---

### **1. Understand the Basics of SSA**

**Static Single Assignment (SSA)** requires that:
- **Each variable is assigned exactly once.**
- **Every variable is defined before it is used.**

To achieve this:
- **Unique Versions:** Assign a unique version to each variable assignment.
- **Φ (Phi) Functions:** Introduce special functions at control flow join points to merge different variable versions.

---

### **2. Prerequisites**

Before converting to SSA, ensure you have:
- **Control Flow Graph (CFG):** Comprised of basic blocks within a function.
- **Basic Blocks:** Sequences of instructions with a single entry and exit point.
- **Variable Definitions and Uses:** Knowledge of where each variable is defined and used within the CFG.

---

### **3. Compute Dominators and Dominance Frontiers**

**Dominators:**

- A node **A** **dominates** node **B** if every path from the entry node to **B** must go through **A**.
- The **immediate dominator** of a node is the closest dominator of that node. Mathematically, we can express this as:

```math
A \text{ dominates } B \iff \forall \text{ path } P \text{ from Entry to } B, \, A \in P
```

**Strict Dominance**

Node $A$ **strictly dominates** node $B$ if:

```math
A \text{ strictly dominates } B \iff A \text{ dominates } B \text{ and } A \neq B
```

In other words, strict dominance means $A$ dominates $B$, but $A$ and $B$ are not the same node. This is denoted as:

```math
\text{strict-dom}(A, B) \iff \text{dom}(A, B) \land A \neq B
```

**Immediate Dominator**

The **immediate dominator** of a node $B$, denoted as $\text{idom}(B)$, is the closest dominator of $B$. It is the last dominator encountered on every path to $B$ before $B$ itself.

Formally, $A = \text{idom}(B)$ if:

1. $A \text{ dominates } B$, i.e., $\text{dom}(A, B)$.
2. $A$ is the closest dominator of $B$, i.e., there is no node $C$ such that $\text{dom}(C, B)$ and $\text{dom}(A, C)$.

Mathematically, this can be written as:

```math
A = \text{idom}(B) \iff \text{dom}(A, B) \land (\nexists C \text{ such that } \text{dom}(A, C) \land \text{dom}(C, B))
```

The immediate dominator helps construct the **dominator tree**, where each node points to its immediate dominator.

**Dominance Properties**

Dominance relations are a **partial order**, meaning they have the following properties:

1. **Reflexive**: A node always dominates itself.
   $$
   \forall A, \text{ dom}(A, A)
   $$

   This means every node $A$ trivially dominates itself.

2. **Antisymmetric**: If node $A$ dominates node $B$, and $B$ dominates $A$, then $A$ and $B$ must be the same node.
   
```math
\text{dom}(A, B) \land \text{dom}(B, A) \implies A = B
```

3. **Transitive**: If $A$ dominates $B$ and $B$ dominates $C$, then $A$ also dominates $C$.
   
```math
\text{dom}(A, B) \land \text{dom}(B, C) \implies \text{dom}(A, C)
```

These properties make dominance a **partial order** on the nodes of the control flow graph.

**Dominator Tree**

Using the immediate dominator relationship, we can form a **dominator tree**:

- Each node $B$ is connected to its immediate dominator $\text{idom}(B)$.
- The entry node (start node) is the root of the tree, as it has no dominators other than itself.

If we have a CFG, the dominator tree helps understand the flow of control and can be used to optimize the program by placing **φ-functions** for SSA form where necessary.


**Dominance Frontiers:**

- The **dominance frontier** of a node **A** consists of all nodes **B** where **A** dominates a predecessor of **B**, but **A** does not strictly dominate **B**.

Formally:

```math
DF(A) = \{ B \mid \exists P \in \text{preds}(B), \text{ dom}(A, P) \land \neg \text{dom}(A, B) \}
```

Where:
- $\text{preds}(B)$ represents the set of predecessors of $B$ in the CFG.


**Why This Matters:**

- Dominance frontiers help determine where to place φ-functions by identifying where different definitions of a variable can converge.

**How to Compute:**

- Use algorithms like the **Lengauer-Tarjan** algorithm to efficiently compute dominators and dominance frontiers.
- Many compiler frameworks provide built-in functions to compute these, so leveraging existing tools is recommended if available.


### **4. Identify Variable Definitions and Uses**

- **Definitions (Defs):** Points in the code where variables are assigned a value.
- **Uses:** Points where variables are read or utilized.

**Action Steps:**
1. **Traverse the CFG** and **record all definitions** for each variable.
2. **Map variables to their defining blocks** to understand where their values originate.

---

### **5. Insert Φ Functions**

**Purpose of Φ Functions:**
- When a variable can have multiple incoming definitions due to branching, φ-functions merge these definitions into a single SSA variable.

**Placement Strategy:**
1. **For Each Variable:**
   - Determine all blocks where the variable is defined.
   - For each defining block, find the dominance frontier blocks.
2. **Insert a Φ Function** for the variable in each dominance frontier block identified.
   - **Example:** If a variable `x` is defined in multiple branches that merge, a φ-function will select the appropriate version of `x` based on the incoming path.

**Action Steps:**
1. **Iterate through all variables** in your CFG.
2. **For each variable**, identify the blocks needing φ-functions using dominance frontiers.
3. **Insert φ-functions** in these blocks, effectively creating new variable definitions.

---

### **6. Rename Variables to Ensure Single Assignment**

**Objective:**
- Assign a unique version number to each variable definition, ensuring each variable is assigned only once.

**Process Using Depth-First Search (DFS):**

1. **Initialize:**
   - **Version Counters:** For each variable, maintain a counter to assign unique versions.
   - **Stacks:** Use a stack for each variable to keep track of the current active version.

2. **DFS Traversal:**
   - **Start at the Entry Block:** Begin traversing the CFG in a depth-first manner.
   
3. **Within Each Block:**
   - **For Each Φ Function:**
     - Assign a new version to the variable.
     - Push this version onto the variable's stack.
   
   - **For Each Instruction:**
     - **Definitions (Assignments):**
       - Assign a new version number to the defined variable.
       - Replace the variable with its new version.
       - Push this version onto the stack.
     - **Uses:**
       - Replace the variable with the current top version from its stack.

4. **Handle Successors and Predecessors:**
   - **For Each Successor Block:**
     - If a φ-function exists for a variable, set the φ-function's operand corresponding to the current block to the current version of that variable.

5. **After Processing the Block:**
   - **Pop Versions:**
     - For each variable defined in the block (including φ-functions), pop its version from the stack to revert to the previous version as you backtrack.

**Example:**

Consider a simple CFG with two branches merging:

```
Block1:
    x = 1
    goto Block3

Block2:
    x = 2
    goto Block3

Block3:
    φ_x = φ(x from Block1, x from Block2)
    y = φ_x + 3
```

**Renaming Steps:**
1. Assign `x1 = 1` in Block1.
2. Assign `x2 = 2` in Block2.
3. Insert `x3 = φ(x1, x2)` in Block3.
4. Use `x3` in subsequent instructions (`y = x3 + 3`).

---

### **7. Update All Variable References**

After renaming:
- **Each variable use** should refer to the **latest version** assigned along its control flow path.
- **Φ Functions** ensure that merged paths have a consistent version for each variable.

**Action Steps:**
- Ensure all uses of variables in instructions are replaced with their uniquely renamed versions.
- Verify that φ-functions correctly represent the merging of different versions from predecessor blocks.

---

### **8. Validate the SSA Form**

**Consistency Checks:**
- **Single Assignment:** Each variable version is assigned exactly once.
- **Proper Use Before Definition:** Every variable is defined before it's used in the SSA form.

**Action Steps:**
- Traverse the SSA form to ensure that no variable is assigned more than once.
- Check that all uses of variables reference an existing definition.

---

### **9. Optimize and Finalize**

With SSA form established:
- **Optimize Easily:** SSA simplifies optimizations like constant propagation, dead code elimination, and register allocation.
- **Prepare for Further Compiler Stages:** SSA is an excellent intermediate representation for subsequent compilation phases.

---

### **Additional Tips**

- **Leverage Compiler Frameworks:** Tools like LLVM handle SSA construction automatically. Studying their implementation can provide deeper insights.
- **Practice with Examples:** Work through simple code examples manually converting them to SSA to reinforce understanding.
- **Refer to Key Literature:** The original SSA paper by Cytron et al. provides a comprehensive explanation of the algorithm.

---

### **Summary**

Converting your linear TAC and CFG into SSA involves:

1. **Computing dominators and dominance frontiers** to understand the CFG structure.
2. **Identifying variable definitions and uses** to know where variables are assigned and utilized.
3. **Inserting φ-functions** at appropriate join points in the CFG.
4. **Renaming variables** to ensure each assignment is unique.
5. **Validating the SSA form** to ensure correctness.

By following these steps, you'll transform your intermediate representation into SSA, paving the way for more efficient and effective compiler optimizations.

---

If you have any specific questions or need further clarification on any of these steps, feel free to ask!