# IR Generator

## Overview

This module handles the **Intermediate Representation (IR) Generation** phase of the compiler, which includes:
- **Control Flow Graph (CFG) construction**: A representation of the flow of control through the program.
- **Static Single Assignment (SSA)** form: A transformation that simplifies the optimization process by ensuring each variable is assigned exactly once.
- **High-Level IR**: A more abstract and target-independent representation of the program.
- **Low-Level IR**: A machine-specific IR closer to the constraints and instructions of our accumulator-based architecture.

The IR is first generated as **high-level IR**, which provides an expressive, target-independent abstraction of the program. Later, it is lowered into **low-level IR**, which is closer to the architecture of the target machine, specifically designed to fit the constraints of an accumulator-based HRM architecture.

## Intermediate Representations

### 1. **High-Level IR (HIR)**

The High-Level IR (HIR) is generated from the Abstract Syntax Tree (AST) and is designed to be a target-independent representation of the program. The HIR uses a rich set of instructions, allowing for more expressive control flow, arithmetic, and logical operations.

Key features of HIR include:
- **Three-Address Code (TAC)** format for most operations.
- **Comprehensive control flow instructions**: Conditional branches like `je` (jump if equal), `jne` (jump if not equal), `jgt` (jump if greater), etc.
- **Arithmetic operations**: Operations like `mul` (multiply), `div` (divide), and `mod` (modulus) are represented, even if the target architecture lacks direct support for them.

HIR allows for powerful optimizations such as:
- **Control flow analysis and optimization** (e.g., dead code elimination, branch simplification).
- **Algebraic optimizations** (constant folding, strength reduction, etc.).

### 2. **Low-Level IR (LIR)**

After generating HIR and applying high-level optimizations, the IR is **lowered** into **Low-Level IR (LIR)**. This representation is closer to the actual instruction set of the target architecture, which is an accumulator-based machine with limited branching and arithmetic capabilities.

Key aspects of LIR:
- **Accumulator-based**: All operations assume an accumulator-based instruction set where most arithmetic and memory operations involve the accumulator.
- **Translation of complex operations**: Operations like `mul`, `div`, and `mod` are broken down into sequences of addition and subtraction.
- **Simple branching**: High-level conditional jumps are translated into basic jump instructions (`jump`, `jumpn`, `jumpz`).

## IR Structure

### High-Level IR (HIR) Instructions

#### 1. **Arithmetic Operations**
These operations handle basic arithmetic, represented in the TAC format.

| Operation | Description                         | Example          |
|-----------|-------------------------------------|------------------|
| `add`     | Adds `src1` and `src2`, stores in `dest` | `add a b, c`    |
| `sub`     | Subtracts `src2` from `src1`, stores in `dest` | `sub a, b, c`    |
| `mul`     | Multiplies `src1` by `src2`, stores in `dest` | `mul a, b, c`    |
| `div`     | Divides `src1` by `src2`, stores in `dest` | `div a, b, c`    |
| `mod`     | Modulo operation, `src1 % src2`, stores in `dest` | `mod a, b, c`    |

*Note*: `mul`, `div`, and `mod` will be translated into appropriate LIR sequences during the lowering phase.

#### 2. **Control Flow Operations**
HIR includes a rich set of branching instructions, allowing for more expressive control flow.

| Operation  | Description                                                | Example               |
|------------|------------------------------------------------------------|-----------------------|
| `je`       | Jump if `src1 == src2`                                     | `je a, b, label`      |
| `jne`      | Jump if `src1 != src2`                                     | `jne a, b, label`     |
| `jgt`      | Jump if `src1 > src2`                                      | `jgt a, b, label`     |
| `jlt`      | Jump if `src1 < src2`                                      | `jlt a, b, label`     |
| `jge`      | Jump if `src1 >= src2`                                     | `jge a, b, label`     |
| `jle`      | Jump if `src1 <= src2`                                     | `jle a, b, label`     |

*Note*: These high-level branching instructions are later lowered into simpler jumps supported by the architecture (e.g., `jumpz`, `jumpn`).

### Low-Level IR (LIR) Instructions

#### 1. **Accumulator-Based Arithmetic**
In the low-level IR, arithmetic operations are tailored to fit the accumulator-based nature of the target architecture.

| Operation | Description                        | Example         |
|-----------|------------------------------------|-----------------|
| `load`    | Loads a value from memory into the accumulator | `load addr`     |
| `store`   | Stores the accumulator value into memory       | `store addr`    |
| `add`     | Adds a value to the accumulator                | `add addr`      |
| `sub`     | Subtracts a value from the accumulator         | `sub addr`      |
| `mul_seq` | Sequence of instructions for multiplication    | Expanded during codegen |
| `div_seq` | Sequence of instructions for division          | Expanded during codegen |

#### 2. **Simple Control Flow**

| Operation  | Description                           | Example              |
|------------|---------------------------------------|----------------------|
| `jump`     | Unconditional jump to a label         | `jump label`         |
| `jumpz`      | Jump if accumulator is zero       | `jumpz label`          |
| `jumpn`     | Jump if accumulator is negative       | `jumpn label`         |

### Example of HIR to LIR Translation

Consider the following HIR (High-Level IR):

```plaintext
add a, b, c      // a = b + c
mul d, a, 3      // d = a * 5
jlt d, e, label  // Jump to label if d < e
```

In LIR, this might translate to:

```plaintext
load b           // Load b into accumulator
add c            // Add c to accumulator
store a          // Store accumulator into a

// Multiplication by 5 (sequence of add and shift instructions)
load a
add a
add a
add a
store d

load d           // Load d into accumulator
sub e            // Subtract e from accumulator
jumpz label      // Jump if result is negative (d < e)
```

## Control Flow Graph (CFG) and SSA

### Control Flow Graph (CFG)
The **Control Flow Graph (CFG)** is generated during IR generation to represent the possible paths of execution within a function. Each node in the CFG corresponds to a basic block, and edges represent possible control flow transitions (e.g., jumps, branches).

### Static Single Assignment (SSA)
During the IR generation process, we also convert the HIR into **Static Single Assignment (SSA) form**. In SSA, each variable is assigned exactly once, which simplifies many optimizations like constant propagation, dead code elimination, and common subexpression elimination.

SSA transformation involves:
- **Renaming variables**: Every time a variable is assigned, it gets a new name.
- **Φ-functions**: At points where multiple control flow paths converge, **φ-functions** are introduced to select the correct variable value based on the execution path.

## Workflow

[ ] **AST to HIR**: The first step translates the Abstract Syntax Tree (AST) generated by the parser into High-Level IR (HIR).
[ ] **CFG and SSA**: During HIR generation, we also build the Control Flow Graph (CFG) and convert the IR into Static Single Assignment (SSA) form for optimization.
[ ] **HIR Optimizations**: Apply high-level optimizations such as constant folding, dead code elimination, and algebraic simplifications.
[ ] **HIR to LIR Lowering**: The HIR is then lowered into Low-Level IR (LIR), closer to the accumulator-based machine.
[ ] **Code Generation**: Finally, the LIR is translated into target-specific assembly or machine code, expanding complex operations like multiplication and handling limited branching instructions.
